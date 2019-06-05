#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>
#include <regex.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "misc/macros.h"
#include "misc/log.h"
#include "misc/helpers.h"
#include "misc/cfstring.h"
#include "misc/sbuffer.h"
#define HASHTABLE_IMPLEMENTATION
#include "misc/hashtable.h"
#undef HASHTABLE_IMPLEMENTATION
#include "misc/socket.h"
#include "misc/socket.c"

#include "osax/sa.h"
#include "osax/sa.m"

#include "event.h"
#include "eventloop.h"
#include "event_tap.h"
#include "process.h"
#include "workspace.h"
#include "mouse.h"
#include "rule.h"
#include "message.h"
#include "display.h"
#include "space.h"
#include "view.h"
#include "border.h"
#include "window.h"
#include "application.h"
#include "process_manager.h"
#include "display_manager.h"
#include "space_manager.h"
#include "window_manager.h"

#include "event.c"
#include "eventloop.c"
#include "event_tap.c"
#include "process.c"
#include "workspace.m"
#include "rule.c"
#include "message.c"
#include "display.c"
#include "space.c"
#include "view.c"
#include "border.c"
#include "window.c"
#include "application.c"
#include "process_manager.c"
#include "display_manager.c"
#include "space_manager.c"
#include "window_manager.c"

#define SA_SOCKET_PATH_FMT "/tmp/yabai-sa_%s.socket"

#define CONFIG_FILE_FMT "%s/.yabairc"
#define SOCKET_PATH_FMT "/tmp/yabai_%s.socket"
#define LCFILE_PATH_FMT "/tmp/yabai_%s.lock"

#define CLIENT_OPT_LONG "--message"
#define CLIENT_OPT_SHRT "-m"

#define DEBUG_VERBOSE_OPT_LONG "--verbose"
#define DEBUG_VERBOSE_OPT_SHRT "-V"
#define VERSION_OPT_LONG       "--version"
#define VERSION_OPT_SHRT       "-v"
#define CONFIG_OPT_LONG       "--config"
#define CONFIG_OPT_SHRT       "-c"

#define SCRPT_ADD_INSTALL_OPT   "--install-sa"
#define SCRPT_ADD_UNINSTALL_OPT "--uninstall-sa"
#define SCRPT_ADD_LOAD_OPT      "--load-sa"

#define MAJOR 0
#define MINOR 1
#define PATCH 0

#define CONNECTION_CALLBACK(name) void name(uint32_t type, void *data, size_t data_length, void *context, int cid)
typedef CONNECTION_CALLBACK(connection_callback);
extern CGError SLSRegisterConnectionNotifyProc(int cid, connection_callback *handler, uint32_t event, void *context);

struct eventloop g_eventloop;
void *g_workspace_context;
struct process_manager g_process_manager;
struct display_manager g_display_manager;
struct space_manager g_space_manager;
struct window_manager g_window_manager;
struct mouse_state g_mouse_state;
struct event_tap g_event_tap;
struct daemon g_daemon;
int g_connection;

bool g_mission_control_active;
char g_sa_socket_file[MAXLEN];
char g_socket_file[MAXLEN];
char g_config_file[MAXLEN];
char g_lock_file[MAXLEN];
bool g_verbose;

static int client_send_message(int argc, char **argv)
{
    char *user = getenv("USER");
    if (!user) {
        error("yabai: 'env USER' not set! abort..\n");
    }

    int sockfd;
    char socket_file[MAXLEN];
    snprintf(socket_file, sizeof(socket_file), SOCKET_PATH_FMT, user);

    if (!socket_connect_un(&sockfd, socket_file)) {
        error("yabai: failed to connect to socket..\n");
    }

    int message_length = argc - 1;
    int argl[argc];

    for (int i = 1; i < argc; ++i) {
        argl[i] = strlen(argv[i]);
        message_length += argl[i];
    }

    char message[message_length];
    char *temp = message;

    for (int i = 1; i < argc; ++i) {
        memcpy(temp, argv[i], argl[i]);
        temp += argl[i];
        *temp++ = '\0';
    }

    if (!socket_write_bytes(sockfd, message, message_length)) {
        error("yabai: failed to send data..\n");
    }

    int result = EXIT_SUCCESS;
    int byte_count = 0;
    char rsp[BUFSIZ];

    struct pollfd fds[] = {
        { sockfd, POLLIN, 0 }
    };

    while (poll(fds, 1, -1) > 0) {
        if (fds[0].revents & POLLIN) {
            if ((byte_count = recv(sockfd, rsp, sizeof(rsp)-1, 0)) <= 0) {
                break;
            }

            rsp[byte_count] = '\0';

            if (rsp[0] == FAILURE_MESSAGE[0]) {
                fprintf(stdout, "%s", rsp + 1);
                result = EXIT_FAILURE;
            } else {
                fprintf(stdout, "%s", rsp);
            }

            fflush(stdout);
        }
    }

    socket_close(sockfd);
    return result;
}

static void acquire_lockfile(void)
{
    int handle = open(g_lock_file, O_CREAT | O_WRONLY, 0600);
    if (handle == -1) {
        error("yabai: could not create lock-file! abort..\n");
    }

    struct flock lockfd = {
        .l_start  = 0,
        .l_len    = 0,
        .l_pid    = getpid(),
        .l_type   = F_WRLCK,
        .l_whence = SEEK_SET
    };

    if (fcntl(handle, F_SETLK, &lockfd) == -1) {
        error("yabai: could not acquire lock-file! abort..\n");
    }
}

static void exec_config_file(void)
{
    struct stat buffer;
    if (stat(g_config_file, &buffer) != 0) {
        error("yabai: config '%s' not found! abort..\n", g_config_file);
    }

    if (buffer.st_mode & S_IFDIR) {
        error("yabai: config '%s' is a directory! abort..\n", g_config_file);
    }

    bool is_executable = buffer.st_mode & S_IXUSR;
    if (!is_executable && chmod(g_config_file, S_IXUSR | buffer.st_mode) != 0) {
        error("yabai: could not set the executable permission bit for config '%s'! abort..\n", g_config_file);
    }

    if (!fork_exec_wait(g_config_file)) {
        error("yabai: failed to execute config '%s'!\n", g_config_file);
    }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline void init_misc_settings(void)
{
    char *user = getenv("USER");
    if (!user) {
        error("yabai: 'env USER' not set! abort..\n");
    }

    char *home = getenv("HOME");
    if (!home) {
        error("yabai: 'env HOME' not set! abort..\n");
    }

    if (!g_config_file[0]) snprintf(g_config_file, sizeof(g_config_file), CONFIG_FILE_FMT, home);
    snprintf(g_sa_socket_file, sizeof(g_sa_socket_file), SA_SOCKET_PATH_FMT, user);
    snprintf(g_socket_file, sizeof(g_socket_file), SOCKET_PATH_FMT, user);
    snprintf(g_lock_file, sizeof(g_lock_file), LCFILE_PATH_FMT, user);

    NSApplicationLoad();
    CGSetLocalEventsSuppressionInterval(0.0f);
    CGEnableEventStateCombining(false);
    g_connection = SLSMainConnectionID();
}
#pragma clang diagnostic pop

static CONNECTION_CALLBACK(connection_handler)
{
    struct event *event;
    event_create(event, MISSION_CONTROL_ENTER, NULL);
    eventloop_post(&g_eventloop, event);
}

static void parse_arguments(int argc, char **argv)
{
    if ((string_equals(argv[1], VERSION_OPT_LONG)) ||
        (string_equals(argv[1], VERSION_OPT_SHRT))) {
        fprintf(stdout, "yabai-v%d.%d.%d\n", MAJOR, MINOR, PATCH);
        exit(EXIT_SUCCESS);
    }

    if ((string_equals(argv[1], CLIENT_OPT_LONG)) ||
        (string_equals(argv[1], CLIENT_OPT_SHRT))) {
        exit(client_send_message(argc-1, argv+1));
    }

    if (string_equals(argv[1], SCRPT_ADD_INSTALL_OPT)) {
        exit(scripting_addition_install());
    }

    if (string_equals(argv[1], SCRPT_ADD_UNINSTALL_OPT)) {
        exit(scripting_addition_uninstall());
    }

    if (string_equals(argv[1], SCRPT_ADD_LOAD_OPT)) {
        exit(scripting_addition_load());
    }

    for (int i = 1; i < argc; ++i) {
        char *opt = argv[i];

        if ((string_equals(opt, DEBUG_VERBOSE_OPT_LONG)) ||
            (string_equals(opt, DEBUG_VERBOSE_OPT_SHRT))) {
            g_verbose = true;
        } else if ((string_equals(opt, CONFIG_OPT_LONG)) ||
                   (string_equals(opt, CONFIG_OPT_SHRT))) {
            char *val = i < argc - 1 ? argv[++i] : NULL;
            if (!val) error("yabai: option '%s|%s' requires an argument!\n", CONFIG_OPT_LONG, CONFIG_OPT_SHRT);
            snprintf(g_config_file, sizeof(g_config_file), "%s", val);
        } else {
            error("yabai: '%s' is not a valid option!\n", opt);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        parse_arguments(argc, argv);
    }

    init_misc_settings();
    acquire_lockfile();

    if (is_root()) {
        error("yabai: running as root is not allowed! abort..\n");
    }

    if (!ax_privilege()) {
        error("yabai: could not access accessibility features! abort..\n");
    }

    if (!space_manager_has_separate_spaces()) {
        error("yabai: 'display has separate spaces' is disabled! abort..\n");
    }

    if (!eventloop_init(&g_eventloop)) {
        error("yabai: could not initialize eventloop! abort..\n");
    }

    if (!socket_daemon_begin_un(&g_daemon, g_socket_file, message_handler)) {
        error("yabai: could not initialize daemon! abort..\n");
    }

    if (scripting_addition_is_installed()) {
        scripting_addition_load();
    }

    process_manager_init(&g_process_manager);
    workspace_event_handler_init(&g_workspace_context);
    space_manager_init(&g_space_manager);
    window_manager_init(&g_window_manager);
    mouse_state_init(&g_mouse_state);

    eventloop_begin(&g_eventloop);
    exec_config_file();

    SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 1204, NULL);
    display_manager_begin(&g_display_manager);
    space_manager_begin(&g_space_manager);
    window_manager_begin(&g_space_manager, &g_window_manager);
    window_manager_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
    process_manager_begin(&g_process_manager);
    workspace_event_handler_begin(&g_workspace_context);
    event_tap_begin(&g_event_tap, EVENT_MASK_MOUSE, mouse_handler);

    CFRunLoopRun();
    return 0;
}

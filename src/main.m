#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "misc/macros.h"
#include "misc/helpers.h"
#include "misc/log.h"
#include "misc/cfstring.h"
#include "misc/sbuffer.h"
#define HASHTABLE_IMPLEMENTATION
#include "misc/hashtable.h"
#undef HASHTABLE_IMPLEMENTATION
#include "misc/socket.h"
#include "misc/socket.c"

#include "dispatch/event.h"
#include "dispatch/eventloop.h"
#include "dispatch/event_tap.h"
#include "dispatch/process.h"
#include "dispatch/workspace.h"

#include "mouse.h"
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
#include "sa.h"

#include "dispatch/event.c"
#include "dispatch/eventloop.c"
#include "dispatch/event_tap.c"
#include "dispatch/process.c"
#include "dispatch/workspace.m"

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
#include "sa.m"

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
bool g_mission_control_active;
int g_connection;
char *g_config;
bool g_verbose;

static int client_send_message(int argc, char **argv)
{
    char *user = getenv("USER");
    if (!user) {
        error("yabai: 'env USER' not set! abort..\n");
    }

    int sockfd;
    char socket_file[255];
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
        *temp++ = ' ';
    }
    *(temp - 1) = '\0';

    if (!socket_write(sockfd, message)) {
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

static bool daemon_init(struct daemon *daemon, socket_daemon_handler *handler)
{
    char *user = getenv("USER");
    if (!user) {
        error("yabai: 'env USER' not set! abort..\n");
    }

    char lock_file[255];
    snprintf(lock_file, sizeof(lock_file), LCFILE_PATH_FMT, user);

    int handle = open(lock_file, O_CREAT | O_WRONLY, 0600);
    if (handle == -1) {
        error("yabai: could not create lock-file! abort..\n");
    }

    if (flock(handle, LOCK_EX | LOCK_NB) == -1) {
        error("yabai: could not acquire lock-file! abort..\n");
    }

    char socket_file[255];
    snprintf(socket_file, sizeof(socket_file), SOCKET_PATH_FMT, user);

    return socket_daemon_begin_un(daemon, socket_file, handler);
}

static void exec_config_file(char *config)
{
    char config_file[BUFSIZ];

    if (!config) {
        char *home = getenv("HOME");
        if (!home) {
            error("yabai: 'env HOME' not set! abort..\n");
        }

        snprintf(config_file, sizeof(config_file), CONFIG_FILE_FMT, home);
        config = config_file;
    }

    struct stat buffer;
    if (stat(config, &buffer) != 0) {
        error("yabai: config '%s' not found!\n", config);
    }

    if (!fork_exec_wait(config)) {
        error("yabai: failed to execute config '%s'!\n", config);
    }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline void init_misc_settings(void)
{
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
            g_config = string_copy(val);
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

    if (is_root()) {
        error("yabai: running as root is not allowed! abort..\n");
    }

    if (!ax_privilege()) {
        error("yabai: could not access accessibility features! abort..\n");
    }

    init_misc_settings();

    if (!space_manager_has_separate_spaces()) {
        error("yabai: 'display has separate spaces' is disabled! abort..\n");
    }

    if (!eventloop_init(&g_eventloop)) {
        error("yabai: could not initialize eventloop! abort..\n");
    }

    if (scripting_addition_is_installed()) {
        scripting_addition_load();
    }

    process_manager_init(&g_process_manager);
    workspace_event_handler_init(&g_workspace_context);
    space_manager_init(&g_space_manager);
    window_manager_init(&g_window_manager);
    mouse_state_init(&g_mouse_state);

    if (!daemon_init(&g_daemon, message_handler)) {
        error("yabai: could not initialize daemon! abort..\n");
    }

    eventloop_begin(&g_eventloop);
    exec_config_file(g_config);

    SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 1204, NULL);
    display_manager_begin(&g_display_manager);
    space_manager_begin(&g_space_manager);
    window_manager_begin(&g_window_manager);
    window_manager_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
    process_manager_begin(&g_process_manager);
    workspace_event_handler_begin(&g_workspace_context);
    event_tap_begin(&g_event_tap, EVENT_MASK_MOUSE, mouse_handler);

    CFRunLoopRun();
    return 0;
}

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

#define CLIENT_OPT_LONG "--msg"
#define CLIENT_OPT_SHRT "-m"

#define DEBUG_VERBOSE_OPT_LONG "--verbose"
#define DEBUG_VERBOSE_OPT_SHRT "-V"
#define VERSION_OPT_LONG       "--version"
#define VERSION_OPT_SHRT       "-v"

#define SCRPT_ADD_INSTALL_OPT   "--install-sa"
#define SCRPT_ADD_UNINSTALL_OPT "--uninstall-sa"
#define SCRPT_ADD_LOAD_OPT      "--load-sa"

#define MAJOR 0
#define MINOR 1
#define PATCH 0

struct eventloop g_eventloop;
void *g_workspace_context;
struct process_manager g_process_manager;
struct display_manager g_display_manager;
struct space_manager g_space_manager;
struct window_manager g_window_manager;
struct event_tap g_event_tap;
struct daemon g_daemon;
int g_connection;
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
    char rsp[BUFSIZ];

    if (socket_wait_read(sockfd, rsp, sizeof(rsp) - 1)) {
        if (rsp[0] == FAILURE_MESSAGE[0]) {
            fprintf(stdout, "%s", rsp + 1);
            result = EXIT_FAILURE;
        } else {
            fprintf(stdout, "%s", rsp);
        }
        fflush(stdout);
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

static void exec_config_file(void)
{
    char *home = getenv("HOME");
    if (!home) {
        error("yabai: 'env HOME' not set! abort..\n");
    }

    char config_file[BUFSIZ];
    snprintf(config_file, sizeof(config_file), CONFIG_FILE_FMT, home);

    struct stat buffer;
    if (stat(config_file, &buffer) != 0) {
        error("yabai: config '%s' not found!\n", config_file);
    }

    if (!fork_exec_wait(config_file)) {
        error("yabai: failed to execute config '%s'!\n", config_file);
    }
}

static bool parse_arguments(int argc, char **argv)
{
    if (argc <= 1) return false;

    if ((strcmp(argv[1], VERSION_OPT_LONG) == 0) ||
        (strcmp(argv[1], VERSION_OPT_SHRT) == 0)) {
        fprintf(stdout, "yabai version %d.%d.%d\n", MAJOR, MINOR, PATCH);
        return true;
    }

    if ((strcmp(argv[1], CLIENT_OPT_LONG) == 0) ||
        (strcmp(argv[1], CLIENT_OPT_SHRT) == 0)) {
        exit(client_send_message(argc-1, argv+1));
    }

    if (strcmp(argv[1], SCRPT_ADD_INSTALL_OPT) == 0) {
        exit(scripting_addition_install());
    }

    if (strcmp(argv[1], SCRPT_ADD_UNINSTALL_OPT) == 0) {
        exit(scripting_addition_uninstall());
    }

    if (strcmp(argv[1], SCRPT_ADD_LOAD_OPT) == 0) {
        exit(scripting_addition_load());
    }

    while (argc > 1) {
        char *opt = argv[--argc];

        if ((strcmp(opt, DEBUG_VERBOSE_OPT_LONG) == 0) ||
            (strcmp(opt, DEBUG_VERBOSE_OPT_SHRT) == 0)) {
            g_verbose = true;
        }
    }

    return false;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
void init_misc_settings(void)
{
    NSApplicationLoad();
    CGSetLocalEventsSuppressionInterval(0.0f);
    CGEnableEventStateCombining(false);
    g_connection = SLSMainConnectionID();
}
#pragma clang diagnostic pop

int main(int argc, char **argv)
{
    if (parse_arguments(argc, argv)) {
        return EXIT_SUCCESS;
    }

    if (is_root()) {
        error("yabai: running as root is not allowed! abort..\n");
    }

    if (!ax_privilege()) {
        error("yabai: could not access accessibility features! abort..\n");
    }

    init_misc_settings();

    if (!space_manager_has_separate_spaces()) {
        error("yabai: 'display has separate spaces' is enabled! abort..\n");
    }

    if (!eventloop_init(&g_eventloop)) {
        error("yabai: could not initialize eventloop! abort..\n");
    }

    process_manager_init(&g_process_manager);
    workspace_event_handler_init(&g_workspace_context);
    space_manager_init(&g_space_manager);
    window_manager_init(&g_window_manager);

    if (!daemon_init(&g_daemon, message_handler)) {
        error("yabai: could not initialize daemon! abort..\n");
    }

    if (scripting_addition_is_installed()) {
        scripting_addition_load();
    }

    eventloop_begin(&g_eventloop);
    exec_config_file();

    display_manager_begin(&g_display_manager);
    space_manager_begin(&g_space_manager);
    window_manager_begin(&g_window_manager);
    window_manager_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
    process_manager_begin(&g_process_manager);
    workspace_event_handler_begin(&g_workspace_context);
    event_tap_begin(&g_event_tap, 1 << kCGEventMouseMoved, mouse_handler);

    CFRunLoopRun();
    return 0;
}

#define SA_SOCKET_PATH_FMT      "/tmp/yabai-sa_%s.socket"
#define SOCKET_PATH_FMT         "/tmp/yabai_%s.socket"
#define LCFILE_PATH_FMT         "/tmp/yabai_%s.lock"

#define CLIENT_OPT_LONG         "--message"
#define CLIENT_OPT_SHRT         "-m"

#define DEBUG_VERBOSE_OPT_LONG  "--verbose"
#define DEBUG_VERBOSE_OPT_SHRT  "-V"
#define VERSION_OPT_LONG        "--version"
#define VERSION_OPT_SHRT        "-v"
#define CONFIG_OPT_LONG         "--config"
#define CONFIG_OPT_SHRT         "-c"

#define SCRPT_ADD_UNINSTALL_OPT "--uninstall-sa"
#define SCRPT_ADD_LOAD_OPT      "--load-sa"

#define MAJOR  5
#define MINOR  0
#define PATCH  3

struct event_loop g_event_loop;
void *g_workspace_context;
struct process_manager g_process_manager;
struct display_manager g_display_manager;
struct space_manager g_space_manager;
struct window_manager g_window_manager;
struct mouse_state g_mouse_state;
struct event_tap g_event_tap;
int g_normal_window_level;
int g_floating_window_level;
int g_connection;
pid_t g_pid;

struct signal *g_signal_event[SIGNAL_TYPE_COUNT];
struct memory_pool g_signal_storage;
int g_mission_control_active;
char g_sa_socket_file[MAXLEN];
char g_socket_file[MAXLEN];
char g_config_file[4096];
char g_lock_file[MAXLEN];
bool g_verbose;

static int client_send_message(int argc, char **argv)
{
    if (argc <= 1) {
        error("yabai-msg: no arguments given! abort..\n");
    }

    char *user = getenv("USER");
    if (!user) {
        error("yabai-msg: 'env USER' not set! abort..\n");
    }

    int message_length = argc;
    int argl[argc];

    for (int i = 1; i < argc; ++i) {
        argl[i] = strlen(argv[i]);
        message_length += argl[i];
    }

    char *message = malloc(sizeof(int)+message_length);
    char *temp = sizeof(int)+message;

    memcpy(message, &message_length, sizeof(int));
    for (int i = 1; i < argc; ++i) {
        memcpy(temp, argv[i], argl[i]);
        temp += argl[i];
        *temp++ = '\0';
    }
    *temp++ = '\0';

    int sockfd;
    char socket_file[MAXLEN];
    snprintf(socket_file, sizeof(socket_file), SOCKET_PATH_FMT, user);

    if (!socket_open(&sockfd)) {
        error("yabai-msg: failed to open socket..\n");
    }

    if (!socket_connect(sockfd, socket_file)) {
        error("yabai-msg: failed to connect to socket..\n");
    }

    if (send(sockfd, message, sizeof(int)+message_length, 0) == -1) {
        error("yabai-msg: failed to send data..\n");
    }

    shutdown(sockfd, SHUT_WR);
    free(message);

    int result = EXIT_SUCCESS;
    FILE *output = stdout;
    int bytes_read = 0;
    char rsp[BUFSIZ];

    while ((bytes_read = read(sockfd, rsp, sizeof(rsp)-1)) > 0) {
        rsp[bytes_read] = '\0';

        if (rsp[0] == FAILURE_MESSAGE[0]) {
            result = EXIT_FAILURE;
            output = stderr;
            fprintf(output, "%s", rsp + 1);
            fflush(output);
        } else {
            fprintf(output, "%s", rsp);
            fflush(output);
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
        .l_pid    = g_pid,
        .l_type   = F_WRLCK,
        .l_whence = SEEK_SET
    };

    if (fcntl(handle, F_SETLK, &lockfd) == -1) {
        error("yabai: could not acquire lock-file! abort..\n");
    }
}

static bool get_config_file(char *restrict filename, char *restrict buffer, int buffer_size)
{
    char *xdg_home = getenv("XDG_CONFIG_HOME");
    if (xdg_home && *xdg_home) {
        snprintf(buffer, buffer_size, "%s/yabai/%s", xdg_home, filename);
        if (file_exists(buffer)) return true;
    }

    char *home = getenv("HOME");
    if (!home) return false;

    snprintf(buffer, buffer_size, "%s/.config/yabai/%s", home, filename);
    if (file_exists(buffer)) return true;

    snprintf(buffer, buffer_size, "%s/.%s", home, filename);
    return file_exists(buffer);
}

static void exec_config_file(void)
{
    if (!*g_config_file && !get_config_file("yabairc", g_config_file, sizeof(g_config_file))) {
        notify("configuration", "could not locate config file..");
        return;
    }

    if (!file_exists(g_config_file)) {
        notify("configuration", "file '%s' does not exist..", g_config_file);
        return;
    }

    if (!ensure_executable_permission(g_config_file)) {
        notify("configuration", "could not set the executable permission bit for '%s'", g_config_file);
        return;
    }

    int pid = fork();
    if (pid == 0) {
        char *exec[] = { "/usr/bin/env", "sh", "-c", g_config_file, NULL};
        exit(execvp(exec[0], exec));
    } else if (pid == -1) {
        notify("configuration", "failed to execute file '%s'", g_config_file);
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

    snprintf(g_sa_socket_file, sizeof(g_sa_socket_file), SA_SOCKET_PATH_FMT, user);
    snprintf(g_socket_file, sizeof(g_socket_file), SOCKET_PATH_FMT, user);
    snprintf(g_lock_file, sizeof(g_lock_file), LCFILE_PATH_FMT, user);

    NSApplicationLoad();
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    CGSetLocalEventsSuppressionInterval(0.0f);
    CGEnableEventStateCombining(false);
    g_pid = getpid();
    g_connection = SLSMainConnectionID();
    g_normal_window_level   = CGWindowLevelForKey(LAYER_NORMAL);
    g_floating_window_level = CGWindowLevelForKey(LAYER_ABOVE);

#if 0
    hook_nsobject_autorelease();
    hook_autoreleasepool_drain();
    hook_autoreleasepool_release();
#endif
}
#pragma clang diagnostic pop

static CONNECTION_CALLBACK(connection_handler)
{
    if (type == 1204) {
        event_loop_post(&g_event_loop, MISSION_CONTROL_ENTER, NULL, 0, NULL);
    } else if (type == 806) {
        event_loop_post(&g_event_loop, SLS_WINDOW_MOVED, (void *) (intptr_t) (*(uint32_t *) data), 0, NULL);
    } else if (type == 807) {
        event_loop_post(&g_event_loop, SLS_WINDOW_RESIZED, (void *) (intptr_t) (* (uint32_t *) data), 0, NULL);
    } else if (type == 808) {
        event_loop_post(&g_event_loop, SLS_WINDOW_ORDER_CHANGED, (void *) (intptr_t) (* (uint32_t *) data), 0, NULL);
    } else if (type == 815) {
        event_loop_post(&g_event_loop, SLS_WINDOW_IS_VISIBLE, (void *) (intptr_t) (* (uint32_t *) data), 0, NULL);
    } else if (type == 816) {
        event_loop_post(&g_event_loop, SLS_WINDOW_IS_INVISIBLE, (void *) (intptr_t) (* (uint32_t *) data), 0, NULL);
    }
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

    if (is_root()) {
        error("yabai: running as root is not allowed! abort..\n");
    }

    if (!ax_privilege()) {
        error("yabai: could not access accessibility features! abort..\n");
    }

    init_misc_settings();
    acquire_lockfile();

    if (!space_manager_has_separate_spaces()) {
        error("yabai: 'display has separate spaces' is disabled! abort..\n");
    }

    if (!event_loop_init(&g_event_loop)) {
        error("yabai: could not initialize event_loop! abort..\n");
    }

    if (!memory_pool_init(&g_signal_storage, KILOBYTES(128))) {
        error("yabai: could not allocate memory for event_signal! abort..\n");
    }

    if (!ts_init(MEGABYTES(4))) {
        error("yabai: could not allocate temporary storage! abort..\n");
    }

    process_manager_init(&g_process_manager);
    workspace_event_handler_init(&g_workspace_context);
    space_manager_init(&g_space_manager);
    window_manager_init(&g_window_manager);
    mouse_state_init(&g_mouse_state);

    event_loop_begin(&g_event_loop);
    display_manager_begin(&g_display_manager);
    space_manager_begin(&g_space_manager);
    window_manager_begin(&g_space_manager, &g_window_manager);
    process_manager_begin(&g_process_manager);
    workspace_event_handler_begin(&g_workspace_context);
    event_tap_begin(&g_event_tap, EVENT_MASK_MOUSE, mouse_handler);

    if (workspace_is_macos_monterey() || workspace_is_macos_ventura()) {
        mission_control_observe();
    } else {
        SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 1204, NULL);
    }

    SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 806, NULL);
    SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 807, NULL);
    SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 808, NULL);
    SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 815, NULL);
    SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 816, NULL);

    if (!message_loop_begin(g_socket_file)) {
        error("yabai: could not initialize message_loop! abort..\n");
    }

    exec_config_file();

    for (;;) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        CFRunLoopRunResult result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 300, true);
        [pool drain];

        if (result == kCFRunLoopRunFinished || result == kCFRunLoopRunStopped) break;
    }

    return 0;
}

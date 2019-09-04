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

#define SCRPT_ADD_INSTALL_OPT   "--install-sa"
#define SCRPT_ADD_UNINSTALL_OPT "--uninstall-sa"
#define SCRPT_ADD_CHECK_OPT     "--check-sa"
#define SCRPT_ADD_LOAD_OPT      "--load-sa"

#define MAJOR 2
#define MINOR 0
#define PATCH 1

#define CONNECTION_CALLBACK(name) void name(uint32_t type, void *data, size_t data_length, void *context, int cid)
typedef CONNECTION_CALLBACK(connection_callback);
extern CGError SLSRegisterConnectionNotifyProc(int cid, connection_callback *handler, uint32_t event, void *context);

struct event_loop g_event_loop;
void *g_workspace_context;
struct process_manager g_process_manager;
struct display_manager g_display_manager;
struct space_manager g_space_manager;
struct window_manager g_window_manager;
struct mouse_state g_mouse_state;
struct event_tap g_event_tap;
struct daemon g_daemon;
struct bar g_bar;
int g_connection;

struct signal *g_signal_event[EVENT_TYPE_COUNT];
bool g_mission_control_active;
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

    int sockfd;
    char socket_file[MAXLEN];
    snprintf(socket_file, sizeof(socket_file), SOCKET_PATH_FMT, user);

    if (!socket_connect_un(&sockfd, socket_file)) {
        error("yabai-msg: failed to connect to socket..\n");
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
        error("yabai-msg: failed to send data..\n");
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
                result = EXIT_FAILURE;
                fprintf(stderr, "%s", rsp + 1);
                fflush(stderr);
            } else {
                fprintf(stdout, "%s", rsp);
                fflush(stdout);
            }
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
    if (*g_config_file || get_config_file("yabairc", g_config_file, sizeof(g_config_file))) {
        if (!file_exists(g_config_file)) {
            error("yabai: specified config file '%s' does not exist! abort..\n", g_config_file);
        }

        if (!ensure_executable_permission(g_config_file)) {
            error("yabai: could not set the executable permission bit for config file '%s'! abort..\n", g_config_file);
        }

        if (!fork_exec(g_config_file, NULL)) {
            error("yabai: failed to execute config file '%s'!\n", g_config_file);
        }
    } else {
        error("yabai: could not locate config file!\n");
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
    CGSetLocalEventsSuppressionInterval(0.0f);
    CGEnableEventStateCombining(false);
    g_connection = SLSMainConnectionID();
}
#pragma clang diagnostic pop

static CONNECTION_CALLBACK(connection_handler)
{
    struct event *event;
    event_create(event, MISSION_CONTROL_ENTER, NULL);
    event_loop_post(&g_event_loop, event);
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

    if (string_equals(argv[1], SCRPT_ADD_CHECK_OPT)) {
        exit(scripting_addition_check());
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

    if (!socket_daemon_begin_un(&g_daemon, g_socket_file, message_handler)) {
        error("yabai: could not initialize daemon! abort..\n");
    }

    process_manager_init(&g_process_manager);
    workspace_event_handler_init(&g_workspace_context);
    space_manager_init(&g_space_manager);
    window_manager_init(&g_window_manager);
    mouse_state_init(&g_mouse_state);
    SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 1204, NULL);

    event_loop_begin(&g_event_loop);
    display_manager_begin(&g_display_manager);
    space_manager_begin(&g_space_manager);
    window_manager_begin(&g_space_manager, &g_window_manager);
    process_manager_begin(&g_process_manager);
    workspace_event_handler_begin(&g_workspace_context);
    event_tap_begin(&g_event_tap, EVENT_MASK_MOUSE, mouse_handler);

    if (scripting_addition_is_installed()) {
        scripting_addition_load();
    } else {
        notify("payload is not installed, some features will not work!", "scripting-addition");
    }

    exec_config_file();
    CFRunLoopRun();
    return 0;
}

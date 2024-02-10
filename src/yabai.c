#define SA_SOCKET_PATH_FMT      "/tmp/yabai-sa_%s.socket"
#define SOCKET_PATH_FMT         "/tmp/yabai_%s.socket"
#define LCFILE_PATH_FMT         "/tmp/yabai_%s.lock"

#define SCRPT_ADD_LOAD_OPT      "--load-sa"
#define SCRPT_ADD_UNINSTALL_OPT "--uninstall-sa"
#define SERVICE_INSTALL_OPT     "--install-service"
#define SERVICE_UNINSTALL_OPT   "--uninstall-service"
#define SERVICE_START_OPT       "--start-service"
#define SERVICE_RESTART_OPT     "--restart-service"
#define SERVICE_STOP_OPT        "--stop-service"
#define CLIENT_OPT_LONG         "--message"
#define CLIENT_OPT_SHRT         "-m"
#define CONFIG_OPT_LONG         "--config"
#define CONFIG_OPT_SHRT         "-c"
#define DEBUG_VERBOSE_OPT_LONG  "--verbose"
#define DEBUG_VERBOSE_OPT_SHRT  "-V"
#define VERSION_OPT_LONG        "--version"
#define VERSION_OPT_SHRT        "-v"
#define HELP_OPT_LONG           "--help"
#define HELP_OPT_SHRT           "-h"

#define MAJOR  6
#define MINOR  0
#define PATCH 10

struct event_loop g_event_loop;
void *g_workspace_context;
struct process_manager g_process_manager;
struct display_manager g_display_manager;
struct space_manager g_space_manager;
struct window_manager g_window_manager;
struct mouse_state g_mouse_state;
double g_cv_host_clock_frequency;
int g_layer_normal_window_level;
int g_layer_below_window_level;
int g_layer_above_window_level;
int g_connection;
pid_t g_pid;

struct signal *g_signal_event[SIGNAL_TYPE_COUNT];
struct memory_pool g_signal_storage;
enum mission_control_mode g_mission_control_mode;
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
static inline bool configure_settings_and_acquire_lock(void)
{
    char *user = getenv("USER");
    if (!user) {
        error("yabai: 'env USER' not set! abort..\n");
    }

    snprintf(g_sa_socket_file, sizeof(g_sa_socket_file), SA_SOCKET_PATH_FMT, user);
    snprintf(g_socket_file, sizeof(g_socket_file), SOCKET_PATH_FMT, user);
    snprintf(g_lock_file, sizeof(g_lock_file), LCFILE_PATH_FMT, user);

    g_pid = getpid();
    g_connection = SLSMainConnectionID();
    g_cv_host_clock_frequency   = CVGetHostClockFrequency();
    g_layer_normal_window_level = CGWindowLevelForKey(LAYER_NORMAL);
    g_layer_below_window_level  = CGWindowLevelForKey(LAYER_BELOW);
    g_layer_above_window_level  = CGWindowLevelForKey(LAYER_ABOVE);
    CGSGetConnectionPortById    = macho_find_symbol("/System/Library/PrivateFrameworks/SkyLight.framework/Versions/A/SkyLight", "_CGSGetConnectionPortById");

    NSApplicationLoad();
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    CGSetLocalEventsSuppressionInterval(0.0f);
    CGEnableEventStateCombining(false);
    mouse_state_init(&g_mouse_state);

#if 0
    hook_nsobject_autorelease();
    hook_autoreleasepool_drain();
    hook_autoreleasepool_release();
    window_dump_cg_window_levels();
#endif

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

    return fcntl(handle, F_SETLK, &lockfd) != -1;
}
#pragma clang diagnostic pop

__attribute__((no_sanitize("undefined")))
static CONNECTION_CALLBACK(connection_handler)
{
    //
    // NOTE(koekeishiya): Disable undefined sanitizer for this particular function.
    // It will sometimes report load of misaligned address when reading from the
    // data buffer, but there is nothing for us to do here because said memory is
    // allocated and managed by macOS.
    //

    if (type == 1204) {
        event_loop_post(&g_event_loop, MISSION_CONTROL_ENTER, NULL, 0);
    } else if (type == 1325) {
        event_loop_post(&g_event_loop, SLS_SPACE_ADD_WINDOW, (void *) (intptr_t) (* (uint64_t *) data), (int) (* (uint32_t *) (data + 8)));
    } else if (type == 1326) {
        event_loop_post(&g_event_loop, SLS_SPACE_REMOVE_WINDOW, (void *) (intptr_t) (* (uint64_t *) data), (int) (* (uint32_t *) (data + 8)));
    } else if (type == 1327) {
        event_loop_post(&g_event_loop, SLS_SPACE_CREATED, (void *) (intptr_t) (* (uint64_t *) data), 0);
    } else if (type == 1328) {
        event_loop_post(&g_event_loop, SLS_SPACE_DESTROYED, (void *) (intptr_t) (* (uint64_t *) data), 0);
    }
}

static void parse_arguments(int argc, char **argv)
{
    if ((string_equals(argv[1], HELP_OPT_LONG)) ||
        (string_equals(argv[1], HELP_OPT_SHRT))) {
        fprintf(stdout, "Usage: yabai [option]\n"
                        "Options:\n"
                        "    --load-sa              Install and load the scripting-addition.\n"
                        "    --uninstall-sa         Uninstall the scripting-addition.\n"
                        "    --install-service      Write launchd service file to disk.\n"
                        "    --uninstall-service    Remove launchd service file from disk.\n"
                        "    --start-service        Enable, load, and start the launchd service.\n"
                        "    --restart-service      Attempts to restart the service instance.\n"
                        "    --stop-service         Stops a running instance of the service.\n"
                        "    --message, -m <msg>    Send message to a running instance of yabai.\n"
                        "    --config, -c <config>  Use the specified configuration file.\n"
                        "    --verbose, -V          Output debug information to stdout.\n"
                        "    --version, -v          Print version to stdout and exit.\n"
                        "    --help, -h             Print options to stdout and exit.\n"
                        "Type `man yabai` for more information, or visit: "
                        "https://github.com/koekeishiya/yabai/blob/v%d.%d.%d/doc/yabai.asciidoc\n", MAJOR, MINOR, PATCH);
        exit(EXIT_SUCCESS);
    }

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

    if (string_equals(argv[1], SERVICE_INSTALL_OPT)) {
        exit(service_install());
    }

    if (string_equals(argv[1], SERVICE_UNINSTALL_OPT)) {
        exit(service_uninstall());
    }

    if (string_equals(argv[1], SERVICE_START_OPT)) {
        exit(service_start());
    }

    if (string_equals(argv[1], SERVICE_RESTART_OPT)) {
        exit(service_restart());
    }

    if (string_equals(argv[1], SERVICE_STOP_OPT)) {
        exit(service_stop());
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
        require("yabai: running as root is not allowed! abort..\n");
    }

    if (!ax_privilege()) {
        require("yabai: could not access accessibility features! abort..\n");
    }

    if (!(SLSGetSpaceManagementMode(SLSMainConnectionID()) == 1)) {
        require("yabai: 'display has separate spaces' is disabled! abort..\n");
    }

    if (!ts_init(MEGABYTES(4))) {
        error("yabai: could not allocate temporary storage! abort..\n");
    }

    if (!memory_pool_init(&g_signal_storage, KILOBYTES(128))) {
        error("yabai: could not allocate event signal storage! abort..\n");
    }

    if (!configure_settings_and_acquire_lock()) {
        error("yabai: could not acquire lock-file! abort..\n");
    }

    if (!event_loop_begin(&g_event_loop)) {
        error("yabai: could not start event loop! abort..\n");
    }

    if (!workspace_event_handler_begin(&g_workspace_context)) {
        error("yabai: could not start workspace context! abort..\n");
    }

    if (!process_manager_begin(&g_process_manager)) {
        error("yabai: could not start process manager! abort..\n");
    }

    if (!display_manager_begin(&g_display_manager)) {
        error("yabai: could not start display manager! abort..\n");
    }

    if (!mouse_handler_begin(&g_mouse_state, MOUSE_EVENT_MASK)) {
        error("yabai: could not start mouse handler! abort..\n");
    }

    if (workspace_is_macos_monterey() || workspace_is_macos_ventura() || workspace_is_macos_sonoma()) {
        mission_control_observe();
    } else {
        SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 1204, NULL);
    }

    if (workspace_is_macos_ventura() || workspace_is_macos_sonoma()) {
        SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 1325, NULL);
        SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 1326, NULL);
        SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 1327, NULL);
        SLSRegisterConnectionNotifyProc(g_connection, connection_handler, 1328, NULL);
    }

    window_manager_init(&g_window_manager);
    space_manager_begin(&g_space_manager);
    window_manager_begin(&g_space_manager, &g_window_manager);

    if (!message_loop_begin(g_socket_file)) {
        error("yabai: could not start message loop! abort..\n");
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

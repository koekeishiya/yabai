extern struct signal *g_signal_event[SIGNAL_TYPE_COUNT];
extern struct memory_pool g_signal_storage;
extern struct process_manager g_process_manager;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;

static bool event_signal_filter(struct event_signal *es, struct signal *signal)
{
    switch (es->type) {
    default: return false;

    case SIGNAL_APPLICATION_LAUNCHED:
    case SIGNAL_APPLICATION_ACTIVATED:
    case SIGNAL_APPLICATION_DEACTIVATED:
    case SIGNAL_APPLICATION_VISIBLE: {
        int regex_match_app = signal->app_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
        return regex_match(signal->app_regex_valid, &signal->app_regex, es->app) == regex_match_app;
    } break;
    case SIGNAL_APPLICATION_TERMINATED:
    case SIGNAL_APPLICATION_HIDDEN:
    case SIGNAL_WINDOW_DESTROYED: {
        int regex_match_app = signal->app_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
        bool app_no_match = regex_match(signal->app_regex_valid, &signal->app_regex, es->app) == regex_match_app;

        bool active = signal->active == SIGNAL_PROP_UD;
        if (!active) active = es->active == (signal->active == SIGNAL_PROP_YES);

        return app_no_match || !active;
    } break;
    case SIGNAL_WINDOW_CREATED:
    case SIGNAL_WINDOW_FOCUSED:
    case SIGNAL_WINDOW_DEMINIMIZED: {
        int regex_match_app = signal->app_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
        bool app_no_match = regex_match(signal->app_regex_valid, &signal->app_regex, es->app) == regex_match_app;

        int regex_match_title = signal->title_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
        bool title_no_match = regex_match(signal->title_regex_valid, &signal->title_regex, es->title) == regex_match_title;

        return app_no_match || title_no_match;
    } break;
    case SIGNAL_WINDOW_MOVED:
    case SIGNAL_WINDOW_RESIZED:
    case SIGNAL_WINDOW_MINIMIZED:
    case SIGNAL_WINDOW_TITLE_CHANGED: {
        int regex_match_app = signal->app_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
        bool app_no_match = regex_match(signal->app_regex_valid, &signal->app_regex, es->app) == regex_match_app;

        int regex_match_title = signal->title_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
        bool title_no_match = regex_match(signal->title_regex_valid, &signal->title_regex, es->title) == regex_match_title;

        bool active = signal->active == SIGNAL_PROP_UD;
        if (!active) active = es->active == (signal->active == SIGNAL_PROP_YES);

        return app_no_match || title_no_match || !active;
    } break;
    }
}

void event_signal_flush(void)
{
    if (!g_signal_storage.used) return;

    pid_t pid = fork();
    if (pid) {
        g_signal_storage.used = 0;
        return;
    }

    uint64_t size  = sizeof(struct event_signal);
    uint64_t count = g_signal_storage.used / size;

    for (int i = 0; i < count; ++i) {
        struct event_signal *es = g_signal_storage.memory + (i * size);

        int signal_count = buf_len(g_signal_event[es->type]);
        debug("%s: transmitting %s to %d subscriber(s)\n", __FUNCTION__, signal_type_str[es->type], signal_count);

        for (int j = 0; j < signal_count; ++j) {
            struct signal *signal = &g_signal_event[es->type][j];
            if (event_signal_filter(es, signal)) continue;

            int pid = fork();
            if (pid) continue;

            if (es->arg_name[0]) setenv(es->arg_name[0], es->arg_value[0], 1);
            if (es->arg_name[1]) setenv(es->arg_name[1], es->arg_value[1], 1);

            char *exec[] = { "/usr/bin/env", "sh", "-c", signal->command, NULL};
            exit(execvp(exec[0], exec));
        }
    }

    exit(EXIT_SUCCESS);
}

void event_signal_push(enum signal_type type, void *context)
{
    int signal_count = buf_len(g_signal_event[type]);
    if (!signal_count) return;

    uint64_t arg_size = 128;
    uint64_t size = sizeof(struct event_signal);

    uint64_t used = __sync_fetch_and_add(&g_signal_storage.used, size);
    struct event_signal *es = g_signal_storage.memory + used;

    es->type         = type;
    es->arg_name[0]  = NULL;
    es->arg_name[1]  = NULL;
    es->arg_value[0] = NULL;
    es->arg_value[1] = NULL;

    switch (type) {
    default: break;

    case SIGNAL_APPLICATION_LAUNCHED:
    case SIGNAL_APPLICATION_ACTIVATED:
    case SIGNAL_APPLICATION_DEACTIVATED:
    case SIGNAL_APPLICATION_VISIBLE: {
        struct application *application = context;

        es->arg_name[0]  = ts_alloc_unaligned(arg_size);
        es->arg_value[0] = ts_alloc_unaligned(arg_size);

        snprintf(es->arg_name[0],  arg_size, "%s", "YABAI_PROCESS_ID");
        snprintf(es->arg_value[0], arg_size, "%d", application->pid);

        es->app = application->name;
    } break;
    case SIGNAL_APPLICATION_TERMINATED: {
        struct application *application = context;

        es->arg_name[0]  = ts_alloc_unaligned(arg_size);
        es->arg_value[0] = ts_alloc_unaligned(arg_size);

        snprintf(es->arg_name[0],  arg_size, "%s", "YABAI_PROCESS_ID");
        snprintf(es->arg_value[0], arg_size, "%d", application->pid);

        es->app = ts_string_copy(application->name);

        //
        // NOTE(koekeishiya): We always receive an application_front_switched event *before* an application_terminated
        // event. We need to know the difference between a front_switched + application_terminated sequence and a regular
        // application switch followed by a user-initiated termination of the previously focused application. The system
        // events are triggered within an interval that appear to be impossible to match even with a user automated sequence.
        // The dt threshold below is triple the average interval computed, to allow for some leeway.
        //

        EventTime dt = GetCurrentEventTime() - g_process_manager.switch_event_time;
        if (dt >= 0.05f) {
            es->active = g_process_manager.front_pid == application->pid;
        } else {
            es->active = g_process_manager.last_front_pid == application->pid;
        }
    } break;
    case SIGNAL_APPLICATION_FRONT_SWITCHED: {
        es->arg_name[0]  = ts_alloc_unaligned(arg_size);
        es->arg_value[0] = ts_alloc_unaligned(arg_size);
        es->arg_name[1]  = ts_alloc_unaligned(arg_size);
        es->arg_value[1] = ts_alloc_unaligned(arg_size);

        snprintf(es->arg_name[0],  arg_size, "%s", "YABAI_PROCESS_ID");
        snprintf(es->arg_value[0], arg_size, "%d", g_process_manager.front_pid);
        snprintf(es->arg_name[1],  arg_size, "%s", "YABAI_RECENT_PROCESS_ID");
        snprintf(es->arg_value[1], arg_size, "%d", g_process_manager.last_front_pid);
    } break;
    case SIGNAL_APPLICATION_HIDDEN: {
        struct application *application = context;

        es->arg_name[0]  = ts_alloc_unaligned(arg_size);
        es->arg_value[0] = ts_alloc_unaligned(arg_size);

        snprintf(es->arg_name[0],  arg_size, "%s", "YABAI_PROCESS_ID");
        snprintf(es->arg_value[0], arg_size, "%d", application->pid);

        es->app = application->name;
        es->active = g_process_manager.front_pid == application->pid;
    } break;
    case SIGNAL_WINDOW_CREATED:
    case SIGNAL_WINDOW_FOCUSED:
    case SIGNAL_WINDOW_DEMINIMIZED: {
        struct window *window = context;

        es->arg_name[0]  = ts_alloc_unaligned(arg_size);
        es->arg_value[0] = ts_alloc_unaligned(arg_size);

        snprintf(es->arg_name[0],  arg_size, "%s", "YABAI_WINDOW_ID");
        snprintf(es->arg_value[0], arg_size, "%d", window->id);

        es->app   = window->application->name;
        es->title = window_title_ts(window);
    } break;
    case SIGNAL_WINDOW_DESTROYED: {
        struct window *window = context;

        es->arg_name[0]  = ts_alloc_unaligned(arg_size);
        es->arg_value[0] = ts_alloc_unaligned(arg_size);

        snprintf(es->arg_name[0],  arg_size, "%s", "YABAI_WINDOW_ID");
        snprintf(es->arg_value[0], arg_size, "%d", window->id);

        es->app = ts_string_copy(window->application->name);
        es->active = g_window_manager.focused_window_id == window->id;
    } break;
    case SIGNAL_WINDOW_MOVED:
    case SIGNAL_WINDOW_RESIZED:
    case SIGNAL_WINDOW_MINIMIZED:
    case SIGNAL_WINDOW_TITLE_CHANGED: {
        struct window *window = context;

        es->arg_name[0]  = ts_alloc_unaligned(arg_size);
        es->arg_value[0] = ts_alloc_unaligned(arg_size);

        snprintf(es->arg_name[0],  arg_size, "%s", "YABAI_WINDOW_ID");
        snprintf(es->arg_value[0], arg_size, "%d", window->id);

        es->app   = window->application->name;
        es->title = window_title_ts(window);
        es->active = g_window_manager.focused_window_id == window->id;
    } break;
    case SIGNAL_SPACE_CHANGED: {
        es->arg_name[0]  = ts_alloc_unaligned(arg_size);
        es->arg_value[0] = ts_alloc_unaligned(arg_size);
        es->arg_name[1]  = ts_alloc_unaligned(arg_size);
        es->arg_value[1] = ts_alloc_unaligned(arg_size);

        snprintf(es->arg_name[0],  arg_size, "%s",   "YABAI_SPACE_ID");
        snprintf(es->arg_value[0], arg_size, "%lld", g_space_manager.current_space_id);
        snprintf(es->arg_name[1],  arg_size, "%s",   "YABAI_RECENT_SPACE_ID");
        snprintf(es->arg_value[1], arg_size, "%lld", g_space_manager.last_space_id);
    } break;
    case SIGNAL_DISPLAY_ADDED:
    case SIGNAL_DISPLAY_REMOVED:
    case SIGNAL_DISPLAY_MOVED:
    case SIGNAL_DISPLAY_RESIZED: {
        uint32_t did = (uint32_t)(uintptr_t) context;

        es->arg_name[0]  = ts_alloc_unaligned(arg_size);
        es->arg_value[0] = ts_alloc_unaligned(arg_size);

        snprintf(es->arg_name[0],  arg_size, "%s", "YABAI_DISPLAY_ID");
        snprintf(es->arg_value[0], arg_size, "%d", did);
    } break;
    case SIGNAL_DISPLAY_CHANGED: {
        es->arg_name[0]  = ts_alloc_unaligned(arg_size);
        es->arg_value[0] = ts_alloc_unaligned(arg_size);
        es->arg_name[1]  = ts_alloc_unaligned(arg_size);
        es->arg_value[1] = ts_alloc_unaligned(arg_size);

        snprintf(es->arg_name[0],  arg_size, "%s", "YABAI_DISPLAY_ID");
        snprintf(es->arg_value[0], arg_size, "%d", g_display_manager.current_display_id);
        snprintf(es->arg_name[1],  arg_size, "%s", "YABAI_RECENT_DISPLAY_ID");
        snprintf(es->arg_value[1], arg_size, "%d", g_display_manager.last_display_id);
    } break;
    }
}

enum signal_type signal_type_from_string(const char *str)
{
    for (int i = SIGNAL_APPLICATION_LAUNCHED; i < SIGNAL_TYPE_COUNT; ++i) {
        if (string_equals(str, signal_type_str[i])) return i;
    }

    return SIGNAL_TYPE_UNKNOWN;
}

void event_signal_add(enum signal_type type, struct signal *signal)
{
    if (signal->label) event_signal_remove(signal->label);
    buf_push(g_signal_event[type], *signal);
}

void event_signal_destroy(struct signal *signal)
{
    if (signal->app_regex_valid)   regfree(&signal->app_regex);
    if (signal->title_regex_valid) regfree(&signal->title_regex);
    if (signal->command) free(signal->command);
    if (signal->label)   free(signal->label);
    if (signal->app)     free(signal->app);
    if (signal->title)   free(signal->title);
}

bool event_signal_remove_by_index(int index)
{
    int signal_index = 0;
    for (int i = SIGNAL_APPLICATION_LAUNCHED; i < SIGNAL_TYPE_COUNT; ++i) {
        for (int j = 0; j < buf_len(g_signal_event[i]); ++j) {
            if (signal_index == index) {
                event_signal_destroy(&g_signal_event[i][j]);
                buf_del(g_signal_event[i], j);
                return true;
            }
            ++signal_index;
        }
    }

    return false;
}

bool event_signal_remove(char *label)
{
    for (int i = SIGNAL_APPLICATION_LAUNCHED; i < SIGNAL_TYPE_COUNT; ++i) {
        for (int j = 0; j < buf_len(g_signal_event[i]); ++j) {
            if (string_equals(label, g_signal_event[i][j].label)) {
                event_signal_destroy(&g_signal_event[i][j]);
                buf_del(g_signal_event[i], j);
                return true;
            }
        }
    }

    return false;
}

static void event_signal_serialize(FILE *rsp, struct signal *signal, enum signal_type type, int index)
{
    char *escaped_action = ts_string_escape(signal->command);

    fprintf(rsp,
            "{\n"
            "\t\"index\":%d,\n"
            "\t\"label\":\"%s\",\n"
            "\t\"app\":\"%s\",\n"
            "\t\"title\":\"%s\",\n"
            "\t\"active\":%s,\n"
            "\t\"event\":\"%s\",\n"
            "\t\"action\":\"%s\"\n"
            "}",
            index,
            signal->label ? signal->label : "",
            signal->app ? signal->app : "",
            signal->title ? signal->title : "",
            json_optional_bool(signal->active),
            signal_type_str[type],
            escaped_action ? escaped_action : signal->command ? signal->command : "");
}

void event_signal_list(FILE *rsp)
{
    fprintf(rsp, "[");
    int signal_index = 0;
    bool event_did_output = false;
    for (int i = SIGNAL_APPLICATION_LAUNCHED; i < SIGNAL_TYPE_COUNT; ++i) {
        int count = buf_len(g_signal_event[i]);

        if (count > 0 && event_did_output) {
            fprintf(rsp, ",");
        }

        for (int j = 0; j < count; ++j) {
            event_signal_serialize(rsp, &g_signal_event[i][j], i, signal_index);
            if (j < buf_len(g_signal_event[i]) - 1) fprintf(rsp, ",");
            ++signal_index;
        }

        if (!event_did_output) event_did_output = count > 0;
    }
    fprintf(rsp, "]\n");
}

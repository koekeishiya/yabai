#include "event_signal.h"

extern struct signal *g_signal_event[EVENT_TYPE_COUNT];
extern struct process_manager g_process_manager;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;

static void event_signal_serialize(FILE *rsp, struct signal *signal, enum event_type type, int index)
{
    char *escaped_action = string_escape(signal->command);

    fprintf(rsp,
            "{\n"
            "\t\"index\":%d,\n"
            "\t\"label\":\"%s\",\n"
            "\t\"app\":\"%s\",\n"
            "\t\"title\":\"%s\",\n"
            "\t\"event\":\"%s\",\n"
            "\t\"action\":\"%s\"\n"
            "}",
            index,
            signal->label ? signal->label : "",
            signal->app ? signal->app : "",
            signal->title ? signal->title : "",
            event_type_str[type],
            escaped_action ? escaped_action : signal->command ? signal->command : "");

    if (escaped_action) free(escaped_action);
}

static void event_signal_populate_args(void *context, enum event_type type, struct signal_args *args)
{
    switch (type) {
    default: break;

    case APPLICATION_LAUNCHED:
    case APPLICATION_TERMINATED: {
        struct process *process = context;
        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_PROCESS_ID");
        snprintf(args->value[0], sizeof(args->value[0]), "%d", process->pid);
        args->entity = process;
    } break;
    case APPLICATION_FRONT_SWITCHED: {
        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_PROCESS_ID");
        snprintf(args->value[0], sizeof(args->value[0]), "%d", g_process_manager.front_pid);

        snprintf(args->name[1], sizeof(args->name[1]), "%s", "YABAI_RECENT_PROCESS_ID");
        snprintf(args->value[1], sizeof(args->value[1]), "%d", g_process_manager.last_front_pid);
    } break;
    case APPLICATION_ACTIVATED:
    case APPLICATION_DEACTIVATED:
    case APPLICATION_VISIBLE:
    case APPLICATION_HIDDEN: {
        pid_t pid = (pid_t)(uintptr_t) context;
        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_PROCESS_ID");
        snprintf(args->value[0], sizeof(args->value[0]), "%d", pid);
        args->entity = window_manager_find_application(&g_window_manager, pid);
    } break;
    case WINDOW_CREATED: {
        uint32_t wid = ax_window_id(context);
        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_WINDOW_ID");
        snprintf(args->value[0], sizeof(args->value[0]), "%d", wid);
        args->entity = window_manager_find_window(&g_window_manager, wid);
        if (args->entity) args->param1 = window_title(args->entity);
    } break;
    case WINDOW_DESTROYED: {
        uint32_t wid = (uint32_t)(uintptr_t) context;
        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_WINDOW_ID");
        snprintf(args->value[0], sizeof(args->value[0]), "%d", wid);
    } break;
    case WINDOW_FOCUSED:
    case WINDOW_MOVED:
    case WINDOW_RESIZED:
    case WINDOW_MINIMIZED:
    case WINDOW_DEMINIMIZED:
    case WINDOW_TITLE_CHANGED: {
        uint32_t wid = (uint32_t)(uintptr_t) context;
        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_WINDOW_ID");
        snprintf(args->value[0], sizeof(args->value[0]), "%d", wid);
        args->entity = window_manager_find_window(&g_window_manager, wid);
        if (args->entity) args->param1 = window_title(args->entity);
    } break;
    case SPACE_CHANGED: {
        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_SPACE_ID");
        snprintf(args->value[0], sizeof(args->value[0]), "%lld", g_space_manager.current_space_id);

        snprintf(args->name[1], sizeof(args->name[1]), "%s", "YABAI_RECENT_SPACE_ID");
        snprintf(args->value[1], sizeof(args->value[1]), "%lld", g_space_manager.last_space_id);
    } break;
    case DISPLAY_ADDED:
    case DISPLAY_REMOVED:
    case DISPLAY_MOVED:
    case DISPLAY_RESIZED: {
        uint32_t did = (uint32_t)(uintptr_t) context;
        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_DISPLAY_ID");
        snprintf(args->value[0], sizeof(args->value[0]), "%d", did);
    } break;
    case DISPLAY_CHANGED: {
        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_DISPLAY_ID");
        snprintf(args->value[0], sizeof(args->value[0]), "%d", g_display_manager.current_display_id);

        snprintf(args->name[1], sizeof(args->name[1]), "%s", "YABAI_RECENT_DISPLAY_ID");
        snprintf(args->value[1], sizeof(args->value[1]), "%d", g_display_manager.last_display_id);
    } break;
    case MOUSE_DOWN:
    case MOUSE_UP:
    case MOUSE_DRAGGED:
    case MOUSE_MOVED: {
        CGPoint point = CGEventGetLocation(context);
        int64_t button = CGEventGetIntegerValueField(context, kCGMouseEventButtonNumber);

        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_BUTTON");
        snprintf(args->value[0], sizeof(args->value[0]), "%lld", button);

        snprintf(args->name[1], sizeof(args->name[1]), "%s", "YABAI_POINT");
        snprintf(args->value[1], sizeof(args->value[1]), "%f %f", point.x, point.y);
    } break;
    }
}

static void event_signal_destroy_args(enum event_type type, struct signal_args *args)
{
    switch (type) {
    default: break;

    case WINDOW_CREATED:
    case WINDOW_FOCUSED:
    case WINDOW_MOVED:
    case WINDOW_RESIZED:
    case WINDOW_MINIMIZED:
    case WINDOW_DEMINIMIZED:
    case WINDOW_TITLE_CHANGED: {
        if (args->param1) free(args->param1);
    } break;
    }
}

static bool event_signal_filter(struct signal *signal, enum event_type type, struct signal_args *args)
{
    switch (type) {
    default: return false;

    case APPLICATION_LAUNCHED:
    case APPLICATION_TERMINATED: {
        struct process *process = args->entity;
        if (!process) return true;

        int regex_match_app = signal->app_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
        return regex_match(signal->app_regex_valid, &signal->app_regex, process->name) == regex_match_app;
    } break;
    case APPLICATION_ACTIVATED:
    case APPLICATION_DEACTIVATED:
    case APPLICATION_VISIBLE:
    case APPLICATION_HIDDEN: {
        struct application *application = args->entity;
        if (!application) return true;

        int regex_match_app = signal->app_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
        return regex_match(signal->app_regex_valid, &signal->app_regex, application->name) == regex_match_app;
    } break;
    case WINDOW_CREATED:
    case WINDOW_FOCUSED:
    case WINDOW_MOVED:
    case WINDOW_RESIZED:
    case WINDOW_MINIMIZED:
    case WINDOW_DEMINIMIZED:
    case WINDOW_TITLE_CHANGED: {
        struct window *window = args->entity;
        if (!window) return true;

        int regex_match_app = signal->app_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
        bool app_no_match = regex_match(signal->app_regex_valid,   &signal->app_regex,   window->application->name) == regex_match_app;

        int regex_match_title = signal->title_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
        bool title_no_match = regex_match(signal->title_regex_valid, &signal->title_regex, args->param1)            == regex_match_title;

        return app_no_match || title_no_match;
    } break;
    }
}

void event_signal_transmit(void *context, enum event_type type)
{
    int signal_count = buf_len(g_signal_event[type]);
    if (!signal_count) return;

    struct signal_args args = {};
    event_signal_populate_args(context, type, &args);
    debug("%s: transmitting %s to %d subscriber(s)\n", __FUNCTION__, event_type_str[type], signal_count);

    if (fork() != 0) {
        event_signal_destroy_args(type, &args);
        return;
    }

    for (int i = 0; i < signal_count; ++i) {
        struct signal *signal = &g_signal_event[type][i];
        if (!event_signal_filter(signal, type, &args)) {
            fork_exec(signal->command, &args);
        }
    }

    exit(EXIT_SUCCESS);
}

enum event_type event_signal_type_from_string(const char *str)
{
    for (int i = APPLICATION_LAUNCHED; i < EVENT_TYPE_COUNT; ++i) {
        if (string_equals(str, event_type_str[i])) return i;
    }

    return EVENT_TYPE_UNKNOWN;
}

void event_signal_add(enum event_type type, struct signal *signal)
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
    for (int i = 0; i < EVENT_TYPE_COUNT; ++i) {
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
    for (int i = 0; i < EVENT_TYPE_COUNT; ++i) {
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

void event_signal_list(FILE *rsp)
{
    fprintf(rsp, "[");
    int signal_index = 0;
    bool event_did_output = false;
    for (int i = 0; i < EVENT_TYPE_COUNT; ++i) {
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

#include "event.h"

extern struct signal *g_signal_event[EVENT_TYPE_COUNT];
extern struct event_loop g_event_loop;
extern struct process_manager g_process_manager;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;
extern struct mouse_state g_mouse_state;
extern struct bar g_bar;
extern bool g_mission_control_active;
extern int g_connection;

static bool event_signal_filter(struct signal *signal, enum event_type type, struct signal_args *args)
{
    switch (type) {
    default: return false;

    case APPLICATION_LAUNCHED:
    case APPLICATION_TERMINATED: {
        struct process *process = args->entity;
        if (!process) return true;

        return regex_match(signal->app_regex_valid, &signal->app_regex, process->name) == REGEX_MATCH_NO;
    } break;
    case APPLICATION_ACTIVATED:
    case APPLICATION_DEACTIVATED:
    case APPLICATION_VISIBLE:
    case APPLICATION_HIDDEN: {
        struct application *application = args->entity;
        if (!application) return true;

        return regex_match(signal->app_regex_valid, &signal->app_regex, application->name) == REGEX_MATCH_NO;
    } break;
    case WINDOW_CREATED:
    case WINDOW_DESTROYED:
    case WINDOW_FOCUSED:
    case WINDOW_MOVED:
    case WINDOW_RESIZED:
    case WINDOW_MINIMIZED:
    case WINDOW_DEMINIMIZED:
    case WINDOW_TITLE_CHANGED: {
        struct window *window = args->entity;
        if (!window) return true;

        return regex_match(signal->app_regex_valid,   &signal->app_regex,   window->application->name) == REGEX_MATCH_NO ||
               regex_match(signal->title_regex_valid, &signal->title_regex, window_title(window))      == REGEX_MATCH_NO;
    } break;
    }
}

static void event_signal_populate_args(void *context, enum event_type type, struct signal_args *args)
{
    switch (type) {
    default: {} break;

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
    } break;
    case WINDOW_FOCUSED:
    case WINDOW_MOVED:
    case WINDOW_RESIZED:
    case WINDOW_MINIMIZED:
    case WINDOW_DEMINIMIZED:
    case WINDOW_TITLE_CHANGED:
    case WINDOW_DESTROYED: {
        uint32_t wid = (uint32_t)(uintptr_t) context;
        snprintf(args->name[0], sizeof(args->name[0]), "%s", "YABAI_WINDOW_ID");
        snprintf(args->value[0], sizeof(args->value[0]), "%d", wid);
        args->entity = window_manager_find_window(&g_window_manager, wid);
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

void event_signal_transmit(void *context, enum event_type type)
{
    int signal_count = buf_len(g_signal_event[type]);
    if (!signal_count) return;

    struct signal_args args = {};
    event_signal_populate_args(context, type, &args);

    if (fork() != 0) return;
    debug("%s: %s\n", __FUNCTION__, event_type_str[type]);

    for (int i = 0; i < signal_count; ++i) {
        struct signal *signal = &g_signal_event[type][i];
        if (!event_signal_filter(signal, type, &args)) {
            fork_exec(signal->command, &args);
        }
    }

    exit(EXIT_SUCCESS);
}

void event_signal_add(enum event_type type, struct signal signal)
{
    if (signal.label) event_signal_remove(signal.label);
    buf_push(g_signal_event[type], signal);
}

bool event_signal_remove(char *label)
{
    for (int i = 0; i < EVENT_TYPE_COUNT; ++i) {
        for (int j = 0; j < buf_len(g_signal_event[i]); ++j) {
            if (string_equals(label, g_signal_event[i][j].label)) {
                if (g_signal_event[i][j].app_regex_valid)   regfree(&g_signal_event[i][j].app_regex);
                if (g_signal_event[i][j].title_regex_valid) regfree(&g_signal_event[i][j].title_regex);
                free(g_signal_event[i][j].command);
                free(g_signal_event[i][j].label);
                buf_del(g_signal_event[i], j);
                return true;
            }
        }
    }

    return false;
}

enum event_type event_type_from_string(const char *str)
{
    for (int i = APPLICATION_LAUNCHED; i < EVENT_TYPE_COUNT; ++i) {
        if (string_equals(str, event_type_str[i])) return i;
    }

    return EVENT_TYPE_UNKNOWN;
}

void event_destroy(struct event *event)
{
    switch (event->type) {
    default: {} break;
    case APPLICATION_TERMINATED: {
        process_destroy(event->context);
    } break;
    case WINDOW_CREATED: {
        CFRelease(event->context);
    } break;
    case MOUSE_DOWN:
    case MOUSE_UP:
    case MOUSE_DRAGGED:
    case MOUSE_MOVED: {
        CFRelease(event->context);
    } break;
    }

    free(event);
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_LAUNCHED)
{
    struct process *process = context;
    debug("%s: %s\n", __FUNCTION__, process->name);

    if ((process->terminated) || (kill(process->pid, 0) == -1)) {
        debug("%s: %s terminated during launch\n", __FUNCTION__, process->name);
        window_manager_remove_lost_front_switched_event(&g_window_manager, process->pid);
        return EVENT_FAILURE;
    }

    struct application *application = application_create(process);
    if (application_observe(application)) {
        window_manager_add_application(&g_window_manager, application);
        window_manager_add_application_windows(&g_space_manager, &g_window_manager, application);

        int window_count = 0;
        uint32_t prev_window_id = g_window_manager.focused_window_id;
        struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);
        if (!window_list) goto end;

        for (int i = 0; i < window_count; ++i) {
            struct window *window = window_list[i];
            if (window) {
                if (window_manager_should_manage_window(window)) {
                    struct view *view = space_manager_tile_window_on_space_with_insertion_point(&g_space_manager, window, window_space(window), prev_window_id);
                    window_manager_add_managed_window(&g_window_manager, window, view);
                    prev_window_id = window->id;
                }
            } else {
                prev_window_id = 0;
            }
        }

        free(window_list);

end:
        if (window_manager_find_lost_front_switched_event(&g_window_manager, process->pid)) {
            struct event *event;
            event_create(event, APPLICATION_FRONT_SWITCHED, process);
            event_loop_post(&g_event_loop, event);
            window_manager_remove_lost_front_switched_event(&g_window_manager, process->pid);
        }

        return EVENT_SUCCESS;
    } else {
        bool retry_ax = application->retry;
        application_unobserve(application);
        application_destroy(application);
        debug("%s: could not observe %s (%d)\n", __FUNCTION__, process->name, retry_ax);

        if (retry_ax) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.01f * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
                struct event *event;
                event_create(event, APPLICATION_LAUNCHED, process);
                event_loop_post(&g_event_loop, event);
            });
        }

        return EVENT_FAILURE;
    }
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_TERMINATED)
{
    struct process *process = context;
    struct application *application = window_manager_find_application(&g_window_manager, process->pid);

    if (application) {
        debug("%s: %s\n", __FUNCTION__, process->name);
        window_manager_remove_application(&g_window_manager, application->pid);

        int window_count = 0;
        struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);
        if (!window_list) goto end;

        for (int i = 0; i < window_count; ++i) {
            struct window *window = window_list[i];
            if (!window) continue;

            struct view *view = window_manager_find_managed_window(&g_window_manager, window);
            if (view) {
                space_manager_untile_window(&g_space_manager, view, window);
                window_manager_remove_managed_window(&g_window_manager, window->id);
                window_manager_purify_window(&g_window_manager, window);
            }

            if (g_mouse_state.window == window) g_mouse_state.window = NULL;

            window_manager_remove_window(&g_window_manager, window->id);
            window_destroy(window);
        }

        free(window_list);

end:
        application_unobserve(application);
        application_destroy(application);
    } else {
        debug("%s: %s (not observed)\n", __FUNCTION__, process->name);
    }

    return EVENT_SUCCESS;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_FRONT_SWITCHED)
{
    struct process *process = context;

    struct application *application = window_manager_find_application(&g_window_manager, process->pid);
    if (!application) {
        window_manager_add_lost_front_switched_event(&g_window_manager, process->pid);
        return EVENT_FAILURE;
    }

    struct event *de_event;
    event_create(de_event, APPLICATION_DEACTIVATED, (void *)(intptr_t) g_process_manager.front_pid);
    event_loop_post(&g_event_loop, de_event);

    struct event *re_event;
    event_create(re_event, APPLICATION_ACTIVATED, (void *)(intptr_t) process->pid);
    event_loop_post(&g_event_loop, re_event);

    debug("%s: %s\n", __FUNCTION__, process->name);
    g_process_manager.last_front_pid = g_process_manager.front_pid;
    g_process_manager.front_pid = process->pid;
    bar_refresh(&g_bar);

    return EVENT_SUCCESS;
}
#pragma clang diagnostic pop

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_ACTIVATED)
{
    struct application *application = window_manager_find_application(&g_window_manager, (pid_t)(intptr_t) context);
    if (!application) return EVENT_FAILURE;

    debug("%s: %s\n", __FUNCTION__, application->name);
    uint32_t application_focused_window_id = application_focused_window(application);
    if (!application_focused_window_id) {
        g_window_manager.last_window_id = g_window_manager.focused_window_id;
        g_window_manager.focused_window_id = 0;
        g_window_manager.focused_window_psn = application->psn;
        return EVENT_SUCCESS;
    }

    struct window *window = window_manager_find_window(&g_window_manager, application_focused_window_id);
    if (!window) {
        window_manager_add_lost_focused_event(&g_window_manager, application_focused_window_id);
        return EVENT_SUCCESS;
    }

    if (g_window_manager.focused_window_id != application_focused_window_id) {
        g_window_manager.last_window_id = g_window_manager.focused_window_id;
    }

    g_window_manager.focused_window_id = application_focused_window_id;
    g_window_manager.focused_window_psn = application->psn;

    border_window_activate(window);
    window_manager_set_window_opacity(&g_window_manager, window, g_window_manager.active_window_opacity);

    if (g_mouse_state.ffm_window_id != window->id) {
        window_manager_center_mouse(&g_window_manager, window);
    }

    g_mouse_state.ffm_window_id = 0;

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_DEACTIVATED)
{
    struct application *application = window_manager_find_application(&g_window_manager, (pid_t)(intptr_t) context);
    if (!application) return EVENT_FAILURE;

    debug("%s: %s\n", __FUNCTION__, application->name);
    struct window *focused_window = window_manager_find_window(&g_window_manager, application_focused_window(application));
    if (focused_window) {
        border_window_deactivate(focused_window);
        window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.normal_window_opacity);

        if (!window_level_is_standard(focused_window) || !window_is_standard(focused_window)) {
            struct window *main_window = window_manager_find_window(&g_window_manager, application_main_window(application));
            if (main_window && main_window != focused_window) {
                border_window_deactivate(main_window);
                window_manager_set_window_opacity(&g_window_manager, main_window, g_window_manager.normal_window_opacity);
            }
        }
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_VISIBLE)
{
    struct application *application = window_manager_find_application(&g_window_manager, (pid_t)(intptr_t) context);
    if (!application) return EVENT_FAILURE;

    debug("%s: %s\n", __FUNCTION__, application->name);
    application->is_hidden = false;

    int window_count = 0;
    struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);
    if (!window_list) return EVENT_SUCCESS;

    uint32_t prev_window_id = g_window_manager.last_window_id;
    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_list[i];
        if (window) {
            struct view *view = window_manager_find_managed_window(&g_window_manager, window);
            if (view) continue;

            if (window_manager_should_manage_window(window)) {
                struct view *view = space_manager_tile_window_on_space_with_insertion_point(&g_space_manager, window, window_space(window), prev_window_id);
                window_manager_add_managed_window(&g_window_manager, window, view);
                border_window_show(window);
                prev_window_id = window->id;
            }
        } else {
            prev_window_id = 0;
        }
    }

    free(window_list);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_HIDDEN)
{
    struct application *application = window_manager_find_application(&g_window_manager, (pid_t)(intptr_t) context);
    if (!application) return EVENT_FAILURE;

    debug("%s: %s\n", __FUNCTION__, application->name);
    application->is_hidden = true;

    int window_count = 0;
    struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);
    if (!window_list) return EVENT_SUCCESS;

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_list[i];
        if (!window) continue;

        border_window_hide(window);

        struct view *view = window_manager_find_managed_window(&g_window_manager, window);
        if (view) {
            space_manager_untile_window(&g_space_manager, view, window);
            window_manager_remove_managed_window(&g_window_manager, window->id);
            window_manager_purify_window(&g_window_manager, window);
        }
    }

    free(window_list);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_CREATED)
{
    uint32_t window_id = ax_window_id(context);
    if (!window_id) return EVENT_FAILURE;

    struct window *existing_window = window_manager_find_window(&g_window_manager, window_id);
    if (existing_window) return EVENT_FAILURE;

    pid_t window_pid = ax_window_pid(context);
    if (!window_pid) return EVENT_FAILURE;

    struct application *application = window_manager_find_application(&g_window_manager, window_pid);
    if (!application) return EVENT_FAILURE;

    struct window *window = window_create(application, CFRetain(context), window_id);
    if (window_is_popover(window) || window_is_unknown(window)) {
        debug("%s: ignoring window %s %d\n", __FUNCTION__, window->application->name, window->id);
        window_manager_make_children_floating(&g_window_manager, window, true);
        window_manager_make_floating(&g_window_manager, window->id, true);
        window_manager_remove_lost_focused_event(&g_window_manager, window->id);
        window_destroy(window);
        return EVENT_FAILURE;
    }

    window_manager_apply_rules_to_window(&g_space_manager, &g_window_manager, window);
    window_manager_set_window_opacity(&g_window_manager, window, g_window_manager.normal_window_opacity);
    window_manager_purify_window(&g_window_manager, window);

    if (window_observe(window)) {
        debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
        window_manager_add_window(&g_window_manager, window);

        if ((!application->is_hidden) && (!window->is_minimized) && (!window->is_fullscreen) && (!window->rule_manage)) {
            if (window->rule_fullscreen) {
                window->rule_fullscreen = false;
            } else if ((!window_level_is_standard(window)) ||
                       (!window_is_standard(window)) ||
                       (!window_can_move(window)) ||
                       (window_is_sticky(window)) ||
                       (window_is_undersized(window))) {
                window_manager_make_children_floating(&g_window_manager, window, true);
                window_manager_make_floating(&g_window_manager, window->id, true);
                window->is_floating = true;
            }
        }

        if (window_manager_should_manage_window(window)) {
            struct view *view = space_manager_tile_window_on_space(&g_space_manager, window, window_space(window));
            window_manager_add_managed_window(&g_window_manager, window, view);
        }

        if (window_manager_find_lost_focused_event(&g_window_manager, window->id)) {
            struct event *event;
            event_create(event, WINDOW_FOCUSED, (void *)(intptr_t) window->id);
            event_loop_post(&g_event_loop, event);
            window_manager_remove_lost_focused_event(&g_window_manager, window->id);
        }
    } else {
        debug("%s: could not observe %s %d\n", __FUNCTION__, window->application->name, window->id);
        window_manager_make_children_floating(&g_window_manager, window, true);
        window_manager_make_floating(&g_window_manager, window->id, true);
        window_manager_remove_lost_focused_event(&g_window_manager, window->id);
        window_unobserve(window);
        window_destroy(window);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_DESTROYED)
{
    uint32_t window_id = (uint32_t)(uintptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return EVENT_FAILURE;

    assert(!*window->id_ptr);
    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    window_unobserve(window);

    if (g_mouse_state.window == window) g_mouse_state.window = NULL;

    struct view *view = window_manager_find_managed_window(&g_window_manager, window);
    if (view) {
        space_manager_untile_window(&g_space_manager, view, window);
        window_manager_remove_managed_window(&g_window_manager, window->id);
        window_manager_purify_window(&g_window_manager, window);
    }

    window_manager_remove_window(&g_window_manager, window->id);
    window_destroy(window);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_FOCUSED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);

    if (!window) {
        window_manager_add_lost_focused_event(&g_window_manager, window_id);
        return EVENT_FAILURE;
    }

    if (!__sync_bool_compare_and_swap(window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return EVENT_SUCCESS;
    }

    if (window_is_minimized(window)) {
        window_manager_add_lost_focused_event(&g_window_manager, window->id);
        return EVENT_SUCCESS;
    }

    if (!application_is_frontmost(window->application)) {
        return EVENT_SUCCESS;
    }

    struct window *focused_window = window_manager_find_window(&g_window_manager, g_window_manager.focused_window_id);
    if (focused_window && focused_window != window) {
        border_window_deactivate(focused_window);
        window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.normal_window_opacity);
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    border_window_activate(window);
    window_manager_set_window_opacity(&g_window_manager, window, g_window_manager.active_window_opacity);
    bar_refresh(&g_bar);

    if (window_level_is_standard(window) && window_is_standard(window)) {
        if (g_window_manager.focused_window_id != window->id) {
            if (g_mouse_state.ffm_window_id != window->id) {
                window_manager_center_mouse(&g_window_manager, window);
            }

            g_window_manager.last_window_id = g_window_manager.focused_window_id;
        }

        g_window_manager.focused_window_id = window->id;
        g_window_manager.focused_window_psn = window->application->psn;
    }

    g_mouse_state.ffm_window_id = 0;

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MOVED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return EVENT_FAILURE;

    if (!__sync_bool_compare_and_swap(window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return EVENT_SUCCESS;
    }

    if (window->application->is_hidden) return EVENT_SUCCESS;

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);

    border_window_refresh(window);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_RESIZED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return EVENT_FAILURE;

    if (!__sync_bool_compare_and_swap(window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return EVENT_SUCCESS;
    }

    if (window->application->is_hidden) return EVENT_SUCCESS;

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);

    if (g_mouse_state.current_action == MOUSE_MODE_MOVE && g_mouse_state.window == window) {
        g_mouse_state.window_frame.size = window_ax_frame(g_mouse_state.window).size;
    }

    bool is_fullscreen = window_is_fullscreen(window);

    if (!window->is_fullscreen && is_fullscreen) {
        border_window_hide(window);

        struct view *view = window_manager_find_managed_window(&g_window_manager, window);
        if (view) {
            space_manager_untile_window(&g_space_manager, view, window);
            window_manager_remove_managed_window(&g_window_manager, window->id);
            window_manager_purify_window(&g_window_manager, window);
        }
    } else if (window->is_fullscreen && !is_fullscreen) {
        while (!space_is_user(space_manager_active_space())) { /* maybe spin lock */ }

        // @hack
        // Artificially delay by 50ms. This is necessary because macOS is crazy town.
        usleep(500000);

        if (window_manager_should_manage_window(window) && !window_manager_find_managed_window(&g_window_manager, window)) {
            struct view *view = space_manager_tile_window_on_space(&g_space_manager, window, window_space(window));
            window_manager_add_managed_window(&g_window_manager, window, view);
        }

        border_window_show(window);
    }

    window->is_fullscreen = is_fullscreen;

    if (!window->is_fullscreen) {
        border_window_refresh(window);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MINIMIZED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return EVENT_FAILURE;

    if (!__sync_bool_compare_and_swap(window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return EVENT_SUCCESS;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    window->is_minimized = true;
    border_window_hide(window);

    if (window->id == g_window_manager.last_window_id) {
        g_window_manager.last_window_id = g_window_manager.focused_window_id;
    }

    struct view *view = window_manager_find_managed_window(&g_window_manager, window);
    if (view) {
        space_manager_untile_window(&g_space_manager, view, window);
        window_manager_remove_managed_window(&g_window_manager, window->id);
        window_manager_purify_window(&g_window_manager, window);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_DEMINIMIZED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return EVENT_FAILURE;

    if (!__sync_bool_compare_and_swap(window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        window_manager_remove_lost_focused_event(&g_window_manager, window_id);
        return EVENT_SUCCESS;
    }

    window->is_minimized = false;
    border_window_show(window);

    if (space_manager_is_window_on_active_space(window)) {
        debug("%s: window %s %d is deminimized on active space\n", __FUNCTION__, window->application->name, window->id);
        if (window_manager_should_manage_window(window)) {
            struct window *last_window = window_manager_find_window(&g_window_manager, g_window_manager.last_window_id);
            uint32_t insertion_point = last_window && last_window->application->pid != window->application->pid ? last_window->id : 0;
            struct view *view = space_manager_tile_window_on_space_with_insertion_point(&g_space_manager, window, space_manager_active_space(), insertion_point);
            window_manager_add_managed_window(&g_window_manager, window, view);
        }
    } else {
        debug("%s: window %s %d is deminimized on inactive space\n", __FUNCTION__, window->application->name, window->id);
    }

    if (window_manager_find_lost_focused_event(&g_window_manager, window->id)) {
        struct event *event;
        event_create(event, WINDOW_FOCUSED, (void *)(intptr_t) window->id);
        event_loop_post(&g_event_loop, event);
        window_manager_remove_lost_focused_event(&g_window_manager, window->id);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_TITLE_CHANGED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return EVENT_FAILURE;

    if (!__sync_bool_compare_and_swap(window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return EVENT_SUCCESS;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    if (window->id == g_window_manager.focused_window_id) bar_refresh(&g_bar);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_SPACE_CHANGED)
{
    g_space_manager.last_space_id = g_space_manager.current_space_id;
    g_space_manager.current_space_id = space_manager_active_space();

    debug("%s: %lld\n", __FUNCTION__, g_space_manager.current_space_id);
    struct view *view = space_manager_find_view(&g_space_manager, g_space_manager.current_space_id);

    if (space_manager_refresh_application_windows(&g_space_manager)) {
        struct window *focused_window = window_manager_focused_window(&g_window_manager);
        if (focused_window && window_manager_find_lost_focused_event(&g_window_manager, focused_window->id)) {
            border_window_activate(focused_window);
            window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.active_window_opacity);
            window_manager_center_mouse(&g_window_manager, focused_window);
            window_manager_remove_lost_focused_event(&g_window_manager, focused_window->id);
        }
    }

    bar_refresh(&g_bar);

    if (space_is_user(g_space_manager.current_space_id)) {
        if (view_is_invalid(view)) view_update(view);
        if (view_is_dirty(view))   view_flush(view);

        window_manager_validate_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
        window_manager_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_CHANGED)
{
    g_display_manager.last_display_id = g_display_manager.current_display_id;
    g_display_manager.current_display_id = display_manager_active_display_id();

    g_space_manager.last_space_id = g_space_manager.current_space_id;
    g_space_manager.current_space_id = display_space_id(g_display_manager.current_display_id);

    uint32_t expected_display_id = space_display_id(g_space_manager.current_space_id);
    if (g_display_manager.current_display_id != expected_display_id) {
        debug("%s: %d %lld did not match %d! ignoring event..\n", __FUNCTION__, g_display_manager.current_display_id, g_space_manager.current_space_id, expected_display_id);
        return EVENT_FAILURE;
    }

    debug("%s: %d %lld\n", __FUNCTION__, g_display_manager.current_display_id, g_space_manager.current_space_id);
    struct view *view = space_manager_find_view(&g_space_manager, g_space_manager.current_space_id);

    if (space_manager_refresh_application_windows(&g_space_manager)) {
        struct window *focused_window = window_manager_focused_window(&g_window_manager);
        if (focused_window && window_manager_find_lost_focused_event(&g_window_manager, focused_window->id)) {
            border_window_activate(focused_window);
            window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.active_window_opacity);

            if (g_mouse_state.ffm_window_id != focused_window->id) {
                window_manager_center_mouse(&g_window_manager, focused_window);
            }

            g_mouse_state.ffm_window_id = 0;
            window_manager_remove_lost_focused_event(&g_window_manager, focused_window->id);
        }
    }

    bar_refresh(&g_bar);

    if (space_is_user(g_space_manager.current_space_id)) {
        if (view_is_invalid(view)) view_update(view);
        if (view_is_dirty(view))   view_flush(view);

        window_manager_validate_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
        window_manager_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_ADDED)
{
    uint32_t display_id = (uint32_t)(intptr_t) context;
    uint32_t sid = display_space_id(display_id);
    debug("%s: %d\n", __FUNCTION__, display_id);
    window_manager_handle_display_add_and_remove(&g_space_manager, &g_window_manager, display_id, sid);
    bar_resize(&g_bar);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_REMOVED)
{
    uint32_t display_id = display_manager_main_display_id();
    uint32_t sid = display_space_id(display_id);
    debug("%s: %d\n", __FUNCTION__, display_id);
    window_manager_handle_display_add_and_remove(&g_space_manager, &g_window_manager, display_id, sid);
    bar_resize(&g_bar);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_MOVED)
{
    uint32_t display_id = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, display_id);
    space_manager_mark_spaces_invalid(&g_space_manager);
    bar_resize(&g_bar);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_RESIZED)
{
    uint32_t display_id = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, display_id);
    space_manager_mark_spaces_invalid_for_display(&g_space_manager, display_id);
    bar_resize(&g_bar);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_DOWN)
{
    if (g_mission_control_active)                        return EVENT_SUCCESS;
    if (g_mouse_state.current_action != MOUSE_MODE_NONE) return EVENT_SUCCESS;

    CGPoint point = CGEventGetLocation(context);
    debug("%s: %.2f, %.2f\n", __FUNCTION__, point.x, point.y);

    g_mouse_state.window = window_manager_find_window_at_point(&g_window_manager, point);
    if (!g_mouse_state.window) g_mouse_state.window = window_manager_focused_window(&g_window_manager);
    if (!g_mouse_state.window) return EVENT_SUCCESS;

    g_mouse_state.down_location = point;
    g_mouse_state.window_frame = window_ax_frame(g_mouse_state.window);

    int64_t button = CGEventGetIntegerValueField(context, kCGMouseEventButtonNumber);
    uint8_t mod = mouse_mod_from_cgflags(CGEventGetFlags(context));

    if (button == kCGMouseButtonLeft && g_mouse_state.modifier == mod) {
        g_mouse_state.current_action = g_mouse_state.action1;
        return EVENT_MOUSE_IGNORE;
    } else if (button == kCGMouseButtonRight && g_mouse_state.modifier == mod) {
        g_mouse_state.current_action = g_mouse_state.action2;
        return EVENT_MOUSE_IGNORE;
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_UP)
{
    if (g_mission_control_active) return EVENT_SUCCESS;
    if (!g_mouse_state.window)    return EVENT_SUCCESS;

    if (!__sync_bool_compare_and_swap(g_mouse_state.window->id_ptr, &g_mouse_state.window->id, &g_mouse_state.window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, g_mouse_state.window->id);
        g_mouse_state.window = NULL;
        g_mouse_state.current_action = MOUSE_MODE_NONE;
        return EVENT_SUCCESS;
    }

    CGPoint point = CGEventGetLocation(context);
    debug("%s: %.2f, %.2f\n", __FUNCTION__, point.x, point.y);

    struct view *src_view = window_manager_find_managed_window(&g_window_manager, g_mouse_state.window);
    if ((g_mouse_state.current_action != MOUSE_MODE_RESIZE) && (src_view && src_view->layout == VIEW_BSP)) {
        CGRect frame = window_ax_frame(g_mouse_state.window);
        float dx = frame.origin.x - g_mouse_state.window_frame.origin.x;
        float dy = frame.origin.y - g_mouse_state.window_frame.origin.y;
        float dw = frame.size.width - g_mouse_state.window_frame.size.width;
        float dh = frame.size.height - g_mouse_state.window_frame.size.height;

        bool did_change_x = dx != 0.0f;
        bool did_change_y = dy != 0.0f;
        bool did_change_w = dw != 0.0f;
        bool did_change_h = dh != 0.0f;
        bool did_change_p = did_change_x || did_change_y;
        bool did_change_s = did_change_w || did_change_h;

        if (did_change_p && !did_change_s) {
            uint64_t cursor_sid = display_space_id(display_manager_cursor_display_id());
            struct view *dst_view = space_manager_find_view(&g_space_manager, cursor_sid);

            struct window *window = window_manager_find_window_at_point_filtering_window(&g_window_manager, point, g_mouse_state.window->id);
            if (!window) window = window_manager_find_window_at_point(&g_window_manager, point);
            if (window == g_mouse_state.window) window = NULL;

            struct window_node *a_node = view_find_window_node(src_view, g_mouse_state.window->id);
            struct window_node *b_node = window ? view_find_window_node(dst_view, window->id) : NULL;

            /*
             * @cleanup
             *
             * NOTE(koekeishiya): The following section of code contains duplicated code,
             * but is left inline as-is for readability purposes. Consider replacing with
             * macros, because C does not allow for locally-scoped functions.
             * */

            if (a_node && b_node) {
                CGRect frame = window_ax_frame(window);
                CGPoint point_in_window = {
                    .x = point.x - frame.origin.x,
                    .y = point.y - frame.origin.y
                };

                CGRect window_center;
                window_center.origin.x    = 0.25f * frame.size.width;
                window_center.origin.y    = 0.25f * frame.size.height;
                window_center.size.width  = 0.50f * frame.size.width;
                window_center.size.height = 0.50f * frame.size.height;

                CGPoint top_triangle[3];
                top_triangle[0].x = 0.0f;
                top_triangle[0].y = 0.0f;
                top_triangle[1].x = 0.5f * frame.size.width;
                top_triangle[1].y = 0.5f * frame.size.height;
                top_triangle[2].x = frame.size.width;
                top_triangle[2].y = 0.0f;

                CGPoint right_triangle[3];
                right_triangle[0].x = frame.size.width;
                right_triangle[0].y = 0.0f;
                right_triangle[1].x = 0.5f * frame.size.width;
                right_triangle[1].y = 0.5f * frame.size.height;
                right_triangle[2].x = frame.size.width;
                right_triangle[2].y = frame.size.height;

                CGPoint bottom_triangle[3];
                bottom_triangle[0].x = frame.size.width;
                bottom_triangle[0].y = frame.size.height;
                bottom_triangle[1].x = 0.5f * frame.size.width;
                bottom_triangle[1].y = 0.5f * frame.size.height;
                bottom_triangle[2].x = 0.0f;
                bottom_triangle[2].y = frame.size.height;

                CGPoint left_triangle[3];
                left_triangle[0].x = 0.0f;
                left_triangle[0].y = frame.size.height;
                left_triangle[1].x = 0.5f * frame.size.width;
                left_triangle[1].y = 0.5f * frame.size.height;
                left_triangle[2].x = 0.0f;
                left_triangle[2].y = 0.0f;

                enum window_node_split new_split;
                enum window_node_child new_child;

                if (CGRectContainsPoint(window_center, point_in_window)) {
do_swap:
                    a_node->window_id = window->id;
                    a_node->zoom = NULL;
                    b_node->window_id = g_mouse_state.window->id;
                    b_node->zoom = NULL;
                    window_node_flush(a_node);
                    window_node_flush(b_node);
                    goto end;
                } else if (triangle_contains_point(top_triangle, point_in_window)) {
                    new_split = SPLIT_X;
                    new_child = CHILD_FIRST;
                } else if (triangle_contains_point(right_triangle, point_in_window)) {
                    new_split = SPLIT_Y;
                    new_child = CHILD_SECOND;
                } else if (triangle_contains_point(bottom_triangle, point_in_window)) {
                    new_split = SPLIT_X;
                    new_child = CHILD_SECOND;
                } else if (triangle_contains_point(left_triangle, point_in_window)) {
                    new_split = SPLIT_Y;
                    new_child = CHILD_FIRST;
                } else {
                    goto end;
                }

                if (a_node->parent == b_node->parent) {
                    if (b_node->parent->split == new_split) {
                        goto do_swap;
                    } else {
                        b_node->parent->split = new_split;
                        b_node->parent->child = new_child;
                    }
                } else {
                    b_node->split = new_split;
                    b_node->child = new_child;
                }

                space_manager_untile_window(&g_space_manager, src_view, g_mouse_state.window);
                window_manager_remove_managed_window(&g_window_manager, g_mouse_state.window->id);
                window_manager_purify_window(&g_window_manager, g_mouse_state.window);

                struct view *view = space_manager_tile_window_on_space_with_insertion_point(&g_space_manager, g_mouse_state.window, dst_view->sid, window->id);
                window_manager_add_managed_window(&g_window_manager, g_mouse_state.window, view);
end:;
            } else if (a_node) {
                if (src_view->sid == dst_view->sid) {
                    a_node->zoom = NULL;
                    window_node_flush(a_node);
                } else {
                    space_manager_untile_window(&g_space_manager, src_view, g_mouse_state.window);
                    window_manager_remove_managed_window(&g_window_manager, g_mouse_state.window->id);
                    window_manager_purify_window(&g_window_manager, g_mouse_state.window);

                    struct view *view = space_manager_tile_window_on_space(&g_space_manager, g_mouse_state.window, dst_view->sid);
                    window_manager_add_managed_window(&g_window_manager, g_mouse_state.window, view);
                }
            }
        } else {
            if (did_change_p) {
                uint8_t direction = 0;
                if (dx != 0.0f) direction |= HANDLE_LEFT;
                if (dy != 0.0f) direction |= HANDLE_TOP;
                window_manager_resize_window_relative(&g_window_manager, g_mouse_state.window, direction, dx, dy);
            }

            if (did_change_s) {
                uint8_t direction = 0;
                if (did_change_w && !did_change_x) direction |= HANDLE_RIGHT;
                if (did_change_h && !did_change_y) direction |= HANDLE_BOTTOM;
                window_manager_resize_window_relative(&g_window_manager, g_mouse_state.window, direction, dw, dh);
            }
        }
    }

    g_mouse_state.current_action = MOUSE_MODE_NONE;
    g_mouse_state.window = NULL;

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_DRAGGED)
{
    if (g_mission_control_active) return EVENT_SUCCESS;
    if (!g_mouse_state.window)    return EVENT_SUCCESS;

    if (!__sync_bool_compare_and_swap(g_mouse_state.window->id_ptr, &g_mouse_state.window->id, &g_mouse_state.window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, g_mouse_state.window->id);
        g_mouse_state.window = NULL;
        g_mouse_state.current_action = MOUSE_MODE_NONE;
        return EVENT_SUCCESS;
    }

    if (g_mouse_state.current_action == MOUSE_MODE_MOVE) {
        uint64_t event_time = CGEventGetTimestamp(context);
        float dt = ((float) event_time - g_mouse_state.last_moved_time) * (1.0f / 1E6);
        if (dt < 25.0f) return EVENT_SUCCESS;

        CGPoint point = CGEventGetLocation(context);
        int dx = point.x - g_mouse_state.down_location.x;
        int dy = point.y - g_mouse_state.down_location.y;

        if (dx >= 10 || dx <= -10 || dy >= 10 || dy <= -10) {
            float fx = g_mouse_state.window_frame.origin.x + dx;
            float fy = g_mouse_state.window_frame.origin.y + dy;
            window_manager_move_window(g_mouse_state.window, fx, fy);
            g_mouse_state.last_moved_time = event_time;
        }
    } else if (g_mouse_state.current_action == MOUSE_MODE_RESIZE) {
        uint64_t event_time = CGEventGetTimestamp(context);
        float dt = ((float) event_time - g_mouse_state.last_moved_time) * (1.0f / 1E6);
        if (dt < 200.0f) return EVENT_SUCCESS;

        CGPoint point = CGEventGetLocation(context);
        int dx = point.x - g_mouse_state.down_location.x;
        int dy = point.y - g_mouse_state.down_location.y;

        if (dx >= 25 || dx <= -25 || dy >= 25 || dy <= 25) {
            uint8_t direction = 0;
            CGPoint frame_mid = { CGRectGetMidX(g_mouse_state.window_frame), CGRectGetMidY(g_mouse_state.window_frame) };

            if (point.x < frame_mid.x) direction |= HANDLE_LEFT;
            if (point.y < frame_mid.y) direction |= HANDLE_TOP;
            if (point.x > frame_mid.x) direction |= HANDLE_RIGHT;
            if (point.y > frame_mid.y) direction |= HANDLE_BOTTOM;
            window_manager_resize_window_relative(&g_window_manager, g_mouse_state.window, direction, dx, dy);

            g_mouse_state.last_moved_time = event_time;
            g_mouse_state.down_location = point;
            g_mouse_state.window_frame = window_ax_frame(g_mouse_state.window);
        }
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_MOVED)
{
    if (g_mission_control_active)    return EVENT_SUCCESS;
    if (g_mouse_state.ffm_window_id) return EVENT_SUCCESS;

    uint8_t mod = mouse_mod_from_cgflags(CGEventGetFlags(context));
    if (g_mouse_state.modifier == mod) return EVENT_SUCCESS;

    uint64_t event_time = CGEventGetTimestamp(context);
    float dt = ((float) event_time - g_mouse_state.last_moved_time) * (1.0f / 1E6);
    if (dt < 25.0f) return EVENT_SUCCESS;

    g_mouse_state.last_moved_time = event_time;

    if (g_window_manager.ffm_mode == FFM_DISABLED) return EVENT_SUCCESS;

    CGPoint point = CGEventGetLocation(context);
    struct window *window = window_manager_find_window_at_point(&g_window_manager, point);
    if (!window || window->id == g_window_manager.focused_window_id)      return EVENT_SUCCESS;
    if (!window_level_is_standard(window) || !window_is_standard(window)) return EVENT_SUCCESS;

    g_mouse_state.ffm_window_id = window->id;

    if (g_window_manager.ffm_mode == FFM_AUTOFOCUS) {
        window_manager_focus_window_without_raise(&window->application->psn, window->id);
    } else {
        window_manager_focus_window_with_raise(&window->application->psn, window->id, window->ref);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MISSION_CONTROL_ENTER)
{
    debug("%s:\n", __FUNCTION__);
    g_mission_control_active = true;

    for (int window_index = 0; window_index < g_window_manager.window.capacity; ++window_index) {
        struct bucket *bucket = g_window_manager.window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                border_window_hide(window);
            }

            bucket = bucket->next;
        }
    }

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.1f * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        struct event *event;
        event_create(event, MISSION_CONTROL_CHECK_FOR_EXIT, NULL);
        event_loop_post(&g_event_loop, event);
    });

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MISSION_CONTROL_CHECK_FOR_EXIT)
{
    if (!g_mission_control_active) return EVENT_FAILURE;

    CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, 0);
    int window_count = CFArrayGetCount(window_list);
    bool found = false;

    for (int i = 0; i < window_count; ++i) {
        CFDictionaryRef dictionary = CFArrayGetValueAtIndex(window_list, i);

        CFStringRef name = CFDictionaryGetValue(dictionary, kCGWindowName);
        if (name) continue;

        CFStringRef owner = CFDictionaryGetValue(dictionary, kCGWindowOwnerName);
        if (!owner) continue;

        if (CFEqual(CFSTR("Dock"), owner)) {
            found = true;
            break;
        }
    }

    if (found) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.1f * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            struct event *event;
            event_create(event, MISSION_CONTROL_CHECK_FOR_EXIT, NULL);
            event_loop_post(&g_event_loop, event);
        });
    } else {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.0f), dispatch_get_main_queue(), ^{
            struct event *event;
            event_create(event, MISSION_CONTROL_EXIT, NULL);
            event_loop_post(&g_event_loop, event);
        });
    }

    CFRelease(window_list);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MISSION_CONTROL_EXIT)
{
    debug("%s:\n", __FUNCTION__);
    g_mission_control_active = false;

    for (int window_index = 0; window_index < g_window_manager.window.capacity; ++window_index) {
        struct bucket *bucket = g_window_manager.window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if ((!window->application->is_hidden) &&
                    (!window->is_fullscreen) &&
                    (!window->is_minimized)) {
                    border_window_show(window);
                }
            }

            bucket = bucket->next;
        }
    }

    space_manager_mark_spaces_invalid(&g_space_manager);
    window_manager_validate_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
    window_manager_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DOCK_DID_RESTART)
{
    debug("%s:\n", __FUNCTION__);

    if (scripting_addition_is_installed()) {
        scripting_addition_load();

        for (int window_index = 0; window_index < g_window_manager.window.capacity; ++window_index) {
            struct bucket *bucket = g_window_manager.window.buckets[window_index];
            while (bucket) {
                if (bucket->value) {
                    struct window *window = bucket->value;
                    window_manager_purify_window(&g_window_manager, window);
                }

                bucket = bucket->next;
            }
        }
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MENU_OPENED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    window_manager_make_floating(&g_window_manager, window_id, true);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MENU_BAR_HIDDEN_CHANGED)
{
    debug("%s:\n", __FUNCTION__);
    space_manager_mark_spaces_invalid(&g_space_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DOCK_DID_CHANGE_PREF)
{
    debug("%s:\n", __FUNCTION__);
    space_manager_mark_spaces_invalid(&g_space_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_SYSTEM_WOKE)
{
    debug("%s:\n", __FUNCTION__);
    struct window *focused_window = window_manager_find_window(&g_window_manager, g_window_manager.focused_window_id);
    if (focused_window) {
        border_window_activate(focused_window);
        window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.active_window_opacity);
        window_manager_center_mouse(&g_window_manager, focused_window);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_BAR_REFRESH)
{
    bar_refresh(&g_bar);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DAEMON_MESSAGE)
{
    debug("%s: msg '", __FUNCTION__);
    for (int i = 0; i < param1 - 1; ++i) {
        char c = *((char *) context + i);
        debug("%c", c == '\0' ? ' ' : c);
    }
    debug("'\n");
    handle_message(param2, context);
    return EVENT_SUCCESS;
}

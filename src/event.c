#include "event.h"

extern struct event_loop g_event_loop;
extern struct process_manager g_process_manager;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;
extern struct mouse_state g_mouse_state;
extern bool g_mission_control_active;
extern int g_connection;
extern void *g_workspace_context;

static void window_did_receive_focus(struct window_manager *wm, struct mouse_state *ms, struct window *window)
{
    window_manager_set_window_opacity(wm, window, wm->active_window_opacity);
    border_activate(window);

    if (wm->focused_window_id != window->id) {
        if (ms->ffm_window_id != window->id) {
            window_manager_center_mouse(wm, window);
        }

        wm->last_window_id = wm->focused_window_id;
    }

    wm->focused_window_id = window->id;
    wm->focused_window_psn = window->application->psn;
    ms->ffm_window_id = 0;

    struct view *view = window_manager_find_managed_window(&g_window_manager, window);
    if (!view) return;

    struct window_node *node = view_find_window_node(view, window->id);
    if (node->window_count <= 1) return;

    for (int i = 0; i < node->window_count; ++i) {
        if (node->window_order[i] != window->id) continue;

        memmove(node->window_order + 1, node->window_order, sizeof(uint32_t) * i);
        node->window_order[0] = window->id;

        break;
    }
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_LAUNCHED)
{
    struct process *process = context;

    if (process->terminated) {
        debug("%s: %s (%d) terminated during launch\n", __FUNCTION__, process->name, process->pid);
        window_manager_remove_lost_front_switched_event(&g_window_manager, process->pid);
        return EVENT_FAILURE;
    }

    if (!workspace_application_is_observable(process)) {
        debug("%s: %s (%d) is not observable, subscribing to activationPolicy changes\n", __FUNCTION__, process->name, process->pid);
        workspace_application_observe_activation_policy(g_workspace_context, process);
        return EVENT_FAILURE;
    }

    if (!workspace_application_is_finished_launching(process)) {
        debug("%s: %s (%d) is not finished launching, subscribing to finishedLaunching changes\n", __FUNCTION__, process->name, process->pid);
        workspace_application_observe_finished_launching(g_workspace_context, process);
        return EVENT_FAILURE;
    }

    struct application *application = application_create(process);
    if (!application_observe(application)) {
        bool ax_retry = application->ax_retry;
        application_unobserve(application);
        application_destroy(application);

        debug("%s: could not observe notifications for %s (%d) (ax_retry = %d)\n", __FUNCTION__, process->name, process->pid, ax_retry);

        if (ax_retry) {
            __block ProcessSerialNumber psn = process->psn;
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.1f * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
                struct process *_process = process_manager_find_process(&g_process_manager, &psn);
                if (_process) event_loop_post(&g_event_loop, APPLICATION_LAUNCHED, _process, 0, NULL);
            });
        }

        return EVENT_FAILURE;
    }

    if (window_manager_find_lost_front_switched_event(&g_window_manager, process->pid)) {
        event_loop_post(&g_event_loop, APPLICATION_FRONT_SWITCHED, process, 0, NULL);
        window_manager_remove_lost_front_switched_event(&g_window_manager, process->pid);
    }

    debug("%s: %s (%d)\n", __FUNCTION__, process->name, process->pid);
    window_manager_add_application(&g_window_manager, application);
    window_manager_add_application_windows(&g_space_manager, &g_window_manager, application);

    int window_count;
    struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);
    uint32_t prev_window_id = g_window_manager.focused_window_id;

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_list[i];
        if (window->is_minimized) continue;

        struct view *view = window_manager_find_managed_window(&g_window_manager, window);
        if (view) continue;

        if (window_manager_should_manage_window(window)) {
            struct view *view = space_manager_tile_window_on_space_with_insertion_point(&g_space_manager, window, g_space_manager.current_space_id, prev_window_id);
            window_manager_add_managed_window(&g_window_manager, window, view);
            prev_window_id = window->id;
        }
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_TERMINATED)
{
    struct process *process = context;
    struct application *application = window_manager_find_application(&g_window_manager, process->pid);

    if (!application) {
        debug("%s: %s (%d) (not observed)\n", __FUNCTION__, process->name, process->pid);
        return EVENT_FAILURE;
    }

    debug("%s: %s (%d)\n", __FUNCTION__, process->name, process->pid);
    window_manager_remove_application(&g_window_manager, application->pid);

    int window_count;
    struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_list[i];

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

    application_unobserve(application);
    application_destroy(application);
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

    event_loop_post(&g_event_loop, APPLICATION_DEACTIVATED, (void *)(intptr_t) g_process_manager.front_pid, 0, NULL);
    event_loop_post(&g_event_loop, APPLICATION_ACTIVATED, (void *)(intptr_t) process->pid, 0, NULL);

    debug("%s: %s (%d)\n", __FUNCTION__, process->name, process->pid);
    g_process_manager.last_front_pid = g_process_manager.front_pid;
    g_process_manager.front_pid = process->pid;

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
        g_mouse_state.ffm_window_id = 0;
        return EVENT_SUCCESS;
    }

    struct window *window = window_manager_find_window(&g_window_manager, application_focused_window_id);
    if (!window) {
        window_manager_add_lost_focused_event(&g_window_manager, application_focused_window_id);
        return EVENT_SUCCESS;
    }

    window_did_receive_focus(&g_window_manager, &g_mouse_state, window);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_DEACTIVATED)
{
    struct application *application = window_manager_find_application(&g_window_manager, (pid_t)(intptr_t) context);
    if (!application) return EVENT_FAILURE;

    debug("%s: %s\n", __FUNCTION__, application->name);
    struct window *focused_window = window_manager_find_window(&g_window_manager, application_focused_window(application));
    if (!focused_window) return EVENT_SUCCESS;

    window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.normal_window_opacity);
    border_deactivate(focused_window);

    if (!window_level_is_standard(focused_window) || !window_is_standard(focused_window)) {
        struct window *main_window = window_manager_find_window(&g_window_manager, application_main_window(application));
        if (main_window && main_window != focused_window) {
            window_manager_set_window_opacity(&g_window_manager, main_window, g_window_manager.normal_window_opacity);
            border_deactivate(main_window);
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

    int window_count;
    struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);
    uint32_t prev_window_id = g_window_manager.last_window_id;

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_list[i];
        if (window->is_minimized) continue;

        struct view *view = window_manager_find_managed_window(&g_window_manager, window);
        if (view) continue;

        if (window_manager_should_manage_window(window)) {
            struct view *view = space_manager_tile_window_on_space_with_insertion_point(&g_space_manager, window, window_space(window), prev_window_id);
            window_manager_add_managed_window(&g_window_manager, window, view);
            prev_window_id = window->id;
        }
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_HIDDEN)
{
    struct application *application = window_manager_find_application(&g_window_manager, (pid_t)(intptr_t) context);
    if (!application) return EVENT_FAILURE;

    debug("%s: %s\n", __FUNCTION__, application->name);
    application->is_hidden = true;

    int window_count;
    struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_list[i];

        struct view *view = window_manager_find_managed_window(&g_window_manager, window);
        if (view) {
            space_manager_untile_window(&g_space_manager, view, window);
            window_manager_remove_managed_window(&g_window_manager, window->id);
            window_manager_purify_window(&g_window_manager, window);
        }
    }

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

    struct window *window = window_manager_create_and_add_window(&g_space_manager, &g_window_manager, application, CFRetain(context), window_id);
    if (!window) return EVENT_FAILURE;

    if (window_manager_should_manage_window(window) && !window_manager_find_managed_window(&g_window_manager, window)) {
        struct view *view = space_manager_tile_window_on_space(&g_space_manager, window, g_space_manager.current_space_id);
        window_manager_add_managed_window(&g_window_manager, window, view);
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

    struct view *view = window_manager_find_managed_window(&g_window_manager, window);
    if (view) {
        space_manager_untile_window(&g_space_manager, view, window);
        window_manager_remove_managed_window(&g_window_manager, window->id);
        window_manager_purify_window(&g_window_manager, window);
    }

    if (g_mouse_state.window == window) g_mouse_state.window = NULL;
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
        return EVENT_FAILURE;
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
        window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.normal_window_opacity);
        border_deactivate(focused_window);
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    window_did_receive_focus(&g_window_manager, &g_mouse_state, window);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MOVED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return EVENT_FAILURE;

    if (!__sync_bool_compare_and_swap(window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return EVENT_FAILURE;
    }

    if (window->application->is_hidden) {
        debug("%s: %d was moved while the application is hidden, ignoring event..\n", __FUNCTION__, window_id);
        return EVENT_FAILURE;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_RESIZED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return EVENT_FAILURE;

    if (!__sync_bool_compare_and_swap(window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return EVENT_FAILURE;
    }

    if (window->application->is_hidden) {
        debug("%s: %d was resized while the application is hidden, ignoring event..\n", __FUNCTION__, window_id);
        return EVENT_FAILURE;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    bool was_fullscreen = window->is_fullscreen;
    bool is_fullscreen = window_is_fullscreen(window);
    window->is_fullscreen = is_fullscreen;

    if (!was_fullscreen && is_fullscreen) {
        window_manager_make_window_topmost(&g_window_manager, window, false);
        border_enter_fullscreen(window);

        struct view *view = window_manager_find_managed_window(&g_window_manager, window);
        if (view) {
            space_manager_untile_window(&g_space_manager, view, window);
            window_manager_remove_managed_window(&g_window_manager, window->id);
            window_manager_purify_window(&g_window_manager, window);
        }
    } else if (was_fullscreen && !is_fullscreen) {
        window_manager_wait_for_native_fullscreen_transition(window);

        if (window_manager_should_manage_window(window) && !window_manager_find_managed_window(&g_window_manager, window)) {
            struct view *view = space_manager_tile_window_on_space(&g_space_manager, window, window_space(window));
            window_manager_add_managed_window(&g_window_manager, window, view);
        }

        border_exit_fullscreen(window);
        window_manager_make_window_topmost(&g_window_manager, window, window->is_floating);
    } else if (!was_fullscreen == !is_fullscreen) {
        if (g_mouse_state.current_action == MOUSE_MODE_MOVE && g_mouse_state.window == window) {
            g_mouse_state.window_frame.size = window_ax_frame(g_mouse_state.window).size;
        }

        border_resize(window);
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
        return EVENT_FAILURE;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    window->is_minimized = true;

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
        return EVENT_FAILURE;
    }

    window->is_minimized = false;

    if (space_manager_is_window_on_active_space(window)) {
        debug("%s: window %s %d is deminimized on active space\n", __FUNCTION__, window->application->name, window->id);
        if (window_manager_should_manage_window(window) && !window_manager_find_managed_window(&g_window_manager, window)) {
            struct window *last_window = window_manager_find_window(&g_window_manager, g_window_manager.last_window_id);
            uint32_t insertion_point = last_window && last_window->application->pid != window->application->pid ? last_window->id : 0;
            struct view *view = space_manager_tile_window_on_space_with_insertion_point(&g_space_manager, window, space_manager_active_space(), insertion_point);
            window_manager_add_managed_window(&g_window_manager, window, view);
        }
    } else {
        debug("%s: window %s %d is deminimized on inactive space\n", __FUNCTION__, window->application->name, window->id);
    }

    if (window_manager_find_lost_focused_event(&g_window_manager, window->id)) {
        event_loop_post(&g_event_loop, WINDOW_FOCUSED, (void *)(intptr_t) window->id, 0, NULL);
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
        return EVENT_FAILURE;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
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
            window_did_receive_focus(&g_window_manager, &g_mouse_state, focused_window);
            window_manager_remove_lost_focused_event(&g_window_manager, focused_window->id);
        }
    }

    if (space_is_user(g_space_manager.current_space_id)) {
        if (view_is_invalid(view)) view_update(view);
        if (view_is_dirty(view))   view_flush(view);

        window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
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
            window_did_receive_focus(&g_window_manager, &g_mouse_state, focused_window);
            window_manager_remove_lost_focused_event(&g_window_manager, focused_window->id);
        }
    }

    if (space_is_user(g_space_manager.current_space_id)) {
        if (view_is_invalid(view)) view_update(view);
        if (view_is_dirty(view))   view_flush(view);

        window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_ADDED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    space_manager_handle_display_add(&g_space_manager, did);
    window_manager_handle_display_add_and_remove(&g_space_manager, &g_window_manager, did);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_REMOVED)
{
    uint32_t did = display_manager_main_display_id();
    debug("%s: %d\n", __FUNCTION__, did);
    window_manager_handle_display_add_and_remove(&g_space_manager, &g_window_manager, did);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_MOVED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    space_manager_mark_spaces_invalid(&g_space_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_RESIZED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    space_manager_mark_spaces_invalid_for_display(&g_space_manager, did);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_DOWN)
{
    if (g_mission_control_active)                        return EVENT_SUCCESS;
    if (g_mouse_state.current_action != MOUSE_MODE_NONE) return EVENT_SUCCESS;

    CGPoint point = CGEventGetLocation(context);
    debug("%s: %.2f, %.2f\n", __FUNCTION__, point.x, point.y);

    struct window *window = window_manager_find_window_at_point(&g_window_manager, point);
    if (!window) window = window_manager_focused_window(&g_window_manager);
    if (!window || window_is_fullscreen(window)) return EVENT_SUCCESS;

    g_mouse_state.window = window;
    g_mouse_state.down_location = point;
    g_mouse_state.window_frame = window_ax_frame(g_mouse_state.window);

    int64_t button = CGEventGetIntegerValueField(context, kCGMouseEventButtonNumber);
    uint8_t mod = (uint8_t) param1;

    if (button == kCGMouseButtonLeft && g_mouse_state.modifier == mod) {
        g_mouse_state.current_action = g_mouse_state.action1;
    } else if (button == kCGMouseButtonRight && g_mouse_state.modifier == mod) {
        g_mouse_state.current_action = g_mouse_state.action2;
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_UP)
{
    if (g_mission_control_active) return EVENT_SUCCESS;
    if (!g_mouse_state.window)    return EVENT_SUCCESS;

    if (!__sync_bool_compare_and_swap(g_mouse_state.window->id_ptr, &g_mouse_state.window->id, &g_mouse_state.window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, g_mouse_state.window->id);
        goto out;
    }

    if (window_is_fullscreen(g_mouse_state.window)) {
        debug("%s: %d is transitioning into native-fullscreen mode, ignoring event..\n", __FUNCTION__, g_mouse_state.window->id);
        goto out;
    }

    CGPoint point = CGEventGetLocation(context);
    debug("%s: %.2f, %.2f\n", __FUNCTION__, point.x, point.y);

    struct view *src_view = window_manager_find_managed_window(&g_window_manager, g_mouse_state.window);
    if (!src_view) goto out;

    struct mouse_window_info info;
    mouse_window_info_populate(&g_mouse_state, &info);

    if (info.changed_position && !info.changed_size) {
        uint64_t cursor_sid = display_space_id(display_manager_point_display_id(point));
        struct view *dst_view = space_manager_find_view(&g_space_manager, cursor_sid);

        struct window *window = window_manager_find_window_at_point_filtering_window(&g_window_manager, point, g_mouse_state.window->id);
        if (!window) window = window_manager_find_window_at_point(&g_window_manager, point);
        if (window == g_mouse_state.window) window = NULL;

        struct window_node *a_node = view_find_window_node(src_view, g_mouse_state.window->id);
        struct window_node *b_node = window ? view_find_window_node(dst_view, window->id) : NULL;

        if (a_node && b_node && a_node != b_node) {
            enum mouse_drop_action drop_action = mouse_determine_drop_action(&g_mouse_state, a_node, window, point);
            switch (drop_action) {
            case MOUSE_DROP_ACTION_STACK: {
                mouse_drop_action_stack(&g_space_manager, &g_window_manager, src_view, g_mouse_state.window, dst_view, window);
            } break;
            case MOUSE_DROP_ACTION_SWAP: {
                mouse_drop_action_swap(&g_window_manager, src_view, a_node, g_mouse_state.window, dst_view, b_node, window);
            } break;
            case MOUSE_DROP_ACTION_WARP_TOP: {
                mouse_drop_action_warp(&g_space_manager, &g_window_manager, src_view, a_node, g_mouse_state.window, dst_view, b_node, window, SPLIT_X, CHILD_FIRST);
            } break;
            case MOUSE_DROP_ACTION_WARP_RIGHT: {
                mouse_drop_action_warp(&g_space_manager, &g_window_manager, src_view, a_node, g_mouse_state.window, dst_view, b_node, window, SPLIT_Y, CHILD_SECOND);
            } break;
            case MOUSE_DROP_ACTION_WARP_BOTTOM: {
                mouse_drop_action_warp(&g_space_manager, &g_window_manager, src_view, a_node, g_mouse_state.window, dst_view, b_node, window, SPLIT_X, CHILD_SECOND);
            } break;
            case MOUSE_DROP_ACTION_WARP_LEFT: {
                mouse_drop_action_warp(&g_space_manager, &g_window_manager, src_view, a_node, g_mouse_state.window, dst_view, b_node, window, SPLIT_Y, CHILD_FIRST);
            } break;
            case MOUSE_DROP_ACTION_NONE: {
                /* silence compiler warning.. */
            } break;
            }
        } else if (a_node) {
            mouse_drop_no_target(&g_space_manager, &g_window_manager, src_view, dst_view, g_mouse_state.window, a_node);
        }
    } else {
        mouse_drop_try_adjust_bsp_grid(&g_window_manager, src_view, g_mouse_state.window, &info);
    }

out:
    g_mouse_state.window = NULL;
    g_mouse_state.current_action = MOUSE_MODE_NONE;

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
        CGPoint point = CGEventGetLocation(context);

        CGPoint new_point = { g_mouse_state.window_frame.origin.x + (point.x - g_mouse_state.down_location.x),
                              g_mouse_state.window_frame.origin.y + (point.y - g_mouse_state.down_location.y) };

        uint32_t did = display_manager_point_display_id(new_point);
        if (did) {
            CGRect bounds = display_bounds(did);
            if (new_point.y < bounds.origin.y) new_point.y = bounds.origin.y;
        }

        scripting_addition_move_window(g_mouse_state.window->id, new_point.x, new_point.y);
    } else if (g_mouse_state.current_action == MOUSE_MODE_RESIZE) {
        uint64_t event_time = CGEventGetTimestamp(context);
        float dt = ((float) event_time - g_mouse_state.last_moved_time) * (1.0f / 1E6);
        if (dt < 66.67f) return EVENT_SUCCESS;

        CGPoint point = CGEventGetLocation(context);
        int dx = point.x - g_mouse_state.down_location.x;
        int dy = point.y - g_mouse_state.down_location.y;

        uint8_t direction = 0;
        CGRect frame = window_ax_frame(g_mouse_state.window);
        CGPoint frame_mid = { CGRectGetMidX(frame), CGRectGetMidY(frame) };

        if (point.x < frame_mid.x) direction |= HANDLE_LEFT;
        if (point.y < frame_mid.y) direction |= HANDLE_TOP;
        if (point.x > frame_mid.x) direction |= HANDLE_RIGHT;
        if (point.y > frame_mid.y) direction |= HANDLE_BOTTOM;

        window_manager_resize_window_relative_internal(g_mouse_state.window, frame, direction, dx, dy);

        g_mouse_state.last_moved_time = event_time;
        g_mouse_state.down_location = point;
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_MOVED)
{
    if (g_window_manager.ffm_mode == FFM_DISABLED) return EVENT_SUCCESS;

    if (g_mission_control_active)    return EVENT_SUCCESS;
    if (g_mouse_state.ffm_window_id) return EVENT_SUCCESS;

    uint8_t mod = mouse_mod_from_cgflags(CGEventGetFlags(context));
    if (g_mouse_state.modifier == mod) return EVENT_SUCCESS;

    uint64_t event_time = CGEventGetTimestamp(context);
    float dt = ((float) event_time - g_mouse_state.last_moved_time) * (1.0f / 1E6);
    if (dt < 25.0f) return EVENT_SUCCESS;

    CGPoint point = CGEventGetLocation(context);
    g_mouse_state.last_moved_time = event_time;

    struct window *window = window_manager_find_window_at_point(&g_window_manager, point);
    if (window) {
        if (window->id == g_window_manager.focused_window_id) return EVENT_SUCCESS;
        if (!window_level_is_standard(window))                return EVENT_SUCCESS;
        if (!window_is_standard(window))                      return EVENT_SUCCESS;

        if (g_window_manager.ffm_mode == FFM_AUTOFOCUS) {

            //
            // NOTE(koekeishiya): Look for a window with role AXSheet or AXDrawer
            // and forward focus to it because we are not allowed to focus the main
            // window in these cases.
            //

            CFArrayRef window_list = SLSCopyAssociatedWindows(g_connection, window->id);
            int window_count = CFArrayGetCount(window_list);

            uint32_t child_wid;
            for (int i = 0; i < window_count; ++i) {
                CFNumberGetValue(CFArrayGetValueAtIndex(window_list, i), kCFNumberSInt32Type, &child_wid);
                struct window *child = window_manager_find_window(&g_window_manager, child_wid);
                if (!child) continue;

                CFTypeRef role = window_role(child);
                if (!role) continue;

                bool valid = CFEqual(role, kAXSheetRole) || CFEqual(role, kAXDrawerRole);
                CFRelease(role);

                if (valid) {
                    window = child;
                    break;
                }
            }

            CFRelease(window_list);
            window_manager_focus_window_without_raise(&window->application->psn, window->id);
            g_mouse_state.ffm_window_id = window->id;
        } else {
            window_manager_focus_window_with_raise(&window->application->psn, window->id, window->ref);
            g_mouse_state.ffm_window_id = window->id;
        }
    } else {
        uint32_t cursor_did = display_manager_point_display_id(point);
        if (g_display_manager.current_display_id == cursor_did) return EVENT_SUCCESS;

        CGRect menu = display_manager_menu_bar_rect(cursor_did);
        if (cgrect_contains_point(menu, point)) return EVENT_SUCCESS;

        display_manager_focus_display_with_point(cursor_did, point, false);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MISSION_CONTROL_ENTER)
{
    debug("%s:\n", __FUNCTION__);
    g_mission_control_active = true;

    for (int i = 0; i < buf_len(g_window_manager.insert_feedback_windows); ++i) {
        uint32_t feedback_wid = g_window_manager.insert_feedback_windows[i];
        SLSOrderWindow(g_connection, feedback_wid, 0, 0);
    }

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.1f * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        event_loop_post(&g_event_loop, MISSION_CONTROL_CHECK_FOR_EXIT, NULL, 0, NULL);
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

        CFNumberRef layer_ref = CFDictionaryGetValue(dictionary, kCGWindowLayer);
        if (!layer_ref) continue;

        uint64_t layer = 0;
        CFNumberGetValue(layer_ref, CFNumberGetType(layer_ref), &layer);
        if (layer != 18) continue;

        if (CFEqual(CFSTR("Dock"), owner)) {
            found = true;
            break;
        }
    }

    if (found) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.1f * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            event_loop_post(&g_event_loop, MISSION_CONTROL_CHECK_FOR_EXIT, NULL, 0, NULL);
        });
    } else {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.0f), dispatch_get_main_queue(), ^{
            event_loop_post(&g_event_loop, MISSION_CONTROL_EXIT, NULL, 0, NULL);
        });
    }

    CFRelease(window_list);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MISSION_CONTROL_EXIT)
{
    debug("%s:\n", __FUNCTION__);
    g_mission_control_active = false;

    for (int i = 0; i < buf_len(g_window_manager.insert_feedback_windows); ++i) {
        uint32_t feedback_wid = g_window_manager.insert_feedback_windows[i];
        SLSOrderWindow(g_connection, feedback_wid, 1, 0);
    }

    space_manager_mark_spaces_invalid(&g_space_manager);

    if (space_is_user(g_space_manager.current_space_id)) {
        window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DOCK_DID_RESTART)
{
    debug("%s:\n", __FUNCTION__);

    if (!workspace_is_macos_bigsur() && scripting_addition_is_installed()) {
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

    if (g_window_manager.enable_window_topmost) {
        scripting_addition_set_layer(window_id, LAYER_ABOVE);
    }

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
        window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.active_window_opacity);
        border_activate(focused_window);
        window_manager_center_mouse(&g_window_manager, focused_window);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DAEMON_MESSAGE)
{
    int length;
    char *message = socket_read(param1, &length);
    if (!message) goto out;

    FILE *rsp = fdopen(param1, "w");
    if (!rsp) goto out;

    debug_message(__FUNCTION__, message);
    handle_message(rsp, message);

    fflush(rsp);
    fclose(rsp);

out:
    socket_close(param1);
    return EVENT_SUCCESS;
}

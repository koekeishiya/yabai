extern struct event_loop g_event_loop;
extern struct process_manager g_process_manager;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;
extern struct mouse_state g_mouse_state;
extern enum mission_control_mode g_mission_control_mode;
extern int g_connection;
extern void *g_workspace_context;
extern int g_layer_below_window_level;

static void update_window_notifications(void)
{
    int window_count = 0;
    uint32_t window_list[1024] = {0};

    if (workspace_is_macos_sequoia()) {
        // NOTE(koekeishiya): Subscribe to all windows because of window_destroyed (and ordered) notifications
        table_for (struct window *window, g_window_manager.window, {
            window_list[window_count++] = window->id;
        })
    } else {
        // NOTE(koekeishiya): Subscribe to windows that have a feedback_border because of window_ordered notifications
        table_for (struct window_node *node, g_window_manager.insert_feedback, {
            window_list[window_count++] = node->window_order[0];
        })
    }

    SLSRequestNotificationsForWindows(g_connection, window_list, window_count);
}

static void window_did_receive_focus(struct window_manager *wm, struct mouse_state *ms, struct window *window)
{
    struct window *focused_window = window_manager_find_window(wm, wm->focused_window_id);
    if (focused_window && focused_window != window && window_space(focused_window->id) == window_space(window->id)) {
        window_manager_set_window_opacity(wm, focused_window, g_window_manager.normal_window_opacity);
    }

    window_manager_set_window_opacity(wm, window, wm->active_window_opacity);

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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
static EVENT_HANDLER(APPLICATION_LAUNCHED)
{
    struct process *process = context;

    if (__atomic_load_n(&process->terminated, __ATOMIC_RELAXED)) {
        debug("%s: %s (%d) terminated during launch\n", __FUNCTION__, process->name, process->pid);
        window_manager_remove_lost_front_switched_event(&g_window_manager, process->pid);
        return;
    }

    if (!__atomic_load_n(&process->ns_application, __ATOMIC_RELAXED)) {
        debug("%s: %s (%d) missing ns_application. fetching..\n", __FUNCTION__, process->name, process->pid);
        __atomic_store_n(&process->ns_application, workspace_application_create_running_ns_application(process), __ATOMIC_RELEASE);

        if (!__atomic_load_n(&process->ns_application, __ATOMIC_RELAXED)) {
            debug("%s: %s (%d) unable to fetch ns_application..\n", __FUNCTION__, process->name, process->pid);

            __block ProcessSerialNumber psn = process->psn;
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.1f * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
                struct process *_process = process_manager_find_process(&g_process_manager, &psn);
                if (_process) event_loop_post(&g_event_loop, APPLICATION_LAUNCHED, _process, 0);
            });

            return;
        }
    }

    if (!workspace_application_is_finished_launching(process)) {
        debug("%s: %s (%d) is not finished launching, subscribing to finishedLaunching changes\n", __FUNCTION__, process->name, process->pid);
        workspace_application_observe_finished_launching(g_workspace_context, process);

        //
        // NOTE(koekeishiya): Do this again in case of race-conditions between the previous check and key-value observation subscription.
        // Not actually sure if this can happen in practice..
        //

        if (workspace_application_is_finished_launching(process)) {
            @try {
                NSRunningApplication *application = __atomic_load_n(&process->ns_application, __ATOMIC_RELAXED);
                if (application && [application observationInfo]) {
                    [application removeObserver:g_workspace_context forKeyPath:@"finishedLaunching" context:process];
                }
            } @catch (NSException * __unused exception) {}
        } else { return; }
    }

    if (!workspace_application_is_observable(process)) {
        debug("%s: %s (%d) is not observable, subscribing to activationPolicy changes\n", __FUNCTION__, process->name, process->pid);
        workspace_application_observe_activation_policy(g_workspace_context, process);

        //
        // NOTE(koekeishiya): Do this again in case of race-conditions between the previous check and key-value observation subscription.
        // Not actually sure if this can happen in practice..
        //

        if (workspace_application_is_observable(process)) {
            @try {
                NSRunningApplication *application = __atomic_load_n(&process->ns_application, __ATOMIC_RELAXED);
                if (application && [application observationInfo]) {
                    [application removeObserver:g_workspace_context forKeyPath:@"activationPolicy" context:process];
                }
            } @catch (NSException * __unused exception) {}
        } else { return; }
    }

    //
    // NOTE(koekeishiya): If we somehow receive a duplicate launched event due to the subscription-timing-mess above,
    // simply ignore the event..
    //

    struct application *application = window_manager_find_application(&g_window_manager, process->pid);
    if (application) { return; } else { application = application_create(process); }

    if (!application_observe(application)) {
        bool ax_retry = application->ax_retry;

        application_unobserve(application);
        application_destroy(application);
        debug("%s: could not observe notifications for %s (%d) (%d)\n", __FUNCTION__, process->name, process->pid, ax_retry);

        if (ax_retry) {
            __block ProcessSerialNumber psn = process->psn;
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.1f * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
                struct process *_process = process_manager_find_process(&g_process_manager, &psn);
                if (_process) event_loop_post(&g_event_loop, APPLICATION_LAUNCHED, _process, 0);
            });
        }

        return;
    }

    if (window_manager_find_lost_front_switched_event(&g_window_manager, process->pid)) {
        event_loop_post(&g_event_loop, APPLICATION_FRONT_SWITCHED, process, 0);
        window_manager_remove_lost_front_switched_event(&g_window_manager, process->pid);
    }

    debug("%s: %s (%d)\n", __FUNCTION__, process->name, process->pid);
    window_manager_add_application(&g_window_manager, application);
    event_signal_push(SIGNAL_APPLICATION_LAUNCHED, application);

    int window_count;
    struct window **window_list = window_manager_add_application_windows(&g_space_manager, &g_window_manager, application, &window_count);
    uint32_t prev_window_id = g_window_manager.focused_window_id;

    uint64_t sid;
    bool default_origin = g_window_manager.window_origin_mode == WINDOW_ORIGIN_DEFAULT;

    if (!default_origin) {
        if (g_window_manager.window_origin_mode == WINDOW_ORIGIN_FOCUSED) {
            sid = g_space_manager.current_space_id;
        } else /* if (g_window_manager.window_origin_mode == WINDOW_ORIGIN_CURSOR) */ {
            sid = space_manager_cursor_space();
        }
    }

    int view_count = 0;
    struct view **view_list = ts_alloc_list(struct view *, window_count);

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_list[i];

        if (window_manager_should_manage_window(window) && !window_manager_find_managed_window(&g_window_manager, window)) {
            if (default_origin) sid = window_space(window->id);

            struct view *view = space_manager_find_view(&g_space_manager, sid);
            if (view->layout != VIEW_FLOAT) {
                //
                // @cleanup
                //
                // :AXBatching
                //
                // NOTE(koekeishiya): Batch all operations and mark the view as dirty so that we can perform a single flush,
                // making sure that each window is only moved and resized a single time, when the final layout has been computed.
                // This is necessary to make sure that we do not call the AX API for each modification to the tree.
                //

                window_manager_adjust_layer(window, LAYER_BELOW);
                view_add_window_node_with_insertion_point(view, window, prev_window_id);
                window_manager_add_managed_window(&g_window_manager, window, view);

                view_set_flag(view, VIEW_IS_DIRTY);
                view_list[view_count++] = view;

                prev_window_id = window->id;
            }
        }

        if (window_manager_is_window_eligible(window)) {
            event_signal_push(SIGNAL_WINDOW_CREATED, window);
        }
    }

    //
    // @cleanup
    //
    // :AXBatching
    //
    // NOTE(koekeishiya): Flush previously batched operations if the view is marked as dirty.
    // This is necessary to make sure that we do not call the AX API for each modification to the tree.
    //

    for (int i = 0; i < view_count; ++i) {
        struct view *view = view_list[i];
        if (!space_is_visible(view->sid)) continue;
        if (!view_is_dirty(view))         continue;

        window_node_flush(view->root);
        view_clear_flag(view, VIEW_IS_DIRTY);
    }

    if (workspace_is_macos_sequoia()) {
        update_window_notifications();
    }
}

static EVENT_HANDLER(APPLICATION_TERMINATED)
{
    struct process *process = context;
    struct application *application = window_manager_find_application(&g_window_manager, process->pid);

    if (!application) {
        debug("%s: %s (%d) (not observed)\n", __FUNCTION__, process->name, process->pid);
        goto out;
    }

    debug("%s: %s (%d)\n", __FUNCTION__, process->name, process->pid);
    event_signal_push(SIGNAL_APPLICATION_TERMINATED, application);
    window_manager_remove_application(&g_window_manager, application->pid);

    for (int i = 0; i < buf_len(g_window_manager.applications_to_refresh); ++i) {
        if (application == g_window_manager.applications_to_refresh[i]) {
            buf_del(g_window_manager.applications_to_refresh, i);
            break;
        }
    }

    int window_count;
    struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);

    int view_count = 0;
    struct view **view_list = ts_alloc_list(struct view *, window_count);

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_list[i];

        if (!__sync_bool_compare_and_swap(&window->id_ptr, &window->id, NULL)) {
            window->application = NULL;
            continue;
        }

        struct view *view = window_manager_find_managed_window(&g_window_manager, window);
        if (view) {

            //
            // @cleanup
            //
            // :AXBatching
            //
            // NOTE(koekeishiya): Batch all operations and mark the view as dirty so that we can perform a single flush,
            // making sure that each window is only moved and resized a single time, when the final layout has been computed.
            // This is necessary to make sure that we do not call the AX API for each modification to the tree.
            //

            view_remove_window_node(view, window);
            window_manager_remove_managed_window(&g_window_manager, window->id);

            view_set_flag(view, VIEW_IS_DIRTY);
            view_list[view_count++] = view;
        }

        if (g_mouse_state.window == window) g_mouse_state.window = NULL;

        if (window->is_eligible) {
            event_signal_push(SIGNAL_WINDOW_DESTROYED, window);
        }

        window_manager_remove_scratchpad_for_window(&g_window_manager, window, false);
        window_manager_remove_window(&g_window_manager, window->id);
        window_unobserve(window);
        window_destroy(window);
    }

    application_unobserve(application);
    application_destroy(application);

    //
    // @cleanup
    //
    // :AXBatching
    //
    // NOTE(koekeishiya): Flush previously batched operations if the view is marked as dirty.
    // This is necessary to make sure that we do not call the AX API for each modification to the tree.
    //

    for (int i = 0; i < view_count; ++i) {
        struct view *view = view_list[i];
        if (!space_is_visible(view->sid)) continue;
        if (!view_is_dirty(view))         continue;

        window_node_flush(view->root);
        view_clear_flag(view, VIEW_IS_DIRTY);
    }

    if (workspace_is_macos_sequoia()) {
        update_window_notifications();
    }

out:
    process_destroy(process);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static EVENT_HANDLER(APPLICATION_FRONT_SWITCHED)
{
    struct process *process = context;
    struct application *application = window_manager_find_application(&g_window_manager, process->pid);

    if (!application) {
        window_manager_add_lost_front_switched_event(&g_window_manager, process->pid);
        return;
    }

    struct application *deactivated_application = window_manager_find_application(&g_window_manager, g_process_manager.front_pid);
    if (deactivated_application) event_signal_push(SIGNAL_APPLICATION_DEACTIVATED, deactivated_application);

    debug("%s: %s (%d)\n", __FUNCTION__, process->name, process->pid);
    event_signal_push(SIGNAL_APPLICATION_ACTIVATED, application);
    g_process_manager.switch_event_time = GetCurrentEventTime();
    g_process_manager.last_front_pid = g_process_manager.front_pid;
    g_process_manager.front_pid = process->pid;
    event_signal_push(SIGNAL_APPLICATION_FRONT_SWITCHED, NULL);

    for (int i = 0; i < buf_len(g_window_manager.applications_to_refresh); ++i) {
        if (application == g_window_manager.applications_to_refresh[i]) {
            debug("%s: %s has windows that are not yet resolved\n", __FUNCTION__, application->name);
            window_manager_add_existing_application_windows(&g_space_manager, &g_window_manager, application, i);
            break;
        }
    }

    uint32_t application_focused_window_id = application_focused_window(application);
    if (!application_focused_window_id) {
        struct window *focused_window = window_manager_find_window(&g_window_manager, g_window_manager.focused_window_id);
        if (focused_window) {
            window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.normal_window_opacity);
        }

        g_window_manager.last_window_id = g_window_manager.focused_window_id;
        g_window_manager.focused_window_id = 0;
        g_window_manager.focused_window_psn = application->psn;
        g_mouse_state.ffm_window_id = 0;
        return;
    }

    struct window *window = window_manager_find_window(&g_window_manager, application_focused_window_id);
    if (!window) {
        struct window *focused_window = window_manager_find_window(&g_window_manager, g_window_manager.focused_window_id);
        if (focused_window) {
            window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.normal_window_opacity);
        }

        window_manager_add_lost_focused_event(&g_window_manager, application_focused_window_id);
        return;
    }

    window_did_receive_focus(&g_window_manager, &g_mouse_state, window);
    event_signal_push(SIGNAL_WINDOW_FOCUSED, window);
}
#pragma clang diagnostic pop

static EVENT_HANDLER(APPLICATION_VISIBLE)
{
    struct application *application = window_manager_find_application(&g_window_manager, (pid_t)(intptr_t) context);
    if (!application) return;

    debug("%s: %s\n", __FUNCTION__, application->name);
    application->is_hidden = false;

    int window_count;
    struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);
    uint32_t prev_window_id = g_window_manager.last_window_id;

    int view_count = 0;
    struct view **view_list = ts_alloc_list(struct view *, window_count);

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_list[i];

        if (window_manager_should_manage_window(window) && !window_manager_find_managed_window(&g_window_manager, window)) {
            struct view *view = space_manager_find_view(&g_space_manager, window_space(window->id));
            if (view->layout == VIEW_FLOAT) continue;

            //
            // @cleanup
            //
            // :AXBatching
            //
            // NOTE(koekeishiya): Batch all operations and mark the view as dirty so that we can perform a single flush,
            // making sure that each window is only moved and resized a single time, when the final layout has been computed.
            // This is necessary to make sure that we do not call the AX API for each modification to the tree.
            //

            window_manager_adjust_layer(window, LAYER_BELOW);
            view_add_window_node_with_insertion_point(view, window, prev_window_id);
            window_manager_add_managed_window(&g_window_manager, window, view);

            view_set_flag(view, VIEW_IS_DIRTY);
            view_list[view_count++] = view;

            prev_window_id = window->id;
        }
    }

    //
    // @cleanup
    //
    // :AXBatching
    //
    // NOTE(koekeishiya): Flush previously batched operations if the view is marked as dirty.
    // This is necessary to make sure that we do not call the AX API for each modification to the tree.
    //

    for (int i = 0; i < view_count; ++i) {
        struct view *view = view_list[i];
        if (!space_is_visible(view->sid)) continue;
        if (!view_is_dirty(view))         continue;

        window_node_flush(view->root);
        view_clear_flag(view, VIEW_IS_DIRTY);
    }

    event_signal_push(SIGNAL_APPLICATION_VISIBLE, application);
}

static EVENT_HANDLER(APPLICATION_HIDDEN)
{
    struct application *application = window_manager_find_application(&g_window_manager, (pid_t)(intptr_t) context);
    if (!application) return;

    debug("%s: %s\n", __FUNCTION__, application->name);
    application->is_hidden = true;

    int window_count;
    struct window **window_list = window_manager_find_application_windows(&g_window_manager, application, &window_count);

    int view_count = 0;
    struct view **view_list = ts_alloc_list(struct view *, window_count);

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_list[i];

        struct view *view = window_manager_find_managed_window(&g_window_manager, window);
        if (view) {

            //
            // @cleanup
            //
            // :AXBatching
            //
            // NOTE(koekeishiya): Batch all operations and mark the view as dirty so that we can perform a single flush,
            // making sure that each window is only moved and resized a single time, when the final layout has been computed.
            // This is necessary to make sure that we do not call the AX API for each modification to the tree.
            //

            window_manager_adjust_layer(window, LAYER_NORMAL);
            view_remove_window_node(view, window);
            window_manager_remove_managed_window(&g_window_manager, window->id);
            window_manager_purify_window(&g_window_manager, window);

            view_set_flag(view, VIEW_IS_DIRTY);
            view_list[view_count++] = view;
        }
    }

    //
    // @cleanup
    //
    // :AXBatching
    //
    // NOTE(koekeishiya): Flush previously batched operations if the view is marked as dirty.
    // This is necessary to make sure that we do not call the AX API for each modification to the tree.
    //

    for (int i = 0; i < view_count; ++i) {
        struct view *view = view_list[i];
        if (!space_is_visible(view->sid)) continue;
        if (!view_is_dirty(view))         continue;

        window_node_flush(view->root);
        view_clear_flag(view, VIEW_IS_DIRTY);
    }

    event_signal_push(SIGNAL_APPLICATION_HIDDEN, application);
}

static EVENT_HANDLER(WINDOW_CREATED)
{
    uint32_t window_id = ax_window_id(context);
    if (!window_id) { CFRelease(context); return; }

    struct window *existing_window = window_manager_find_window(&g_window_manager, window_id);
    if (existing_window) { CFRelease(context); return; }

    pid_t window_pid = ax_window_pid(context);
    if (!window_pid) { CFRelease(context); return; }

    struct application *application = window_manager_find_application(&g_window_manager, window_pid);
    if (!application) { CFRelease(context); return; }

    struct window *window = window_manager_create_and_add_window(&g_space_manager, &g_window_manager, application, context, window_id, true);
    if (!window) return;

    int rule_len = buf_len(g_window_manager.rules);
    for (int i = 0; i < rule_len; ++i) {
        if (rule_check_flag(&g_window_manager.rules[i], RULE_ONE_SHOT_REMOVE)) {
            rule_destroy(&g_window_manager.rules[i]);
            if (buf_del(g_window_manager.rules, i)) {
                --i;
                --rule_len;
            }
        }
    }

    if (window_manager_should_manage_window(window) && !window_manager_find_managed_window(&g_window_manager, window)) {
        uint64_t sid;

        if (g_window_manager.window_origin_mode == WINDOW_ORIGIN_DEFAULT) {
            sid = window_space(window->id);
        } else if (g_window_manager.window_origin_mode == WINDOW_ORIGIN_FOCUSED) {
            sid = g_space_manager.current_space_id;
        } else /* if (g_window_manager.window_origin_mode == WINDOW_ORIGIN_CURSOR) */ {
            sid = space_manager_cursor_space();
        }

        struct view *view = space_manager_tile_window_on_space(&g_space_manager, window, sid);
        window_manager_add_managed_window(&g_window_manager, window, view);
    }

    if (window_manager_is_window_eligible(window)) {
        event_signal_push(SIGNAL_WINDOW_CREATED, window);
    }

    if (workspace_is_macos_sequoia()) {
        update_window_notifications();
    }
}

static EVENT_HANDLER(WINDOW_DESTROYED)
{
    struct window *window = context;
    if (!window || window->id == 0) {
        debug("%s: window has already been destroyed, ignoring event..\n", __FUNCTION__);
        return;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application ? window->application->name : "<unknown>", window->id);

    struct view *view = window_manager_find_managed_window(&g_window_manager, window);
    if (view) {
        space_manager_untile_window(view, window);
        window_manager_remove_managed_window(&g_window_manager, window->id);
    }

    if (g_mouse_state.window == window) g_mouse_state.window = NULL;

    if (window->is_eligible) {
        event_signal_push(SIGNAL_WINDOW_DESTROYED, window);
    }

    window_manager_remove_scratchpad_for_window(&g_window_manager, window, false);
    window_manager_remove_window(&g_window_manager, window->id);
    window_unobserve(window);
    window_destroy(window);

    if (workspace_is_macos_sequoia()) {
        update_window_notifications();
    }
}

static EVENT_HANDLER(WINDOW_FOCUSED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);

    if (!window) {
        window_manager_add_lost_focused_event(&g_window_manager, window_id);
        return;
    }

    if (!__sync_bool_compare_and_swap(&window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return;
    }

    if (window_check_flag(window, WINDOW_MINIMIZE)) {
        window_manager_add_lost_focused_event(&g_window_manager, window->id);
        return;
    }

    if (!application_is_frontmost(window->application)) {
        return;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    window_did_receive_focus(&g_window_manager, &g_mouse_state, window);
    event_signal_push(SIGNAL_WINDOW_FOCUSED, window);
}

static EVENT_HANDLER(WINDOW_MOVED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return;

    if (!__sync_bool_compare_and_swap(&window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return;
    }

    if (window->application->is_hidden) {
        debug("%s: %d was moved while the application is hidden, ignoring event..\n", __FUNCTION__, window_id);
        return;
    }

    CGPoint new_origin = window_ax_origin(window);
    if (CGPointEqualToPoint(new_origin, window->frame.origin)) {
        debug("%s:DEBOUNCED %s %d\n", __FUNCTION__, window->application->name, window->id);
        return;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    event_signal_push(SIGNAL_WINDOW_MOVED, window);
    bool windowed_fullscreen = CGRectEqualToRect(window->windowed_frame, window->frame);
    window->frame.origin = new_origin;

    if (!windowed_fullscreen) {
        window_clear_flag(window, WINDOW_WINDOWED);

        if (!g_mouse_state.window || g_mouse_state.window != window) {
            struct view *view = window_manager_find_managed_window(&g_window_manager, window);
            if (view) {
                struct window_node *node = view_find_window_node(view, window->id);
                if (node && (AX_DIFF(node->area.x, new_origin.x) ||
                             AX_DIFF(node->area.y, new_origin.y))
                         &&
                   (!node->zoom || AX_DIFF(node->zoom->area.x, new_origin.x) ||
                                   AX_DIFF(node->zoom->area.y, new_origin.y))) {
                    if (space_is_visible(view->sid)) {
                        window_node_flush(node);
                    } else {
                        view_set_flag(view, VIEW_IS_DIRTY);
                    }
                }
            }
        }
    }
}

static EVENT_HANDLER(WINDOW_RESIZED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return;

    if (!__sync_bool_compare_and_swap(&window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return;
    }

    if (window->application->is_hidden) {
        debug("%s: %d was resized while the application is hidden, ignoring event..\n", __FUNCTION__, window_id);
        return;
    }

    CGRect new_frame = window_ax_frame(window);
    if (CGRectEqualToRect(new_frame, window->frame)) {
        debug("%s:DEBOUNCED %s %d\n", __FUNCTION__, window->application->name, window->id);
        return;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    event_signal_push(SIGNAL_WINDOW_RESIZED, window);

    bool was_fullscreen = window_check_flag(window, WINDOW_FULLSCREEN);

    bool is_fullscreen = window_is_fullscreen(window);
    if (is_fullscreen) {
        window_set_flag(window, WINDOW_FULLSCREEN);
    } else {
        window_clear_flag(window, WINDOW_FULLSCREEN);
    }

    if (was_fullscreen != is_fullscreen) {
        if (window_ax_can_move(window)) {
            window_set_flag(window, WINDOW_MOVABLE);
        } else {
            window_clear_flag(window, WINDOW_MOVABLE);
        }

        if (window_ax_can_resize(window)) {
            window_set_flag(window, WINDOW_RESIZABLE);
        } else {
            window_clear_flag(window, WINDOW_RESIZABLE);
        }

        if (window->role) CFRelease(window->role);
        window->role = window_ax_role(window);

        if (window->subrole) CFRelease(window->subrole);
        window->subrole = window_ax_subrole(window);
    }

    bool windowed_fullscreen = CGRectEqualToRect(window->windowed_frame, window->frame);
    window->frame = new_frame;

    if (!was_fullscreen && is_fullscreen) {
        struct view *view = window_manager_find_managed_window(&g_window_manager, window);
        if (view) {
            space_manager_untile_window(view, window);
            window_manager_remove_managed_window(&g_window_manager, window->id);
            window_manager_purify_window(&g_window_manager, window);
        }
    } else if (was_fullscreen && !is_fullscreen) {
        window_manager_wait_for_native_fullscreen_transition(window);

        if (window_manager_should_manage_window(window) && !window_manager_find_managed_window(&g_window_manager, window)) {
            struct view *view = space_manager_tile_window_on_space(&g_space_manager, window, window_space(window->id));
            window_manager_add_managed_window(&g_window_manager, window, view);
        }
    } else if (!was_fullscreen == !is_fullscreen) {
        if (g_mouse_state.current_action == MOUSE_MODE_MOVE && g_mouse_state.window == window) {
            g_mouse_state.window_frame.size = g_mouse_state.window->frame.size;
        }

        if (!windowed_fullscreen) {
            window_clear_flag(window, WINDOW_WINDOWED);

            if (!g_mouse_state.window || g_mouse_state.window != window) {
                struct view *view = window_manager_find_managed_window(&g_window_manager, window);
                if (view) {
                    struct window_node *node = view_find_window_node(view, window->id);
                    if (node && (AX_DIFF(node->area.x, new_frame.origin.x)   ||
                                 AX_DIFF(node->area.y, new_frame.origin.y)   ||
                                 AX_DIFF(node->area.w, new_frame.size.width) ||
                                 AX_DIFF(node->area.h, new_frame.size.height))
                             &&
                       (!node->zoom || AX_DIFF(node->zoom->area.x, new_frame.origin.x)   ||
                                       AX_DIFF(node->zoom->area.y, new_frame.origin.y)   ||
                                       AX_DIFF(node->zoom->area.w, new_frame.size.width) ||
                                       AX_DIFF(node->zoom->area.h, new_frame.size.height))) {
                        if (space_is_visible(view->sid)) {
                            window_node_flush(node);
                        } else {
                            view_set_flag(view, VIEW_IS_DIRTY);
                        }
                    }
                }
            }
        }
    }
}

static EVENT_HANDLER(WINDOW_MINIMIZED)
{
    struct window *window = context;

    if (!__sync_bool_compare_and_swap(&window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window->id);
        return;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    window_set_flag(window, WINDOW_MINIMIZE);

    if (window_ax_can_move(window)) {
        window_set_flag(window, WINDOW_MOVABLE);
    } else {
        window_clear_flag(window, WINDOW_MOVABLE);
    }

    if (window_ax_can_resize(window)) {
        window_set_flag(window, WINDOW_RESIZABLE);
    } else {
        window_clear_flag(window, WINDOW_RESIZABLE);
    }

    if (window->role) CFRelease(window->role);
    window->role = window_ax_role(window);

    if (window->subrole) CFRelease(window->subrole);
    window->subrole = window_ax_subrole(window);

    if (window->id == g_window_manager.last_window_id) {
        g_window_manager.last_window_id = g_window_manager.focused_window_id;
    }

    struct view *view = window_manager_find_managed_window(&g_window_manager, window);
    if (view) {
        space_manager_untile_window(view, window);
        window_manager_remove_managed_window(&g_window_manager, window->id);
        window_manager_purify_window(&g_window_manager, window);
    }

    event_signal_push(SIGNAL_WINDOW_MINIMIZED, window);
}

static EVENT_HANDLER(WINDOW_DEMINIMIZED)
{
    struct window *window = context;

    if (!__sync_bool_compare_and_swap(&window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window->id);
        window_manager_remove_lost_focused_event(&g_window_manager, window->id);
        return;
    }

    window_clear_flag(window, WINDOW_MINIMIZE);

    if (window_ax_can_move(window)) {
        window_set_flag(window, WINDOW_MOVABLE);
    } else {
        window_clear_flag(window, WINDOW_MOVABLE);
    }

    if (window_ax_can_resize(window)) {
        window_set_flag(window, WINDOW_RESIZABLE);
    } else {
        window_clear_flag(window, WINDOW_RESIZABLE);
    }

    if (window->role) CFRelease(window->role);
    window->role = window_ax_role(window);

    if (window->subrole) CFRelease(window->subrole);
    window->subrole = window_ax_subrole(window);

    uint64_t sid = space_manager_active_space();
    if (space_manager_is_window_on_space(sid, window)) {
        debug("%s: window %s %d is deminimized on active space\n", __FUNCTION__, window->application->name, window->id);
        if (window_manager_should_manage_window(window) && !window_manager_find_managed_window(&g_window_manager, window)) {
            struct window *last_window = window_manager_find_window(&g_window_manager, g_window_manager.last_window_id);
            uint32_t insertion_point = last_window && last_window->application->pid != window->application->pid ? last_window->id : 0;
            struct view *view = space_manager_tile_window_on_space_with_insertion_point(&g_space_manager, window, sid, insertion_point);
            window_manager_add_managed_window(&g_window_manager, window, view);
        }
    } else {
        debug("%s: window %s %d is deminimized on inactive space\n", __FUNCTION__, window->application->name, window->id);
    }

    if (window_manager_find_lost_focused_event(&g_window_manager, window->id)) {
        event_loop_post(&g_event_loop, WINDOW_FOCUSED, (void *)(intptr_t) window->id, 0);
        window_manager_remove_lost_focused_event(&g_window_manager, window->id);
    }

    event_signal_push(SIGNAL_WINDOW_DEMINIMIZED, window);
}

static EVENT_HANDLER(WINDOW_TITLE_CHANGED)
{
    uint32_t window_id = (uint32_t)(intptr_t) context;
    struct window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return;

    if (!__sync_bool_compare_and_swap(&window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, window_id);
        return;
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);

    if (window->title) CFRelease(window->title);

    window->title = window_title(window);

    event_signal_push(SIGNAL_WINDOW_TITLE_CHANGED, window);
}

static EVENT_HANDLER(SLS_WINDOW_ORDERED)
{
    uint32_t wid = (uint64_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, wid);
    struct window_node *node = table_find(&g_window_manager.insert_feedback, &wid);
    if (node) SLSOrderWindow(g_connection, node->feedback_window.id, 1, node->window_order[0]);
}

static EVENT_HANDLER(SLS_WINDOW_DESTROYED)
{
    uint32_t wid = (uint64_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, wid);

    struct window *window = window_manager_find_window(&g_window_manager, wid);
    if (!window) return;

    if (!__sync_bool_compare_and_swap(&window->id_ptr, &window->id, &window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, wid);
        return;
    }

    EVENT_HANDLER_WINDOW_DESTROYED(window, 0);
}

static EVENT_HANDLER(SLS_SPACE_CREATED)
{
    uint64_t sid = (uint64_t)(intptr_t) context;
    int type = SLSSpaceGetType(g_connection, sid);

    if (type == 0 || type == 4) {
        debug("%s: %lld, %d\n", __FUNCTION__, sid, type);
        space_manager_find_view(&g_space_manager, sid);
        event_signal_push(SIGNAL_SPACE_CREATED, context);
    }
}

static EVENT_HANDLER(SLS_SPACE_DESTROYED)
{
    uint64_t sid = (uint64_t)(intptr_t) context;
    struct view *view = table_find(&g_space_manager.view, &sid);
    if (view) {
        debug("%s: %lld\n", __FUNCTION__, sid);
        space_manager_remove_label_for_space(&g_space_manager, sid);
        table_remove(&g_space_manager.view, &sid);
        view_destroy(view);
        free(view);
        event_signal_push(SIGNAL_SPACE_DESTROYED, context);
    }
}

static EVENT_HANDLER(SPACE_CHANGED)
{
    g_space_manager.last_space_id = g_space_manager.current_space_id;
    g_space_manager.current_space_id = space_manager_active_space();

    if (g_window_manager.menubar_opacity != 1.0f) {
        float alpha = space_is_fullscreen(g_space_manager.current_space_id) ? 1.0f : g_window_manager.menubar_opacity;
        SLSSetMenuBarInsetAndAlpha(g_connection, 0, 1, alpha);
    }

    debug("%s: %lld\n", __FUNCTION__, g_space_manager.current_space_id);
    struct view *view = space_manager_find_view(&g_space_manager, g_space_manager.current_space_id);

    if (space_manager_refresh_application_windows(&g_space_manager)) {
        struct window *focused_window = window_manager_focused_window(&g_window_manager);
        if (focused_window && window_manager_find_lost_focused_event(&g_window_manager, focused_window->id)) {
            window_did_receive_focus(&g_window_manager, &g_mouse_state, focused_window);
            window_manager_remove_lost_focused_event(&g_window_manager, focused_window->id);
        }
    }

    if (!mission_control_is_active() && space_is_user(g_space_manager.current_space_id)) {
        window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);

        //
        // NOTE: Some applications (e.g. Arc browser) do not properly update their
        // accessibility attributes when exiting fullscreen. We detect and correct this state
        // during space changes to ensure windows are properly managed.
        //
        
        table_for (struct window *window, g_window_manager.window, {
            if (window && strcmp(window->application->name, "Arc") == 0) {
                bool was_fullscreen = window_check_flag(window, WINDOW_FULLSCREEN);
                bool is_fullscreen = window_is_fullscreen(window);
                
                if (was_fullscreen && (!is_fullscreen || (is_fullscreen && space_is_user(g_space_manager.current_space_id)))) {
                    window_clear_flag(window, WINDOW_FULLSCREEN);
                    
                    if (window_ax_can_move(window)) {
                        window_set_flag(window, WINDOW_MOVABLE);
                    }
                    
                    if (window_ax_can_resize(window)) {
                        window_set_flag(window, WINDOW_RESIZABLE);
                    }
                    
                    if (window->role) CFRelease(window->role);
                    window->role = window_ax_role(window);
                    
                    if (window->subrole) CFRelease(window->subrole);
                    window->subrole = window_ax_subrole(window);
                    
                    if (window_manager_should_manage_window(window) && !window_manager_find_managed_window(&g_window_manager, window)) {
                        struct view *space_view = space_manager_tile_window_on_space(&g_space_manager, window, g_space_manager.current_space_id);
                        window_manager_add_managed_window(&g_window_manager, window, space_view);
                    }
                }
            }
        })

        if (view_is_invalid(view)) {
            view_update(view);
        }

        if (view_is_dirty(view)) {
            window_node_flush(view->root);
            view_clear_flag(view, VIEW_IS_DIRTY);
        }
    }

    event_signal_push(SIGNAL_SPACE_CHANGED, NULL);
}

static EVENT_HANDLER(DISPLAY_CHANGED)
{
    uint32_t new_did = display_manager_active_display_id();
    if (g_display_manager.current_display_id == new_did) {
        debug("%s: newly activated display %d was already active (%d)! ignoring event..\n", __FUNCTION__, g_display_manager.current_display_id, new_did);
        return;
    }

    g_display_manager.last_display_id = g_display_manager.current_display_id;
    g_display_manager.current_display_id = new_did;

    g_space_manager.last_space_id = g_space_manager.current_space_id;
    g_space_manager.current_space_id = display_space_id(g_display_manager.current_display_id);

    uint32_t expected_display_id = space_display_id(g_space_manager.current_space_id);
    if (g_display_manager.current_display_id != expected_display_id) {
        debug("%s: %d %lld did not match %d! ignoring event..\n", __FUNCTION__, g_display_manager.current_display_id, g_space_manager.current_space_id, expected_display_id);
        return;
    }

    if (g_window_manager.menubar_opacity != 1.0f) {
        float alpha = space_is_fullscreen(g_space_manager.current_space_id) ? 1.0f : g_window_manager.menubar_opacity;
        SLSSetMenuBarInsetAndAlpha(g_connection, 0, 1, alpha);
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

    if (!mission_control_is_active() && space_is_user(g_space_manager.current_space_id)) {
        window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, g_space_manager.current_space_id);

        if (view_is_invalid(view)) {
            view_update(view);
        }

        if (view_is_dirty(view)) {
            window_node_flush(view->root);
            view_clear_flag(view, VIEW_IS_DIRTY);
        }
    }

    event_signal_push(SIGNAL_DISPLAY_CHANGED, NULL);
}

static EVENT_HANDLER(DISPLAY_ADDED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    space_manager_handle_display_add(&g_space_manager, did);
    window_manager_handle_display_add_and_remove(&g_space_manager, &g_window_manager, did);
    event_signal_push(SIGNAL_DISPLAY_ADDED, context);
}

static EVENT_HANDLER(DISPLAY_REMOVED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    display_manager_remove_label_for_display(&g_display_manager, did);
    window_manager_handle_display_add_and_remove(&g_space_manager, &g_window_manager, display_manager_main_display_id());
    event_signal_push(SIGNAL_DISPLAY_REMOVED, context);
}

static EVENT_HANDLER(DISPLAY_MOVED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    space_manager_mark_spaces_invalid(&g_space_manager);
    event_signal_push(SIGNAL_DISPLAY_MOVED, context);
}

static EVENT_HANDLER(DISPLAY_RESIZED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    space_manager_mark_spaces_invalid_for_display(&g_space_manager, did);
    event_signal_push(SIGNAL_DISPLAY_RESIZED, context);
}

static EVENT_HANDLER(MOUSE_DOWN)
{
    if (mission_control_is_active())                     goto out;
    if (g_mouse_state.current_action != MOUSE_MODE_NONE) goto out;

    CGPoint point = CGEventGetLocation(context);
    debug("%s: %.2f, %.2f\n", __FUNCTION__, point.x, point.y);

    struct window *window = window_manager_find_window_at_point(&g_window_manager, point);
    if (!window || window_check_flag(window, WINDOW_FULLSCREEN)) goto out;

    g_mouse_state.window = window;
    g_mouse_state.window_frame = g_mouse_state.window->frame;
    g_mouse_state.down_location = point;
    g_mouse_state.direction = 0;

    int64_t button = CGEventGetIntegerValueField(context, kCGMouseEventButtonNumber);
    uint8_t mod = (uint8_t) param1;

    if (button == kCGMouseButtonLeft && g_mouse_state.modifier == mod) {
        g_mouse_state.current_action = g_mouse_state.action1;
    } else if (button == kCGMouseButtonRight && g_mouse_state.modifier == mod) {
        g_mouse_state.current_action = g_mouse_state.action2;
    }

    if (g_mouse_state.current_action == MOUSE_MODE_RESIZE) {
        CGPoint frame_mid = { CGRectGetMidX(g_mouse_state.window_frame), CGRectGetMidY(g_mouse_state.window_frame) };
        if (point.x < frame_mid.x) g_mouse_state.direction |= HANDLE_LEFT;
        if (point.y < frame_mid.y) g_mouse_state.direction |= HANDLE_TOP;
        if (point.x > frame_mid.x) g_mouse_state.direction |= HANDLE_RIGHT;
        if (point.y > frame_mid.y) g_mouse_state.direction |= HANDLE_BOTTOM;
    }

out:
    CFRelease(context);
}

static EVENT_HANDLER(MOUSE_UP)
{
    if (mission_control_is_active()) goto out;
    if (!g_mouse_state.window)       goto res;

    if (!__sync_bool_compare_and_swap(&g_mouse_state.window->id_ptr, &g_mouse_state.window->id, &g_mouse_state.window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, g_mouse_state.window->id);
        goto err;
    }

    if (window_check_flag(g_mouse_state.window, WINDOW_FULLSCREEN)) {
        debug("%s: %d is transitioning into native-fullscreen mode, ignoring event..\n", __FUNCTION__, g_mouse_state.window->id);
        goto err;
    }

    CGPoint point = CGEventGetLocation(context);
    debug("%s: %.2f, %.2f\n", __FUNCTION__, point.x, point.y);

    struct view *src_view = window_manager_find_managed_window(&g_window_manager, g_mouse_state.window);
    if (!src_view) goto err;

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
            if (g_mouse_state.feedback_node) {
                g_mouse_state.feedback_node->insert_dir = 0;
                insert_feedback_destroy(g_mouse_state.feedback_node);
                g_mouse_state.feedback_node = NULL;
            }

            enum mouse_drop_action drop_action = mouse_determine_drop_action(&g_mouse_state, a_node, window, point);
            switch (drop_action) {
            case MOUSE_DROP_ACTION_STACK: {
                mouse_drop_action_stack(&g_window_manager, src_view, g_mouse_state.window, dst_view, window);
            } break;
            case MOUSE_DROP_ACTION_SWAP: {
                mouse_drop_action_swap(&g_window_manager, src_view, a_node, g_mouse_state.window, dst_view, b_node, window);
            } break;
            case MOUSE_DROP_ACTION_WARP_TOP: {
                mouse_drop_action_warp(&g_window_manager, src_view, a_node, g_mouse_state.window, dst_view, b_node, window, SPLIT_X, CHILD_FIRST);
            } break;
            case MOUSE_DROP_ACTION_WARP_RIGHT: {
                mouse_drop_action_warp(&g_window_manager, src_view, a_node, g_mouse_state.window, dst_view, b_node, window, SPLIT_Y, CHILD_SECOND);
            } break;
            case MOUSE_DROP_ACTION_WARP_BOTTOM: {
                mouse_drop_action_warp(&g_window_manager, src_view, a_node, g_mouse_state.window, dst_view, b_node, window, SPLIT_X, CHILD_SECOND);
            } break;
            case MOUSE_DROP_ACTION_WARP_LEFT: {
                mouse_drop_action_warp(&g_window_manager, src_view, a_node, g_mouse_state.window, dst_view, b_node, window, SPLIT_Y, CHILD_FIRST);
            } break;
            case MOUSE_DROP_ACTION_NONE: {
                /* silence compiler warning.. */
            } break;
            }
        } else if (a_node) {
            mouse_drop_no_target(&g_space_manager, &g_window_manager, src_view, dst_view, g_mouse_state.window, a_node);
        }
    } else if (info.changed_position || info.changed_size) {
        mouse_drop_try_adjust_bsp_grid(&g_window_manager, src_view, g_mouse_state.window, &info);
    }

err:
    g_mouse_state.window = NULL;
res:
    g_mouse_state.current_action = MOUSE_MODE_NONE;
out:
    CFRelease(context);
}

static EVENT_HANDLER(MOUSE_DRAGGED)
{
    if (mission_control_is_active()) goto out;
    if (!g_mouse_state.window)       goto out;

    if (!__sync_bool_compare_and_swap(&g_mouse_state.window->id_ptr, &g_mouse_state.window->id, &g_mouse_state.window->id)) {
        debug("%s: %d has been marked invalid by the system, ignoring event..\n", __FUNCTION__, g_mouse_state.window->id);
        g_mouse_state.window = NULL;
        g_mouse_state.current_action = MOUSE_MODE_NONE;
        CFRelease(context);
        return;
    }

    CGPoint point = CGEventGetLocation(context);
    debug("%s: %.2f, %.2f\n", __FUNCTION__, point.x, point.y);

    if (g_mouse_state.current_action == MOUSE_MODE_MOVE) {
        CGPoint new_point = { g_mouse_state.window_frame.origin.x + (point.x - g_mouse_state.down_location.x),
                              g_mouse_state.window_frame.origin.y + (point.y - g_mouse_state.down_location.y) };

        uint32_t did = display_manager_point_display_id(new_point);
        if (did) {
            CGRect bounds = display_bounds_constrained(did, false);
            if (new_point.y < bounds.origin.y) new_point.y = bounds.origin.y;
        }

        if (!scripting_addition_move_window(g_mouse_state.window->id, new_point.x, new_point.y)) {
            window_manager_move_window(g_mouse_state.window, new_point.x, new_point.y);
        }
    } else if (g_mouse_state.current_action == MOUSE_MODE_RESIZE) {
        uint64_t event_time = read_os_timer();
        float dt = ((float) event_time - g_mouse_state.last_moved_time) * (1000.0f / (float)read_os_freq());
        if (dt < 67.67f) goto out;

        int dx = point.x - g_mouse_state.down_location.x;
        int dy = point.y - g_mouse_state.down_location.y;

        window_manager_resize_window_relative_internal(g_mouse_state.window, g_mouse_state.window->frame, g_mouse_state.direction, dx, dy, false);

        g_mouse_state.last_moved_time = event_time;
        g_mouse_state.down_location = point;
    }

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
            if (g_mouse_state.feedback_node && g_mouse_state.feedback_node != b_node) {
                g_mouse_state.feedback_node->insert_dir = 0;
                insert_feedback_destroy(g_mouse_state.feedback_node);
            }

            int insert_dir = 0;
            enum mouse_drop_action drop_action = mouse_determine_drop_action(&g_mouse_state, a_node, window, point);
            switch (drop_action) {
            case MOUSE_DROP_ACTION_STACK: {
                insert_dir = STACK;
            } break;
            case MOUSE_DROP_ACTION_SWAP: {
                insert_dir = STACK;
            } break;
            case MOUSE_DROP_ACTION_WARP_TOP: {
                insert_dir = DIR_NORTH;
            } break;
            case MOUSE_DROP_ACTION_WARP_RIGHT: {
                insert_dir = DIR_EAST;
            } break;
            case MOUSE_DROP_ACTION_WARP_BOTTOM: {
                insert_dir = DIR_SOUTH;
            } break;
            case MOUSE_DROP_ACTION_WARP_LEFT: {
                insert_dir = DIR_WEST;
            } break;
            case MOUSE_DROP_ACTION_NONE: {
                /* silence compiler warning.. */
            } break;
            }

            if (b_node->insert_dir != insert_dir) {
                b_node->insert_dir = insert_dir;
                insert_feedback_show(b_node);
                g_mouse_state.feedback_node = b_node;
            }
        } else if (!b_node) {
            if (g_mouse_state.feedback_node) {
                g_mouse_state.feedback_node->insert_dir = 0;
                insert_feedback_destroy(g_mouse_state.feedback_node);
                g_mouse_state.feedback_node = NULL;
            }
        }
    }

out:
    CFRelease(context);
}

static EVENT_HANDLER(MOUSE_MOVED)
{
    if (g_window_manager.ffm_mode == FFM_DISABLED) goto out;
    if (mission_control_is_active())               goto out;
    if (g_mouse_state.ffm_window_id)               goto out;

    CGPoint point = CGEventGetLocation(context);
    struct window *window = window_manager_find_window_at_point(&g_window_manager, point);

    if (window) {
        if (window->id == g_window_manager.focused_window_id) goto out;
        if (!window_manager_is_window_eligible(window))       goto out;

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
        } else if (g_window_manager.ffm_mode == FFM_AUTORAISE) {

            //
            // NOTE(koekeishiya): If any **floating** window would be fully occluded by
            // autoraising the window below the cursor we do not actually perform the
            // focus change, as it is likely that the user is trying to reach for the
            // smaller window that sits on top of the window we would otherwise raise.
            //

            bool occludes_window = false;

            int window_count;
            uint32_t *window_list = space_window_list(g_space_manager.current_space_id, &window_count, false);

            if (window_list) {
                for (int i = 0; i < window_count; ++i) {
                    uint32_t wid = window_list[i];
                    if (wid == window->id) break;

                    struct window *sub_window = window_manager_find_window(&g_window_manager, wid);
                    if (!sub_window) continue;

                    if (!window_check_flag(sub_window, WINDOW_FLOAT))                     continue;
                    if (window_level(window->id) != window_level(sub_window->id))         continue;
                    if (window_sub_level(window->id) != window_sub_level(sub_window->id)) continue;

                    if (CGRectContainsRect(window->frame, sub_window->frame)) {
                        occludes_window = true;
                        break;
                    }
                }
            }

            if (!occludes_window) {
                window_manager_focus_window_with_raise(&window->application->psn, window->id, window->ref);
                g_mouse_state.ffm_window_id = window->id;
            }
        }
    } else {
        uint32_t cursor_did = display_manager_point_display_id(point);
        if (g_display_manager.current_display_id == cursor_did) goto out;

        CGRect bounds = display_bounds_constrained(cursor_did, false);
        if (!cgrect_contains_point(bounds, point)) goto out;

        uint32_t wid = display_manager_focus_display_with_window_at_point(point);
        if (!wid) display_manager_set_active_display_id(cursor_did);
        g_mouse_state.ffm_window_id = wid;
    }

out:
    CFRelease(context);
}

static EVENT_HANDLER(MISSION_CONTROL_SHOW_ALL_WINDOWS)
{
    debug("%s:\n", __FUNCTION__);
    g_mission_control_mode = MISSION_CONTROL_MODE_SHOW_ALL_WINDOWS;
    event_signal_push(SIGNAL_MISSION_CONTROL_ENTER, (void*)(uintptr_t)g_mission_control_mode);
}

static EVENT_HANDLER(MISSION_CONTROL_SHOW_FRONT_WINDOWS)
{
    debug("%s:\n", __FUNCTION__);
    g_mission_control_mode = MISSION_CONTROL_MODE_SHOW_FRONT_WINDOWS;
    event_signal_push(SIGNAL_MISSION_CONTROL_ENTER, (void*)(uintptr_t)g_mission_control_mode);
}

static EVENT_HANDLER(MISSION_CONTROL_SHOW_DESKTOP)
{
    debug("%s:\n", __FUNCTION__);
    g_mission_control_mode = MISSION_CONTROL_MODE_SHOW_DESKTOP;
    event_signal_push(SIGNAL_MISSION_CONTROL_ENTER, (void*)(uintptr_t)g_mission_control_mode);
}

static EVENT_HANDLER(MISSION_CONTROL_ENTER)
{
    debug("%s:\n", __FUNCTION__);
    g_mission_control_mode = MISSION_CONTROL_MODE_SHOW;

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.1f * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        event_loop_post(&g_event_loop, MISSION_CONTROL_CHECK_FOR_EXIT, NULL, 0);
    });

    event_signal_push(SIGNAL_MISSION_CONTROL_ENTER, (void*)(uintptr_t)g_mission_control_mode);
}

static EVENT_HANDLER(MISSION_CONTROL_CHECK_FOR_EXIT)
{
    if (!mission_control_is_active()) return;

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
            event_loop_post(&g_event_loop, MISSION_CONTROL_CHECK_FOR_EXIT, NULL, 0);
        });
    } else {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.0f), dispatch_get_main_queue(), ^{
            event_loop_post(&g_event_loop, MISSION_CONTROL_EXIT, NULL, 0);
        });
    }

    CFRelease(window_list);
}

static EVENT_HANDLER(MISSION_CONTROL_EXIT)
{
    debug("%s:\n", __FUNCTION__);

    if (g_window_manager.menubar_opacity != 1.0f) {
        float alpha = space_is_fullscreen(g_space_manager.current_space_id) ? 1.0f : g_window_manager.menubar_opacity;
        SLSSetMenuBarInsetAndAlpha(g_connection, 0, 1, alpha);
    }

    if (g_mission_control_mode == MISSION_CONTROL_MODE_SHOW || g_mission_control_mode == MISSION_CONTROL_MODE_SHOW_ALL_WINDOWS) {
        window_manager_correct_for_mission_control_changes(&g_space_manager, &g_window_manager);
    }

    event_signal_push(SIGNAL_MISSION_CONTROL_EXIT, (void*)(uintptr_t)g_mission_control_mode);
    g_mission_control_mode = MISSION_CONTROL_MODE_INACTIVE;
}

static EVENT_HANDLER(DOCK_DID_RESTART)
{
    debug("%s:\n", __FUNCTION__);

    if (workspace_is_macos_monterey() ||
        workspace_is_macos_ventura() ||
        workspace_is_macos_sonoma() ||
        workspace_is_macos_sequoia()) {
        mission_control_unobserve();
        mission_control_observe();
    }

    event_signal_push(SIGNAL_DOCK_DID_RESTART, NULL);
}

static enum ffm_mode ffm_value;
static int is_menu_open = 0;

static EVENT_HANDLER(MENU_OPENED)
{
    debug("%s\n", __FUNCTION__);
    ++is_menu_open;

    if (is_menu_open == 1) {
        ffm_value = g_window_manager.ffm_mode;
        g_window_manager.ffm_mode = FFM_DISABLED;
    }
}

static EVENT_HANDLER(MENU_CLOSED)
{
    debug("%s\n", __FUNCTION__);
    --is_menu_open;

    if (is_menu_open < 0) {
        is_menu_open = 0;
    }

    if (is_menu_open == 0) {
        g_window_manager.ffm_mode = ffm_value;
    }
}

static EVENT_HANDLER(MENU_BAR_HIDDEN_CHANGED)
{
    debug("%s:\n", __FUNCTION__);
    space_manager_mark_spaces_invalid(&g_space_manager);
    event_signal_push(SIGNAL_MENU_BAR_HIDDEN_CHANGED, NULL);
}

static EVENT_HANDLER(DOCK_DID_CHANGE_PREF)
{
    debug("%s:\n", __FUNCTION__);
    space_manager_mark_spaces_invalid(&g_space_manager);
    event_signal_push(SIGNAL_DOCK_DID_CHANGE_PREF, NULL);
}

static EVENT_HANDLER(SYSTEM_WOKE)
{
    debug("%s:\n", __FUNCTION__);

    struct window *focused_window = window_manager_find_window(&g_window_manager, g_window_manager.focused_window_id);
    if (focused_window) {
        window_manager_set_window_opacity(&g_window_manager, focused_window, g_window_manager.active_window_opacity);
        window_manager_center_mouse(&g_window_manager, focused_window);
    }

    event_signal_push(SIGNAL_SYSTEM_WOKE, NULL);
}

static EVENT_HANDLER(DAEMON_MESSAGE)
{
    TIME_FUNCTION;

    FILE *rsp         = NULL;
    int bytes_read    = 0;
    int bytes_to_read = 0;

    if (read(param1, &bytes_to_read, sizeof(int)) == sizeof(int)) {
        char *message = ts_alloc_unaligned(bytes_to_read);

        do {
            int cur_read = read(param1, message+bytes_read, bytes_to_read-bytes_read);
            if (cur_read <= 0) break;

            bytes_read += cur_read;
        } while (bytes_read < bytes_to_read);

        if ((bytes_read == bytes_to_read) && (rsp = fdopen(param1, "w"))) {
            debug_message(__FUNCTION__, message);
            handle_message(rsp, message);

            fflush(rsp);
            fclose(rsp);

            return;
        }
    }

    socket_close(param1);
}
#pragma clang diagnostic pop

static void *event_loop_run(void *context)
{
    struct event *head, *next;
    struct event_loop *event_loop = context;

    while (event_loop->is_running) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        for (;;) {
            profile_begin();

            do {
                head = __atomic_load_n(&event_loop->head, __ATOMIC_RELAXED);
                next = __atomic_load_n(&head->next, __ATOMIC_RELAXED);
                if (!next) goto empty;
            } while (!__sync_bool_compare_and_swap(&event_loop->head, head, next));

            switch (__atomic_load_n(&next->type, __ATOMIC_RELAXED)) {
#define EVENT_TYPE_ENTRY(value) case value: EVENT_HANDLER_##value(__atomic_load_n(&next->context, __ATOMIC_RELAXED), __atomic_load_n(&next->param1, __ATOMIC_RELAXED)); break;
                EVENT_TYPE_LIST
#undef EVENT_TYPE_ENTRY
            }

            event_signal_flush();
            ts_reset();

            profile_end_and_print();
        }

empty:
        [pool drain];
        sem_wait(event_loop->semaphore);
    }

    return NULL;
}

void event_loop_post(struct event_loop *event_loop, enum event_type type, void *context, int param1)
{
    bool success;
    struct event *tail, *new_tail;

    new_tail = memory_pool_push(&event_loop->pool, sizeof(struct event));
    __atomic_store_n(&new_tail->type, type, __ATOMIC_RELEASE);
    __atomic_store_n(&new_tail->param1, param1, __ATOMIC_RELEASE);
    __atomic_store_n(&new_tail->context, context, __ATOMIC_RELEASE);
    __atomic_store_n(&new_tail->next, NULL, __ATOMIC_RELEASE);
    __asm__ __volatile__ ("" ::: "memory");

    do {
        tail = __atomic_load_n(&event_loop->tail, __ATOMIC_RELAXED);
        success = __sync_bool_compare_and_swap(&tail->next, NULL, new_tail);
    } while (!success);
    __sync_bool_compare_and_swap(&event_loop->tail, tail, new_tail);

    sem_post(event_loop->semaphore);
}

bool event_loop_begin(struct event_loop *event_loop)
{
    if (!memory_pool_init(&event_loop->pool, KILOBYTES(512))) return false;

    event_loop->semaphore = sem_open("yabai_event_loop_semaphore", O_CREAT, 0600, 0);
    sem_unlink("yabai_event_loop_semaphore");
    if (event_loop->semaphore == SEM_FAILED) return false;

    event_loop->head = memory_pool_push(&event_loop->pool, sizeof(struct event));
    event_loop->head->next = NULL;
    event_loop->tail = event_loop->head;

    event_loop->is_running = true;
    pthread_create(&event_loop->thread, NULL, &event_loop_run, event_loop);

    return true;
}

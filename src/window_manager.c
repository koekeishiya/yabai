#include "window_manager.h"

extern struct process_manager g_process_manager;

static TABLE_HASH_FUNC(hash_wm)
{
    unsigned long result = *(uint32_t *) key;
    result = (result + 0x7ed55d16) + (result << 12);
    result = (result ^ 0xc761c23c) ^ (result >> 19);
    result = (result + 0x165667b1) + (result << 5);
    result = (result + 0xd3a2646c) ^ (result << 9);
    result = (result + 0xfd7046c5) + (result << 3);
    result = (result ^ 0xb55a4f09) ^ (result >> 16);
    return result;
}

static TABLE_COMPARE_FUNC(compare_wm)
{
    return *(uint32_t *) key_a == *(uint32_t *) key_b;
}

bool window_manager_query_window_title(FILE *rsp)
{
    struct ax_window *window = window_manager_focused_window(&g_window_manager);
    if (!window) return false;

    char *title = window_title(window);
    if (!title) return false;

    fprintf(rsp, "%s\n", title);
    return true;
}

void window_manager_center_mouse(struct window_manager *wm, struct ax_window *window)
{
    if (!wm->enable_mff) return;

    CGPoint cursor;
    SLSGetCurrentCursorLocation(g_connection, &cursor);

    CGRect frame = window_frame(window);
    if (CGRectContainsPoint(frame, cursor)) return;

    uint32_t did = window_display_id(window);
    if (!did) return;

    CGPoint center = {
        frame.origin.x + frame.size.width / 2,
        frame.origin.y + frame.size.height / 2
    };

    CGRect bounds = display_bounds(did);
    if (!CGRectContainsPoint(bounds, center)) return;

    CGWarpMouseCursorPosition(center);
}

bool window_manager_should_manage_window(struct ax_window *window)
{
    return window_is_standard(window) &&
           window_can_move(window) &&
           window_can_resize(window) &&
          !window_is_sticky(window) &&
          !window->is_floating;
}

struct view *window_manager_find_managed_window(struct window_manager *wm, struct ax_window *window)
{
    return table_find(&wm->managed_window, &window->id);
}

void window_manager_remove_managed_window(struct window_manager *wm, struct ax_window *window)
{
    table_remove(&wm->managed_window, &window->id);
}

void window_manager_add_managed_window(struct window_manager *wm, struct ax_window *window, struct view *view)
{
    if (view->type != VIEW_BSP) return;
    table_add(&wm->managed_window, &window->id, view);
}

void window_manager_move_window_relative(struct window_manager *wm, struct ax_window *window, float dx, float dy)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) return;

    CGRect frame = window_frame(window);
    float fx     = frame.origin.x + dx;
    float fy     = frame.origin.y + dy;

    window_manager_move_window(window, fx, fy);
}

void window_manager_resize_window_relative(struct window_manager *wm, struct ax_window *window, int direction, float dx, float dy)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) {
        struct window_node *x_fence = NULL;
        struct window_node *y_fence = NULL;

        struct window_node *node = view_find_window_node(view->root, window->id);
        if (!node) return;

        if (direction & HANDLE_TOP)    x_fence = window_node_fence(node, DIR_NORTH);
        if (direction & HANDLE_BOTTOM) x_fence = window_node_fence(node, DIR_SOUTH);
        if (direction & HANDLE_LEFT)   y_fence = window_node_fence(node, DIR_WEST);
        if (direction & HANDLE_RIGHT)  y_fence = window_node_fence(node, DIR_EAST);
        if (!x_fence && !y_fence)      return;

        if (y_fence) {
            float sr = y_fence->ratio + (float) dx / (float) y_fence->area.w;
            y_fence->ratio = min(1, max(0, sr));
        }

        if (x_fence) {
            float sr = x_fence->ratio + (float) dy / (float) x_fence->area.h;
            x_fence->ratio = min(1, max(0, sr));
        }

        view_update(view);
        view_flush(view);
    } else {
        int x_mod = (direction & HANDLE_LEFT) ? -1 : (direction & HANDLE_RIGHT)  ? 1 : 0;
        int y_mod = (direction & HANDLE_TOP)  ? -1 : (direction & HANDLE_BOTTOM) ? 1 : 0;

        CGRect frame = window_frame(window);
        float fw = max(1, frame.size.width  + dx * x_mod);
        float fh = max(1, frame.size.height + dy * y_mod);
        float fx = (direction & HANDLE_LEFT) ? frame.origin.x + frame.size.width  - fw : frame.origin.x;
        float fy = (direction & HANDLE_TOP)  ? frame.origin.y + frame.size.height - fh : frame.origin.y;

        window_manager_move_window(window, fx, fy);
        window_manager_resize_window(window, fw, fh);
    }
}

void window_manager_move_window(struct ax_window *window, float x, float y)
{
#if 0
    int sockfd;
    char message[255];

    if (socket_connect_in(&sockfd, 5050)) {
        snprintf(message, sizeof(message), "window_move %d %.2f %.2f", window->id, x, y);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
#else
    CGPoint position = CGPointMake(x, y);
    CFTypeRef position_ref = AXValueCreate(kAXValueTypeCGPoint, (void *) &position);
    if (!position_ref) return;

    AXUIElementSetAttributeValue(window->ref, kAXPositionAttribute, position_ref);
    CFRelease(position_ref);
#endif
}

void window_manager_resize_window(struct ax_window *window, float width, float height)
{
    CGSize size = CGSizeMake(width, height);
    CFTypeRef size_ref = AXValueCreate(kAXValueTypeCGSize, (void *) &size);
    if (!size_ref) return;

    AXUIElementSetAttributeValue(window->ref, kAXSizeAttribute, size_ref);
    CFRelease(size_ref);
}

void window_manager_sticky_window(struct ax_window *window, bool sticky)
{
    int sockfd;
    char message[255];

    if (socket_connect_in(&sockfd, 5050)) {
        snprintf(message, sizeof(message), "window_sticky %d %d", window->id, sticky);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);

    if (socket_connect_in(&sockfd, 5050)) {
        snprintf(message, sizeof(message), "window_level %d %d", window->id, sticky ? kCGModalPanelWindowLevelKey : kCGNormalWindowLevelKey);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
}

void window_manager_purify_window(struct ax_window *window)
{
    int sockfd;
    char message[255];

    if (socket_connect_in(&sockfd, 5050)) {
        snprintf(message, sizeof(message), "window_shadow %d 0", window->id);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
}

struct ax_window *window_manager_find_window_at_point(struct window_manager *wm, CGPoint point)
{
    uint32_t window_id = 0;
    CGPoint window_point;
    int window_cid;

    SLSFindWindowByGeometry(g_connection, 0, 1, 0, &point, &window_point, &window_id, &window_cid);
    return window_manager_find_window(wm, window_id);
}

static void send_de_event(ProcessSerialNumber *window_psn, uint32_t window_id)
{
    uint8_t bytes[0xf8] = {
        [0x04] = 0xf8,
        [0x08] = 0x0d,
        [0x8a] = 0x02
    };

    memcpy(bytes + 0x3c, &window_id, sizeof(uint32_t));
    SLPSPostEventRecordTo(window_psn, bytes);
}

static void send_re_event(ProcessSerialNumber *window_psn, uint32_t window_id)
{
    uint8_t bytes[0xf8] = {
        [0x04] = 0xf8,
        [0x08] = 0x0d,
        [0x8a] = 0x01
    };

    memcpy(bytes + 0x3c, &window_id, sizeof(uint32_t));
    SLPSPostEventRecordTo(window_psn, bytes);
}

static void send_pre_event(ProcessSerialNumber *window_psn, uint32_t window_id)
{
    uint8_t bytes[0xf8] = {
        [0x04] = 0xf8,
        [0x08] = 0x0d,
        [0x8a] = 0x09
    };

    memcpy(bytes + 0x3c, &window_id, sizeof(uint32_t));
    SLPSPostEventRecordTo(window_psn, bytes);
}

static void send_post_event(ProcessSerialNumber *window_psn, uint32_t window_id)
{
    uint8_t bytes1[0xf8] = {
        [0x04] = 0xF8,
        [0x08] = 0x01,
        [0x3a] = 0x10
    };

    uint8_t bytes2[0xf8] = {
        [0x04] = 0xF8,
        [0x08] = 0x02,
        [0x3a] = 0x10
    };

    memcpy(bytes1 + 0x3c, &window_id, sizeof(uint32_t));
    memcpy(bytes2 + 0x3c, &window_id, sizeof(uint32_t));
    SLPSPostEventRecordTo(window_psn, bytes1);
    SLPSPostEventRecordTo(window_psn, bytes2);
}

static struct ax_window *window_manager_find_closest_window_for_direction_in_window_list(struct window_manager *wm, struct ax_window *window, int direction, uint32_t *window_list, int window_count)
{
    struct ax_window *best_window = NULL;
    float best_distance = FLT_MAX;

    CGRect source_frame = window_frame(window);
    CGPoint ma = CGPointMake(CGRectGetMidX(source_frame), CGRectGetMidY(source_frame));

    for (int i = 0; i < window_count; ++i) {
        struct ax_window *window = window_manager_find_window(wm, window_list[i]);
        if (!window) continue;

        CGRect frame = window_frame(window);
        CGPoint mb = CGPointMake(CGRectGetMidX(frame), CGRectGetMidY(frame));

        float x_distance = mb.x - ma.x;
        float y_distance = mb.y - ma.y;
        float distance = sqrt((x_distance * x_distance ) + (y_distance * y_distance));

        if (direction == DIR_NORTH) {
            if (y_distance < 0 && distance < best_distance) {
                best_window = window;
                best_distance = distance;
            }
        } else if (direction == DIR_EAST) {
            if (x_distance > 0 && distance < best_distance) {
                best_window = window;
                best_distance = distance;
            }
        } else if (direction == DIR_SOUTH) {
            if (y_distance > 0 && distance < best_distance) {
                best_window = window;
                best_distance = distance;
            }
        } else if (direction == DIR_WEST) {
            if (x_distance < 0 && distance < best_distance) {
                best_window = window;
                best_distance = distance;
            }
        }
    }

    return best_window;
}

struct ax_window *window_manager_find_closest_managed_window_in_direction(struct window_manager *wm, struct ax_window *window, int direction)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (!view) return NULL;

    uint32_t *view_window_list = view_find_window_list(view);
    if (!view_window_list) return NULL;

    struct ax_window *result = window_manager_find_closest_window_for_direction_in_window_list(wm, window, direction, view_window_list, buf_len(view_window_list));
    buf_free(view_window_list);

    return result;
}

struct ax_window *window_manager_find_closest_window_in_direction(struct window_manager *wm, struct ax_window *window, int direction)
{
    int window_count;
    uint32_t *window_list = space_window_list(display_space_id(window_display_id(window)), &window_count);
    if (!window_list) return NULL;

    struct ax_window *result = window_manager_find_closest_window_for_direction_in_window_list(wm, window, direction, window_list, window_count);
    free(window_list);

    return result;
}

void window_manager_focus_window_without_raise(uint32_t window_id)
{
    int window_connection;
    ProcessSerialNumber window_psn;
    pid_t window_pid;

    SLSGetWindowOwner(g_connection, window_id, &window_connection);
    SLSGetConnectionPSN(window_connection, &window_psn);
    SLSConnectionGetPID(window_connection, &window_pid);

    send_pre_event(&window_psn, window_id);
    if (g_window_manager.focused_window_pid != window_pid) {
        _SLPSSetFrontProcessWithOptions(&window_psn, window_id, kCPSUserGenerated);
    } else {
        send_de_event(&window_psn, g_window_manager.focused_window_id);
        send_re_event(&window_psn, window_id);
    }
    send_post_event(&window_psn, window_id);
}

void window_manager_focus_window_with_raise(uint32_t window_id)
{
#if 1
    int sockfd;
    char message[255];

    if (socket_connect_in(&sockfd, 5050)) {
        snprintf(message, sizeof(message), "window_focus %d", window_id);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
#else
    struct ax_window *window = window_manager_find_window(&g_window_manager, window_id);
    if (!window) return;

    AXUIElementPerformAction(window->ref, kAXRaiseAction);
    _SLPSSetFrontProcessWithOptions(&window->application->psn, 0, kCPSNoWindows);
#endif
}

struct ax_window *window_manager_focused_window(struct window_manager *wm)
{
    ProcessSerialNumber psn = {};
    _SLPSGetFrontProcess(&psn);

    struct process *process = process_manager_find_process(&g_process_manager, &psn);
    if (!process) return NULL;

    struct ax_application *application = window_manager_find_application(wm, process->pid);
    if (!application) return NULL;

    uint32_t window_id = application_main_window(application);
    return window_manager_find_window(wm, window_id);
}

struct ax_application *window_manager_focused_application(struct window_manager *wm)
{
    ProcessSerialNumber psn = {};
    _SLPSGetFrontProcess(&psn);

    struct process *process = process_manager_find_process(&g_process_manager, &psn);
    if (!process) return NULL;

    return window_manager_find_application(wm, process->pid);
}

bool window_manager_find_lost_activated_event(struct window_manager *wm, pid_t pid)
{
    return table_find(&wm->application_lost_activated_event, &pid) != NULL;
}

void window_manager_remove_lost_activated_event(struct window_manager *wm, pid_t pid)
{
    table_remove(&wm->application_lost_activated_event, &pid);
}

void window_manager_add_lost_activated_event(struct window_manager *wm, pid_t pid, enum event_type type)
{
    table_add(&wm->application_lost_activated_event, &pid, (void *)(intptr_t) type);
}

bool window_manager_find_lost_focused_event(struct window_manager *wm, uint32_t window_id)
{
    return table_find(&wm->window_lost_focused_event, &window_id) != NULL;
}

void window_manager_remove_lost_focused_event(struct window_manager *wm, uint32_t window_id)
{
    table_remove(&wm->window_lost_focused_event, &window_id);
}

void window_manager_add_lost_focused_event(struct window_manager *wm, uint32_t window_id, enum event_type type)
{
    table_add(&wm->window_lost_focused_event, &window_id, (void *)(intptr_t) type);
}

struct ax_window *window_manager_find_window(struct window_manager *wm, uint32_t window_id)
{
    return table_find(&wm->window, &window_id);
}

void window_manager_remove_window(struct window_manager *wm, uint32_t window_id)
{
    table_remove(&wm->window, &window_id);
}

void window_manager_add_window(struct window_manager *wm, struct ax_window *window)
{
    if (wm->purify_mode != PURIFY_DISABLED) {
        window_manager_purify_window(window);
    }

    table_add(&wm->window, &window->id, window);
}

struct ax_application *window_manager_find_application(struct window_manager *wm, pid_t pid)
{
    return table_find(&wm->application, &pid);
}

void window_manager_remove_application(struct window_manager *wm, pid_t pid)
{
    table_remove(&wm->application, &pid);
}

void window_manager_add_application(struct window_manager *wm, struct ax_application *application)
{
    table_add(&wm->application, &application->pid, application);
}

struct ax_window **window_manager_find_application_windows(struct window_manager *wm, struct ax_application *application, int *count)
{
    int window_count = 0;
    uint32_t window_list[255] = {};

    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct ax_window *window = bucket->value;
                if (window->application == application) {
                    window_list[window_count++] = window->id;
                }
            }

            bucket = bucket->next;
        }
    }

    struct ax_window **result = malloc(sizeof(struct ax_window *) * window_count);
    *count = window_count;

    for (int i = 0; i < window_count; ++i) {
        result[i] = window_manager_find_window(wm, window_list[i]);
    }

    return result;
}

void window_manager_add_application_windows(struct window_manager *wm, struct ax_application *application)
{
    int window_count;
    struct ax_window *window;
    struct ax_window **window_list;

    window_list = application_window_list(application, &window_count);
    if (!window_list) return;

    for (int window_index = 0; window_index < window_count; ++window_index) {
        window = window_list[window_index];

        if (!window->id) {
            goto free_win;
        }

        if (window_manager_find_window(wm, window->id)) {
            goto free_win;
        }

        if (!window_observe(window)) {
            goto uobs_win;
        }

        window_manager_add_window(wm, window);
        goto next;

uobs_win:
        window_unobserve(window);
free_win:
        window_destroy(window);
next:;
    }

    free(window_list);
}

void window_manager_set_window_insertion(struct space_manager *sm, struct ax_window *window, int direction)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (view->type != VIEW_BSP) return;

    struct window_node *node = view_find_window_node(view->root, window->id);
    if (!node) return;

    if (direction == window->border.insert_dir) {
        node->split = SPLIT_NONE;
        node->child = CHILD_NONE;;
        window->border.insert_active = false;
        window->border.insert_dir = 0;
    } else if (direction == DIR_NORTH) {
        node->split = SPLIT_X;
        node->child = CHILD_LEFT;;
        window->border.insert_active = true;
        window->border.insert_dir = direction;
    } else if (direction == DIR_EAST) {
        node->split = SPLIT_Y;
        node->child = CHILD_RIGHT;;
        window->border.insert_active = true;
        window->border.insert_dir = direction;
    } else if (direction == DIR_SOUTH) {
        node->split = SPLIT_X;
        node->child = CHILD_RIGHT;;
        window->border.insert_active = true;
        window->border.insert_dir = direction;
    } else if (direction == DIR_WEST) {
        node->split = SPLIT_Y;
        node->child = CHILD_LEFT;;
        window->border.insert_active = true;
        window->border.insert_dir = direction;
    }

    border_window_refresh(window);
}

void window_manager_warp_window(struct space_manager *sm, struct ax_window *a, struct ax_window *b)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (view->type != VIEW_BSP) return;

    struct window_node *a_node = view_find_window_node(view->root, a->id);
    if (!a_node) return;

    struct window_node *b_node = view_find_window_node(view->root, b->id);
    if (!b_node) return;

    if (a_node->parent == b_node->parent) {
        a_node->window_id = b->id;
        b_node->window_id = a->id;

        window_node_flush(a_node);
        window_node_flush(b_node);
    } else {
        space_manager_untile_window(sm, view, a);
        view->insertion_point = b->id;
        space_manager_tile_window_on_space(sm, a, view->sid);
    }
}

void window_manager_swap_window(struct space_manager *sm, struct window_manager *wm, struct ax_window *a, struct ax_window *b)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (view->type != VIEW_BSP) return;

    struct window_node *a_node = view_find_window_node(view->root, a->id);
    if (!a_node) return;

    struct window_node *b_node = view_find_window_node(view->root, b->id);
    if (!b_node) return;

    a_node->window_id = b->id;
    b_node->window_id = a->id;

    window_node_flush(a_node);
    window_node_flush(b_node);
}

void window_manager_send_window_to_space(struct space_manager *sm, struct window_manager *wm, struct ax_window *window, uint64_t sid)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) {
        space_manager_untile_window(sm, view, window);
        window_manager_remove_managed_window(wm, window);
    }

    space_manager_move_window_to_space(sid, window);
    border_window_refresh(window);

    if (window_manager_should_manage_window(window)) {
        if (space_is_visible(sid)) {
            struct view *view = space_manager_tile_window_on_space(sm, window, sid);
            window_manager_add_managed_window(wm, window, view);
        }
    }
}

void window_manager_apply_grid(struct space_manager *sm, struct window_manager *wm, struct ax_window *window, unsigned r, unsigned c, unsigned x, unsigned y, unsigned w, unsigned h)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) return;

    uint32_t did = window_display_id(window);
    if (!did) return;

    if (x >= c)    x = c - 1;
    if (y >= r)    y = r - 1;
    if (w <= 0)    w = 1;
    if (h <= 0)    h = 1;
    if (w > c - x) w = c - x;
    if (h > r - y) h = r - y;

    uint64_t sid = display_space_id(did);
    int mci = space_manager_mission_control_index(sid);

    CGRect bounds = display_bounds_constrained(did);
    bounds.origin.x    += view_lookup_padding(sm->left_padding, mci);
    bounds.size.width  -= (view_lookup_padding(sm->left_padding, mci) + view_lookup_padding(sm->right_padding, mci));
    bounds.origin.y    += view_lookup_padding(sm->top_padding, mci);
    bounds.size.height -= (view_lookup_padding(sm->top_padding, mci) + view_lookup_padding(sm->bottom_padding, mci));

    float cw = bounds.size.width / c;
    float ch = bounds.size.height / r;

    float fx = bounds.origin.x + bounds.size.width  - cw * (c - x);
    float fy = bounds.origin.y + bounds.size.height - ch * (r - y);
    float fw = cw * w;
    float fh = ch * h;

    window_manager_move_window(window, fx, fy);
    window_manager_resize_window(window, fw, fh);
}

void window_manager_toggle_window_float(struct space_manager *sm, struct window_manager *wm, struct ax_window *window)
{
    if (window->is_floating) {
        window->is_floating = false;
        if (window_manager_should_manage_window(window)) {
            struct view *view = space_manager_tile_window_on_space(sm, window, space_manager_active_space());
            window_manager_add_managed_window(wm, window, view);
        }
    } else {
        struct view *view = window_manager_find_managed_window(wm, window);
        if (view) {
            space_manager_untile_window(sm, view, window);
            window_manager_remove_managed_window(wm, window);
        }
        window->is_floating = true;
    }
}

void window_manager_toggle_window_sticky(struct space_manager *sm, struct window_manager *wm, struct ax_window *window)
{
    if (window_is_sticky(window)) {
        window_manager_sticky_window(window, false);
        if (window_manager_should_manage_window(window)) {
            struct view *view = space_manager_tile_window_on_space(sm, window, space_manager_active_space());
            window_manager_add_managed_window(wm, window, view);
        }
    } else {
        struct view *view = window_manager_find_managed_window(wm, window);
        if (view) {
            space_manager_untile_window(sm, view, window);
            window_manager_remove_managed_window(wm, window);
        }
        window_manager_sticky_window(window, true);
    }
}

void window_manager_toggle_window_native_fullscreen(struct space_manager *sm, struct window_manager *wm, struct ax_window *window)
{
    if (window_is_fullscreen(window)) {
        AXUIElementSetAttributeValue(window->ref, kAXFullscreenAttribute, kCFBooleanFalse);
        return;
    }

    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) {
        space_manager_untile_window(sm, view, window);
        window_manager_remove_managed_window(wm, window);
    }
    AXUIElementSetAttributeValue(window->ref, kAXFullscreenAttribute, kCFBooleanTrue);
}

void window_manager_toggle_window_fullscreen(struct space_manager *sm, struct window_manager *wm, struct ax_window *window)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (!view || view->type != VIEW_BSP) return;

    if (view->root->zoom && view->root->zoom->window_id != window->id) {
        struct ax_window *zoomed_window = window_manager_find_window(wm, view->root->zoom->window_id);
        if (zoomed_window) {
            window_manager_move_window(zoomed_window, view->root->zoom->area.x, view->root->zoom->area.y);
            window_manager_resize_window(zoomed_window, view->root->zoom->area.w, view->root->zoom->area.h);
        }
    }

    if (view->root->zoom && view->root->zoom->window_id == window->id) {
        window_manager_move_window(window, view->root->zoom->area.x, view->root->zoom->area.y);
        window_manager_resize_window(window, view->root->zoom->area.w, view->root->zoom->area.h);
        view->root->zoom = NULL;
    } else {
        window_manager_move_window(window, view->root->area.x, view->root->area.y);
        window_manager_resize_window(window, view->root->area.w, view->root->area.h);
        view->root->zoom = view_find_window_node(view->root, window->id);
    }
}

void window_manager_toggle_window_border(struct ax_window *window)
{
    if (window->border.enabled) {
        border_window_hide(window);
        window->border.enabled = false;
    } else {
        window->border.enabled = true;
        border_window_refresh(window);
    }
}

void window_manager_validate_windows_on_space(struct space_manager *sm, struct window_manager *wm, uint64_t sid)
{
    int window_count;
    uint32_t *window_list = space_window_list(sid, &window_count);
    if (!window_list) return;

    struct view *view = space_manager_find_view(sm, sid);
    uint32_t *view_window_list = view_find_window_list(view);

    for (int i = 0; i < buf_len(view_window_list); ++i) {
        bool found = false;

        for (int j = 0; j < window_count; ++j) {
            if (view_window_list[i]  == window_list[j]) {
                found = true;
                break;
            }
        }

        if (!found) {
            struct ax_window *window = window_manager_find_window(wm, view_window_list[i]);
            if (!window) continue;

            space_manager_untile_window(sm, view, window);
            window_manager_remove_managed_window(wm, window);
        }
    }

    buf_free(view_window_list);
    free(window_list);
}

void window_manager_check_for_windows_on_space(struct space_manager *sm, struct window_manager *wm, uint64_t sid)
{
    int window_count;
    uint32_t *window_list = space_window_list(sid, &window_count);
    if (!window_list) return;

    for (int i = 0; i < window_count; ++i) {
        struct ax_window *window = window_manager_find_window(wm, window_list[i]);
        if (!window || !window_manager_should_manage_window(window)) continue;

        struct view *existing_view = window_manager_find_managed_window(wm, window);
        if (existing_view && existing_view->sid != sid) {
            space_manager_untile_window(sm, existing_view, window);
            window_manager_remove_managed_window(wm, window);
        }

        if (!existing_view || existing_view->sid != sid) {
            struct view *view = space_manager_tile_window_on_space(sm, window, sid);
            window_manager_add_managed_window(wm, window, view);
        }
    }

    free(window_list);
}

void window_manager_handle_display_add_and_remove(struct space_manager *sm, struct window_manager *wm, uint32_t display_id, uint64_t sid)
{
    int space_count;
    uint64_t *space_list = display_space_list(display_id, &space_count);
    if (!space_list) goto out;

    int window_count;
    uint32_t *window_list = space_window_list(space_list[0], &window_count);
    if (!window_list) goto sfree;

    for (int i = 0; i < window_count; ++i) {
        struct ax_window *window = window_manager_find_window(wm, window_list[i]);
        if (!window || !window_manager_should_manage_window(window)) continue;

        struct view *existing_view = window_manager_find_managed_window(wm, window);
        if (existing_view && existing_view->type == VIEW_BSP && existing_view->sid != space_list[0]) {
            space_manager_untile_window(sm, existing_view, window);
            window_manager_remove_managed_window(wm, window);
        }

        if (!existing_view || (existing_view->type == VIEW_BSP && existing_view->sid != space_list[0])) {
            struct view *view = space_manager_tile_window_on_space(sm, window, space_list[0]);
            window_manager_add_managed_window(wm, window, view);
        }
    }

    for (int i = 0; i < space_count; ++i) {
        if (space_list[i] == sid) {
            space_manager_refresh_view(sm, sid);
        } else {
            space_manager_mark_view_invalid(sm, space_list[i]);
        }
    }

    free(window_list);
sfree:
    free(space_list);
out:;
}

void window_manager_init(struct window_manager *wm)
{
    wm->system_element = AXUIElementCreateSystemWide();
    AXUIElementSetMessagingTimeout(wm->system_element, 1.0);

    wm->ffm_mode = FFM_DISABLED;
    wm->purify_mode = PURIFY_ALWAYS;
    wm->enable_mff = true;
    wm->enable_window_border = true;
    wm->window_border_width = 4;
    wm->active_window_border_color = 0xff775759;
    wm->normal_window_border_color = 0xff555555;
    wm->insert_window_border_color = 0xfff57f7f;

    table_init(&wm->application, 150, hash_wm, compare_wm);
    table_init(&wm->window, 150, hash_wm, compare_wm);
    table_init(&wm->managed_window, 150, hash_wm, compare_wm);
    table_init(&wm->window_lost_focused_event, 150, hash_wm, compare_wm);
    table_init(&wm->application_lost_activated_event, 150, hash_wm, compare_wm);
}

void window_manager_begin(struct window_manager *wm)
{
    for (int process_index = 0; process_index < g_process_manager.process.capacity; ++process_index) {
        struct bucket *bucket = g_process_manager.process.buckets[process_index];
        while (bucket) {
            if (bucket->value) {
                struct process *process = bucket->value;
                struct ax_application *application = application_create(process);

                if (application_observe(application)) {
                    window_manager_add_application(wm, application);
                    window_manager_add_application_windows(wm, application);
                } else {
                    application_unobserve(application);
                    application_destroy(application);
                }
            }

            bucket = bucket->next;
        }
    }

    struct ax_window *window = window_manager_focused_window(wm);
    wm->focused_window_id = window->id;
    wm->focused_window_pid = window->application->pid;
    border_window_activate(window);
}

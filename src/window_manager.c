#include "window_manager.h"

extern struct process_manager g_process_manager;
extern struct mouse_state g_mouse_state;
extern char g_sa_socket_file[MAXLEN];

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

void window_manager_query_windows_for_space(FILE *rsp, uint64_t sid)
{
    int window_count;
    uint32_t *window_list = space_window_list(sid, &window_count);
    if (!window_list) return;

    struct window **window_aggregate_list = NULL;
    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_manager_find_window(&g_window_manager, window_list[i]);
        if (window) buf_push(window_aggregate_list, window);
    }

    fprintf(rsp, "[");
    for (int i = 0; i < buf_len(window_aggregate_list); ++i) {
        struct window *window = window_aggregate_list[i];
        window_serialize(rsp, window);
        if (i < buf_len(window_aggregate_list) - 1) fprintf(rsp, ",");
    }
    fprintf(rsp, "]\n");

    buf_free(window_aggregate_list);
    free(window_list);
}

void window_manager_query_windows_for_display(FILE *rsp, uint32_t did)
{
    int space_count;
    uint64_t *space_list = display_space_list(did, &space_count);
    if (!space_list) return;

    struct window **window_aggregate_list = NULL;
    for (int i = 0; i < space_count; ++i) {
        int window_count;
        uint32_t *window_list = space_window_list(space_list[i], &window_count);
        if (!window_list) continue;

        for (int j = 0; j < window_count; ++j) {
            struct window *window = window_manager_find_window(&g_window_manager, window_list[j]);
            if (window) buf_push(window_aggregate_list, window);
        }

        free(window_list);
    }

    fprintf(rsp, "[");
    for (int i = 0; i < buf_len(window_aggregate_list); ++i) {
        struct window *window = window_aggregate_list[i];
        window_serialize(rsp, window);
        if (i < buf_len(window_aggregate_list) - 1) fprintf(rsp, ",");
    }
    fprintf(rsp, "]\n");

    buf_free(window_aggregate_list);
    free(space_list);
}

void window_manager_query_windows_for_displays(FILE *rsp)
{
    uint32_t display_count;
    uint32_t *display_list = display_manager_active_display_list(&display_count);
    if (!display_list) return;

    struct window **window_aggregate_list = NULL;
    for (int i = 0; i < display_count; ++i) {
        int space_count;
        uint64_t *space_list = display_space_list(display_list[i], &space_count);
        if (!space_list) continue;

        for (int j = 0; j < space_count; ++j) {
            int window_count;
            uint32_t *window_list = space_window_list(space_list[j], &window_count);
            if (!window_list) continue;

            for (int k = 0; k < window_count; ++k) {
                struct window *window = window_manager_find_window(&g_window_manager, window_list[k]);
                if (window) buf_push(window_aggregate_list, window);
            }

            free(window_list);
        }

        free(space_list);
    }

    fprintf(rsp, "[");
    for (int i = 0; i < buf_len(window_aggregate_list); ++i) {
        struct window *window = window_aggregate_list[i];
        window_serialize(rsp, window);
        if (i < buf_len(window_aggregate_list) - 1) fprintf(rsp, ",");
    }
    fprintf(rsp, "]\n");

    buf_free(window_aggregate_list);
    free(display_list);
}

static void window_manager_perform_space_assignment_rule(struct space_manager *sm, struct window_manager *wm, struct window *window, struct rule *rule, uint64_t sid)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) {
        space_manager_untile_window(sm, view, window);
        window_manager_remove_managed_window(wm, window->id);
        window_manager_purify_window(wm, window);
    }

    space_manager_move_window_to_space(sid, window);
    if (rule->follow_space || rule->fullscreen == RULE_PROP_ON) {
        space_manager_focus_space(sid);
    }
}

void window_manager_apply_rule_to_window(struct space_manager *sm, struct window_manager *wm, struct window *window, struct rule *rule)
{
    if (regex_match(rule->app_regex_valid,   &rule->app_regex,   window->application->name) == REGEX_MATCH_NO) return;
    if (regex_match(rule->title_regex_valid, &rule->title_regex, window_title(window))      == REGEX_MATCH_NO) return;

    if (!window_is_fullscreen(window) && !space_is_fullscreen(window_space(window))) {
        if (rule->display) {
            uint32_t did = display_manager_arrangement_display_id(rule->display);
            if (did) {
                window_manager_perform_space_assignment_rule(sm, wm, window, rule, display_space_id(did));
            }
        } else if (rule->space) {
            uint64_t sid = space_manager_mission_control_space(rule->space);
            if (sid) {
                window_manager_perform_space_assignment_rule(sm, wm, window, rule, sid);
            }
        }
    }

    if (rule->alpha > 0.0f && rule->alpha <= 1.0f) {
        window_manager_set_window_opacity(wm, window, rule->alpha);
        window->rule_alpha = rule->alpha;
    }

    if (rule->manage == RULE_PROP_ON) {
        window->rule_manage = true;
    } else if (rule->manage == RULE_PROP_OFF) {
        window_manager_make_children_floating(wm, window, true);
        window_manager_make_floating(wm, window->id, true);
        window->is_floating = true;
    }

    if (rule->sticky == RULE_PROP_ON) {
        window_manager_make_sticky(window->id, true);
    } else if (rule->sticky == RULE_PROP_OFF) {
        window_manager_make_sticky(window->id, false);
    }

    if (rule->topmost == RULE_PROP_ON) {
        window_manager_make_topmost(window->id, true);
    } else if (rule->topmost == RULE_PROP_OFF) {
        window_manager_make_topmost(window->id, false);
    }

    if (rule->border == RULE_PROP_ON) {
        border_window_create(window);
    } else if (rule->border == RULE_PROP_OFF) {
        border_window_destroy(window);
    }

    if (rule->fullscreen == RULE_PROP_ON) {
        AXUIElementSetAttributeValue(window->ref, kAXFullscreenAttribute, kCFBooleanTrue);
        window->rule_fullscreen = true;
    }

    if (rule->grid[0] != 0 && rule->grid[1] != 0) {
        window_manager_apply_grid(sm, wm, window, rule->grid[0], rule->grid[1], rule->grid[2], rule->grid[3], rule->grid[4], rule->grid[5]);
    }
}

void window_manager_apply_rules_to_window(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    for (int i = 0; i < buf_len(wm->rules); ++i) {
        window_manager_apply_rule_to_window(sm, wm, window, wm->rules[i]);
    }
}

void window_manager_center_mouse(struct window_manager *wm, struct window *window)
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

bool window_manager_should_manage_window(struct window *window)
{
    if (window->rule_manage) return true;

    return ((window_level_is_standard(window)) &&
            (window_is_standard(window)) &&
            (window_can_move(window)) &&
            (!window_is_sticky(window)) &&
            (!window->is_floating));
}

struct view *window_manager_find_managed_window(struct window_manager *wm, struct window *window)
{
    return table_find(&wm->managed_window, &window->id);
}

void window_manager_remove_managed_window(struct window_manager *wm, uint32_t wid)
{
    table_remove(&wm->managed_window, &wid);
}

void window_manager_add_managed_window(struct window_manager *wm, struct window *window, struct view *view)
{
    if (view->layout != VIEW_BSP) return;
    table_add(&wm->managed_window, &window->id, view);
    window_manager_purify_window(wm, window);
}

void window_manager_move_window_relative(struct window_manager *wm, struct window *window, int type, float dx, float dy)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) return;

    if (type == TYPE_REL) {
        CGRect frame = window_frame(window);
        dx += frame.origin.x;
        dy += frame.origin.y;
    }

    window_manager_move_window(window, dx, dy);
}

void window_manager_resize_window_relative(struct window_manager *wm, struct window *window, int direction, float dx, float dy)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) {
        struct window_node *x_fence = NULL;
        struct window_node *y_fence = NULL;

        struct window_node *node = view_find_window_node(view, window->id);
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
        if (direction == HANDLE_ABS) {
            window_manager_resize_window(window, dx, dy);
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
}

void window_manager_move_window(struct window *window, float x, float y)
{
    CGPoint position = CGPointMake(x, y);
    CFTypeRef position_ref = AXValueCreate(kAXValueTypeCGPoint, (void *) &position);
    if (!position_ref) return;

    AXUIElementSetAttributeValue(window->ref, kAXPositionAttribute, position_ref);
    CFRelease(position_ref);
}

void window_manager_resize_window(struct window *window, float width, float height)
{
    CGSize size = CGSizeMake(width, height);
    CFTypeRef size_ref = AXValueCreate(kAXValueTypeCGSize, (void *) &size);
    if (!size_ref) return;

    AXUIElementSetAttributeValue(window->ref, kAXSizeAttribute, size_ref);
    CFRelease(size_ref);
}

void window_manager_set_window_frame(struct window *window, float x, float y, float width, float height)
{
    window_manager_resize_window(window, width, height);
    window_manager_move_window(window, x, y);
    window_manager_resize_window(window, width, height);
}

void window_manager_set_purify_mode(struct window_manager *wm, enum purify_mode mode)
{
    wm->purify_mode = mode;
    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                window_manager_purify_window(wm, window);
            }

            bucket = bucket->next;
        }
    }
}

void window_manager_set_border_window_enabled(struct window_manager *wm, bool enabled)
{
    wm->enable_window_border = enabled;

    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if ((window_is_standard(window)) || (window_is_dialog(window))) {
                    if (enabled && !window->border.id) {
                        border_window_create(window);

                        if ((!window->application->is_hidden) &&
                            (!window->is_minimized)) {
                            border_window_refresh(window);
                        }

                        if (window->id == wm->focused_window_id) {
                            border_window_activate(window);
                        }
                    } else if (!enabled && window->border.id) {
                        border_window_destroy(window);
                    }
                }
            }

            bucket = bucket->next;
        }
    }
}

void window_manager_set_border_window_width(struct window_manager *wm, int width)
{
    wm->window_border_width = width;
    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if (window->border.id) {
                    window->border.width = width;
                    CGContextSetLineWidth(window->border.context, width);

                    if ((!window->application->is_hidden) &&
                        (!window->is_minimized)) {
                        border_window_refresh(window);
                    }
                }
            }

            bucket = bucket->next;
        }
    }
}

void window_manager_set_border_window_radius(struct window_manager *wm, float radius)
{
    wm->window_border_radius = radius;
    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if (window->border.id) {
                    window->border.radius = radius;

                    if ((!window->application->is_hidden) &&
                        (!window->is_minimized)) {
                        border_window_refresh(window);
                    }
                }
            }

            bucket = bucket->next;
        }
    }
}

void window_manager_set_active_border_window_color(struct window_manager *wm, uint32_t color)
{
    wm->active_window_border_color = color;
    struct window *window = window_manager_focused_window(wm);
    if (window) border_window_activate(window);
}

void window_manager_set_normal_border_window_color(struct window_manager *wm, uint32_t color)
{
    wm->normal_window_border_color = color;
    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if (window->id != wm->focused_window_id) border_window_deactivate(window);
            }

            bucket = bucket->next;
        }
    }
}

void window_manager_set_window_opacity(struct window_manager *wm, struct window *window, float opacity)
{
    if (!wm->enable_window_opacity) return;
    if (window->rule_alpha != 0.0f) return;
    if ((!window_is_standard(window)) && (!window_is_dialog(window))) return;

    int sockfd;
    char message[MAXLEN];

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "window_alpha_fade %d %f %f", window->id, opacity, wm->window_opacity_duration);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
}

void window_manager_set_active_window_opacity(struct window_manager *wm, float opacity)
{
    wm->active_window_opacity = opacity;
    struct window *window = window_manager_focused_window(wm);
    if (window) window_manager_set_window_opacity(wm, window, wm->active_window_opacity);
}

void window_manager_set_normal_window_opacity(struct window_manager *wm, float opacity)
{
    wm->normal_window_opacity = opacity;
    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if (window->id != wm->focused_window_id) window_manager_set_window_opacity(wm, window, wm->normal_window_opacity);
            }

            bucket = bucket->next;
        }
    }
}

void window_manager_make_topmost(uint32_t wid, bool topmost)
{
    int sockfd;
    char message[MAXLEN];

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "window_level %d %d", wid, topmost ? kCGFloatingWindowLevelKey : kCGNormalWindowLevelKey);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
}

void window_manager_make_floating(struct window_manager *wm, uint32_t wid, bool floating)
{
    if (!wm->enable_window_topmost) return;
    window_manager_make_topmost(wid, floating);
}

void window_manager_make_sticky(uint32_t wid, bool sticky)
{
    int sockfd;
    char message[MAXLEN];

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "window_sticky %d %d", wid, sticky);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
}

void window_manager_purify_window(struct window_manager *wm, struct window *window)
{
    int value;
    int sockfd;
    char message[MAXLEN];

    if (wm->purify_mode == PURIFY_DISABLED) {
        value = 1;
    } else if (wm->purify_mode == PURIFY_MANAGED) {
        value = window_manager_find_managed_window(wm, window) ? 0 : 1;
    } else if (wm->purify_mode == PURIFY_ALWAYS) {
        value = 0;
    }

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "window_shadow %d %d", window->id, value);
        socket_write(sockfd, message);
        socket_wait(sockfd);
        window->has_shadow = value;
    }
    socket_close(sockfd);
}

static struct window *window_manager_find_window_on_space_by_rank(struct window_manager *wm, uint64_t sid, int rank)
{
    int count;
    uint32_t *window_list = space_window_list(sid, &count);
    if (!window_list) return NULL;

    struct window *result = NULL;
    for (int i = 0, j = 0; i < count; ++i) {
        struct window *window = window_manager_find_window(wm, window_list[i]);
        if (!window) continue;

        if (++j == rank) {
            result = window;
            break;
        }
    }

    free(window_list);
    return result;
}

struct window *window_manager_find_window_at_point_filtering_window(struct window_manager *wm, CGPoint point, uint32_t filter_wid)
{
    uint32_t window_id = 0;
    CGPoint window_point;
    int window_cid;

    SLSFindWindowByGeometry(g_connection, filter_wid, 0xffffffff, 0, &point, &window_point, &window_id, &window_cid);
    return window_manager_find_window(wm, window_id);
}

struct window *window_manager_find_window_at_point(struct window_manager *wm, CGPoint point)
{
    uint32_t window_id = 0;
    CGPoint window_point;
    int window_cid;

    SLSFindWindowByGeometry(g_connection, 0, 1, 0, &point, &window_point, &window_id, &window_cid);
    return window_manager_find_window(wm, window_id);
}

struct window *window_manager_find_window_below_cursor(struct window_manager *wm)
{
    CGPoint cursor;
    SLSGetCurrentCursorLocation(g_connection, &cursor);
    return window_manager_find_window_at_point(wm, cursor);
}

static struct window *window_manager_find_closest_window_for_direction_in_window_list(struct window_manager *wm, struct window *source, int direction, uint32_t *window_list, int window_count)
{
    CGRect source_frame = window_frame(source);
    struct window *best_window = NULL;
    uint32_t best_distance = UINT32_MAX;

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_manager_find_window(wm, window_list[i]);
        if (!window || !window_is_standard(window) || window == source) continue;

        CGRect frame = window_frame(window);
        if (!rect_is_in_direction(source_frame, frame, direction))      continue;

        uint32_t distance = rect_distance(source_frame, frame, direction);
        if (distance < best_distance) {
            best_window = window;
            best_distance = distance;
        }
    }

    return best_window;
}

struct window *window_manager_find_closest_managed_window_in_direction(struct window_manager *wm, struct window *window, int direction)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (!view) return NULL;

    uint32_t *view_window_list = view_find_window_list(view);
    if (!view_window_list) return NULL;

    struct window *result = window_manager_find_closest_window_for_direction_in_window_list(wm, window, direction, view_window_list, buf_len(view_window_list));
    buf_free(view_window_list);

    return result;
}

struct window *window_manager_find_closest_window_in_direction(struct window_manager *wm, struct window *window, int direction)
{
    int window_count;
    uint32_t *window_list = space_window_list(display_space_id(window_display_id(window)), &window_count);
    if (!window_list) return NULL;

    struct window *result = window_manager_find_closest_window_for_direction_in_window_list(wm, window, direction, window_list, window_count);
    free(window_list);

    return result;
}

struct window *window_manager_find_prev_managed_window(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

    struct window_node *prev = window_node_find_prev_leaf(node);
    if (!prev) return NULL;

    return window_manager_find_window(wm, prev->window_id);
}

struct window *window_manager_find_next_managed_window(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

    struct window_node *prev = window_node_find_next_leaf(node);
    if (!prev) return NULL;

    return window_manager_find_window(wm, prev->window_id);
}

struct window *window_manager_find_first_managed_window(struct space_manager *sm, struct window_manager *wm)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *first = window_node_find_first_leaf(view->root);
    if (!first) return NULL;

    return window_manager_find_window(wm, first->window_id);
}

struct window *window_manager_find_last_managed_window(struct space_manager *sm, struct window_manager *wm)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *last = window_node_find_last_leaf(view->root);
    if (!last) return NULL;

    return window_manager_find_window(wm, last->window_id);
}

struct window *window_manager_find_recent_managed_window(struct space_manager *sm, struct window_manager *wm)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, wm->last_window_id);
    if (!node) return NULL;

    return window_manager_find_window(wm, node->window_id);
}

struct window *window_manager_find_largest_managed_window(struct space_manager *sm, struct window_manager *wm)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    uint32_t best_id   = 0;
    uint32_t best_area = 0;

    for (struct window_node *node = window_node_find_first_leaf(view->root); node != NULL; node = window_node_find_next_leaf(node)) {
        uint32_t area = node->area.w * node->area.h;
        if (area > best_area) {
            best_id   = node->window_id;
            best_area = area;
        }
    }

    return best_id ? window_manager_find_window(wm, best_id) : NULL;
}

struct window *window_manager_find_smallest_managed_window(struct space_manager *sm, struct window_manager *wm)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    uint32_t best_id   = 0;
    uint32_t best_area = UINT32_MAX;

    for (struct window_node *node = window_node_find_first_leaf(view->root); node != NULL; node = window_node_find_next_leaf(node)) {
        uint32_t area = node->area.w * node->area.h;
        if (area <= best_area) {
            best_id   = node->window_id;
            best_area = area;
        }
    }

    return best_id ? window_manager_find_window(wm, best_id) : NULL;
}

static void window_manager_defer_window_raise(ProcessSerialNumber *window_psn, uint32_t window_id)
{
    uint8_t bytes[0xf8] = {
        [0x04] = 0xf8,
        [0x08] = 0x0d,
        [0x8a] = 0x09
    };

    memcpy(bytes + 0x3c, &window_id, sizeof(uint32_t));
    SLPSPostEventRecordTo(window_psn, bytes);
}

static void window_manager_make_key_window(ProcessSerialNumber *window_psn, uint32_t window_id)
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
    memset(bytes1 + 0x20, 0xFF, 0x10);
    memcpy(bytes2 + 0x3c, &window_id, sizeof(uint32_t));
    memset(bytes2 + 0x20, 0xFF, 0x10);
    SLPSPostEventRecordTo(window_psn, bytes1);
    SLPSPostEventRecordTo(window_psn, bytes2);
}

static void window_manager_deactivate_window(ProcessSerialNumber *window_psn, uint32_t window_id)
{
    uint8_t bytes[0xf8] = {
        [0x04] = 0xf8,
        [0x08] = 0x0d,
        [0x8a] = 0x02
    };

    memcpy(bytes + 0x3c, &window_id, sizeof(uint32_t));
    SLPSPostEventRecordTo(window_psn, bytes);
}

static void window_manager_activate_window(ProcessSerialNumber *window_psn, uint32_t window_id)
{
    uint8_t bytes[0xf8] = {
        [0x04] = 0xf8,
        [0x08] = 0x0d,
        [0x8a] = 0x01
    };

    memcpy(bytes + 0x3c, &window_id, sizeof(uint32_t));
    SLPSPostEventRecordTo(window_psn, bytes);
}

void window_manager_focus_window_without_raise(ProcessSerialNumber *window_psn, uint32_t window_id)
{
    window_manager_defer_window_raise(window_psn, window_id);

    if (psn_equals(window_psn, &g_window_manager.focused_window_psn)) {
        window_manager_deactivate_window(&g_window_manager.focused_window_psn, g_window_manager.focused_window_id);

        // @hack
        // Artificially delay the activation by 1ms. This is necessary
        // because some applications appear to be confused if both of
        // the events appear instantaneously.
        usleep(10000);

        window_manager_activate_window(window_psn, window_id);
    }

    _SLPSSetFrontProcessWithOptions(window_psn, window_id, kCPSUserGenerated);
    window_manager_make_key_window(window_psn, window_id);
}

void window_manager_focus_window_with_raise(ProcessSerialNumber *window_psn, uint32_t window_id, AXUIElementRef window_ref)
{
#if 1
    _SLPSSetFrontProcessWithOptions(window_psn, window_id, kCPSUserGenerated);
    window_manager_make_key_window(window_psn, window_id);
    AXUIElementPerformAction(window_ref, kAXRaiseAction);
#else
    int sockfd;
    char message[MAXLEN];

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "window_focus %d", window_id);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
#endif
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
struct window *window_manager_focused_window(struct window_manager *wm)
{
    ProcessSerialNumber psn = {};
    _SLPSGetFrontProcess(&psn);

    pid_t pid;
    GetProcessPID(&psn, &pid);

    struct application *application = window_manager_find_application(wm, pid);
    if (!application) return NULL;

    uint32_t window_id = application_focused_window(application);
    return window_manager_find_window(wm, window_id);
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
struct application *window_manager_focused_application(struct window_manager *wm)
{
    ProcessSerialNumber psn = {};
    _SLPSGetFrontProcess(&psn);

    pid_t pid;
    GetProcessPID(&psn, &pid);

    return window_manager_find_application(wm, pid);
}
#pragma clang diagnostic pop

bool window_manager_find_lost_front_switched_event(struct window_manager *wm, pid_t pid)
{
    return table_find(&wm->application_lost_front_switched_event, &pid) != NULL;
}

void window_manager_remove_lost_front_switched_event(struct window_manager *wm, pid_t pid)
{
    table_remove(&wm->application_lost_front_switched_event, &pid);
}

void window_manager_add_lost_front_switched_event(struct window_manager *wm, pid_t pid)
{
    table_add(&wm->application_lost_front_switched_event, &pid, (void *)(intptr_t) 1);
}

bool window_manager_find_lost_focused_event(struct window_manager *wm, uint32_t window_id)
{
    return table_find(&wm->window_lost_focused_event, &window_id) != NULL;
}

void window_manager_remove_lost_focused_event(struct window_manager *wm, uint32_t window_id)
{
    table_remove(&wm->window_lost_focused_event, &window_id);
}

void window_manager_add_lost_focused_event(struct window_manager *wm, uint32_t window_id)
{
    table_add(&wm->window_lost_focused_event, &window_id, (void *)(intptr_t) 1);
}

struct window *window_manager_find_window(struct window_manager *wm, uint32_t window_id)
{
    return table_find(&wm->window, &window_id);
}

void window_manager_remove_window(struct window_manager *wm, uint32_t window_id)
{
    table_remove(&wm->window, &window_id);
}

void window_manager_add_window(struct window_manager *wm, struct window *window)
{
    table_add(&wm->window, &window->id, window);
}

struct application *window_manager_find_application(struct window_manager *wm, pid_t pid)
{
    return table_find(&wm->application, &pid);
}

void window_manager_remove_application(struct window_manager *wm, pid_t pid)
{
    table_remove(&wm->application, &pid);
}

void window_manager_add_application(struct window_manager *wm, struct application *application)
{
    table_add(&wm->application, &application->pid, application);
}

struct window **window_manager_find_application_windows(struct window_manager *wm, struct application *application, int *count)
{
    int window_count = 0;
    uint32_t window_list[MAXLEN];

    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if (window->application == application) {
                    window_list[window_count++] = window->id;
                }
            }

            bucket = bucket->next;
        }
    }

    if (!window_count) return NULL;

    struct window **result = malloc(sizeof(struct window *) * window_count);
    *count = window_count;

    for (int i = 0; i < window_count; ++i) {
        result[i] = window_manager_find_window(wm, window_list[i]);
    }

    return result;
}

void window_manager_add_application_windows(struct space_manager *sm, struct window_manager *wm, struct application *application)
{
    int window_count;
    struct window **window_list = application_window_list(application, &window_count);
    if (!window_list) return;

    for (int window_index = 0; window_index < window_count; ++window_index) {
        struct window *window = window_list[window_index];
        if (!window) continue;

        if (!window->id || window_manager_find_window(wm, window->id)) {
            window_destroy(window);
            continue;
        }

        if (window_is_popover(window) || window_is_unknown(window)) {
            debug("%s: ignoring window %s %d\n", __FUNCTION__, window->application->name, window->id);
            window_manager_make_children_floating(wm, window, true);
            window_manager_make_floating(wm, window->id, true);
            window_destroy(window);
            continue;
        }

        window_manager_apply_rules_to_window(sm, wm, window);
        window_manager_set_window_opacity(wm, window, wm->normal_window_opacity);
        window_manager_purify_window(wm, window);

        if (window_observe(window)) {
            window_manager_add_window(wm, window);

            if ((!application->is_hidden) && (!window->is_minimized) && (!window->is_fullscreen) && (!window->rule_manage)) {
                if (window->rule_fullscreen) {
                    window->rule_fullscreen = false;
                } else if ((!window_level_is_standard(window)) ||
                           (!window_is_standard(window)) ||
                           (!window_can_move(window)) ||
                           (window_is_sticky(window)) ||
                           (window_is_undersized(window))) {
                    window_manager_make_children_floating(wm, window, true);
                    window_manager_make_floating(wm, window->id, true);
                    window->is_floating = true;
                }
            }
        } else {
            window_manager_make_children_floating(wm, window, true);
            window_manager_make_floating(wm, window->id, true);
            window_unobserve(window);
            window_destroy(window);
        }
    }

    free(window_list);
}

void window_manager_set_window_insertion(struct space_manager *sm, struct window_manager *wm, struct window *window, int direction)
{
    uint64_t sid = window_space(window);
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return;

    if (view->insertion_point && view->insertion_point != window->id) {
        struct window_node *insert_node = view_find_window_node(view, view->insertion_point);
        if (insert_node) {
            insert_node->split = SPLIT_NONE;
            insert_node->child = CHILD_NONE;
        }

        struct window *insert_window = window_manager_find_window(wm, view->insertion_point);
        if (insert_window) {
            insert_window->border.insert_active = false;
            insert_window->border.insert_dir = 0;
            border_window_refresh(insert_window);
        }
    }

    if (direction == window->border.insert_dir) {
        node->split = SPLIT_NONE;
        node->child = CHILD_NONE;
        window->border.insert_active = false;
        window->border.insert_dir = 0;
        view->insertion_point = 0;
    } else if (direction == DIR_NORTH) {
        node->split = SPLIT_X;
        node->child = CHILD_FIRST;
        window->border.insert_active = true;
        window->border.insert_dir = direction;
        view->insertion_point = node->window_id;
    } else if (direction == DIR_EAST) {
        node->split = SPLIT_Y;
        node->child = CHILD_SECOND;
        window->border.insert_active = true;
        window->border.insert_dir = direction;
        view->insertion_point = node->window_id;
    } else if (direction == DIR_SOUTH) {
        node->split = SPLIT_X;
        node->child = CHILD_SECOND;
        window->border.insert_active = true;
        window->border.insert_dir = direction;
        view->insertion_point = node->window_id;
    } else if (direction == DIR_WEST) {
        node->split = SPLIT_Y;
        node->child = CHILD_FIRST;
        window->border.insert_active = true;
        window->border.insert_dir = direction;
        view->insertion_point = node->window_id;
    }

    border_window_refresh(window);
}

void window_manager_warp_window(struct space_manager *sm, struct window_manager *wm, struct window *a, struct window *b)
{
    uint64_t a_sid = window_space(a);
    struct view *a_view = space_manager_find_view(sm, a_sid);
    if (a_view->layout != VIEW_BSP) return;

    uint64_t b_sid = window_space(b);
    struct view *b_view = space_manager_find_view(sm, b_sid);
    if (b_view->layout != VIEW_BSP) return;

    struct window_node *a_node = view_find_window_node(a_view, a->id);
    if (!a_node) return;

    struct window_node *b_node = view_find_window_node(b_view, b->id);
    if (!b_node) return;

    if (a_node->parent == b_node->parent) {
        if (b_view->insertion_point == b_node->window_id) {
            b_node->parent->split = b_node->split;
            b_node->parent->child = b_node->child;
            space_manager_untile_window(sm, a_view, a);
            window_manager_remove_managed_window(wm, a->id);
            window_manager_add_managed_window(wm, a, b_view);
            space_manager_tile_window_on_space_with_insertion_point(sm, a, b_view->sid, b->id);
        } else {
            a_node->window_id = b->id;
            a_node->zoom = NULL;

            b_node->window_id = a->id;
            b_node->zoom = NULL;

            window_node_flush(a_node);
            window_node_flush(b_node);
        }
    } else {
        space_manager_untile_window(sm, a_view, a);

        if (a_view->sid != b_view->sid) {
            window_manager_remove_managed_window(wm, a->id);
            window_manager_add_managed_window(wm, a, b_view);
            space_manager_move_window_to_space(b_view->sid, a);

            if (wm->focused_window_id == a->id) {
                struct window *next = window_manager_find_window_on_space_by_rank(wm, a_view->sid, 1);
                if (next) {
                    window_manager_focus_window_with_raise(&next->application->psn, next->id, next->ref);
                } else {
                    g_mouse_state.ffm_window_id = 0;
                    g_window_manager.last_window_id = g_window_manager.focused_window_id;
                    g_window_manager.focused_window_id = 0;
                    g_window_manager.focused_window_psn = g_process_manager.finder_psn;
                    _SLPSSetFrontProcessWithOptions(&g_process_manager.finder_psn, 0, kCPSNoWindows);
                }
            }
        }

        space_manager_tile_window_on_space_with_insertion_point(sm, a, b_view->sid, b->id);
    }
}

void window_manager_swap_window(struct space_manager *sm, struct window_manager *wm, struct window *a, struct window *b)
{
    uint64_t a_sid = window_space(a);
    struct view *a_view = space_manager_find_view(sm, a_sid);
    if (a_view->layout != VIEW_BSP) return;

    uint64_t b_sid = window_space(b);
    struct view *b_view = space_manager_find_view(sm, b_sid);
    if (b_view->layout != VIEW_BSP) return;

    struct window_node *a_node = view_find_window_node(a_view, a->id);
    if (!a_node) return;

    struct window_node *b_node = view_find_window_node(b_view, b->id);
    if (!b_node) return;

    a_node->window_id = b->id;
    a_node->zoom = NULL;

    b_node->window_id = a->id;
    b_node->zoom = NULL;

    if (a_view->sid != b_view->sid) {
        window_manager_remove_managed_window(wm, a->id);
        window_manager_add_managed_window(wm, a, b_view);
        window_manager_remove_managed_window(wm, b->id);
        window_manager_add_managed_window(wm, b, a_view);
        space_manager_move_window_to_space(b_view->sid, a);
        space_manager_move_window_to_space(a_view->sid, b);

        if (wm->focused_window_id == a->id) {
            window_manager_focus_window_with_raise(&b->application->psn, b->id, b->ref);
        } else if (wm->focused_window_id == b->id) {
            window_manager_focus_window_with_raise(&a->application->psn, a->id, a->ref);
        }
    }

    window_node_flush(a_node);
    window_node_flush(b_node);
}

bool window_manager_close_window(struct window *window)
{
    CFTypeRef button = NULL;
    AXUIElementCopyAttributeValue(window->ref, kAXCloseButtonAttribute, &button);
    if (!button) return false;

    AXUIElementPerformAction(button, kAXPressAction);
    CFRelease(button);

    return true;
}

void window_manager_send_window_to_space(struct space_manager *sm, struct window_manager *wm, struct window *window, uint64_t dst_sid)
{
    uint64_t src_sid = window_space(window);
    if (src_sid == dst_sid) return;

    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) {
        space_manager_untile_window(sm, view, window);
        window_manager_remove_managed_window(wm, window->id);
        window_manager_purify_window(wm, window);
    }

    space_manager_move_window_to_space(dst_sid, window);

    if (window_manager_should_manage_window(window)) {
        struct view *view = space_manager_tile_window_on_space(sm, window, dst_sid);
        window_manager_add_managed_window(wm, window, view);
    }

    if (space_is_visible(src_sid) && wm->focused_window_id == window->id) {
        struct window *next = window_manager_find_window_on_space_by_rank(wm, src_sid, 1);
        if (next) {
            window_manager_focus_window_with_raise(&next->application->psn, next->id, next->ref);
        } else {
            g_mouse_state.ffm_window_id = 0;
            g_window_manager.last_window_id = g_window_manager.focused_window_id;
            g_window_manager.focused_window_id = 0;
            g_window_manager.focused_window_psn = g_process_manager.finder_psn;
            _SLPSSetFrontProcessWithOptions(&g_process_manager.finder_psn, 0, kCPSNoWindows);
        }
    }
}

void window_manager_apply_grid(struct space_manager *sm, struct window_manager *wm, struct window *window, unsigned r, unsigned c, unsigned x, unsigned y, unsigned w, unsigned h)
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
    struct view *dview = space_manager_find_view(sm, sid);

    CGRect bounds = display_bounds_constrained(did);
    if (dview && dview->enable_padding) {
        bounds.origin.x    += dview->left_padding;
        bounds.size.width  -= (dview->left_padding + dview->right_padding);
        bounds.origin.y    += dview->top_padding;
        bounds.size.height -= (dview->top_padding + dview->bottom_padding);
    }

    float offset = window_node_border_window_offset(window);
    float cw = bounds.size.width / c;
    float ch = bounds.size.height / r;
    float fx = bounds.origin.x + bounds.size.width  - cw * (c - x) + offset;
    float fy = bounds.origin.y + bounds.size.height - ch * (r - y) + offset;
    float fw = cw * w - 2 * offset;
    float fh = ch * h - 2 * offset;

    window_manager_move_window(window, fx, fy);
    window_manager_resize_window(window, fw, fh);
}

void window_manager_make_children_floating(struct window_manager *wm, struct window *window, bool floating)
{
    if (!wm->enable_window_topmost) return;

    uint64_t sid = window_space(window);
    if (!sid) sid = space_manager_active_space();

    int count;
    uint32_t *window_list = space_window_list_for_connection(sid, window->connection, &count);
    if (!window_list) return;

    CFArrayRef window_list_ref = cfarray_of_cfnumbers(window_list, sizeof(uint32_t), count, kCFNumberSInt32Type);
    CFTypeRef query = SLSWindowQueryWindows(g_connection, window_list_ref, count);
    CFTypeRef iterator = SLSWindowQueryResultCopyWindows(query);

    while (SLSWindowIteratorAdvance(iterator)) {
        uint32_t parent_wid = SLSWindowIteratorGetParentID(iterator);
        uint32_t wid = SLSWindowIteratorGetWindowID(iterator);
        if (parent_wid == window->id) window_manager_make_floating(wm, wid, floating);
    }

    CFRelease(query);
    CFRelease(iterator);
    CFRelease(window_list_ref);
    free(window_list);
}

void window_manager_toggle_window_topmost(struct window *window)
{
    bool is_topmost = window_is_topmost(window);
    window_manager_make_topmost(window->id, !is_topmost);
}

void window_manager_toggle_window_float(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    if (window->is_floating) {
        window->is_floating = false;
        window_manager_make_children_floating(wm, window, false);
        window_manager_make_floating(wm, window->id, false);
        if (window_manager_should_manage_window(window)) {
            struct view *view = space_manager_tile_window_on_space(sm, window, space_manager_active_space());
            window_manager_add_managed_window(wm, window, view);
        }
    } else {
        struct view *view = window_manager_find_managed_window(wm, window);
        if (view) {
            space_manager_untile_window(sm, view, window);
            window_manager_remove_managed_window(wm, window->id);
            window_manager_purify_window(wm, window);
        }
        window_manager_make_children_floating(wm, window, true);
        window_manager_make_floating(wm, window->id, true);
        window->is_floating = true;
    }
}

void window_manager_toggle_window_sticky(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    if (window_is_sticky(window)) {
        window_manager_make_sticky(window->id, false);
        if (window_manager_should_manage_window(window)) {
            struct view *view = space_manager_tile_window_on_space(sm, window, space_manager_active_space());
            window_manager_add_managed_window(wm, window, view);
        }
    } else {
        struct view *view = window_manager_find_managed_window(wm, window);
        if (view) {
            space_manager_untile_window(sm, view, window);
            window_manager_remove_managed_window(wm, window->id);
            window_manager_purify_window(wm, window);
        }
        window_manager_make_sticky(window->id, true);
    }
}

void window_manager_toggle_window_shadow(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    int sockfd;
    char message[MAXLEN];
    bool shadow = !window->has_shadow;

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "window_shadow %d %d", window->id, shadow);
        socket_write(sockfd, message);
        socket_wait(sockfd);
        window->has_shadow = shadow;
    }
    socket_close(sockfd);
}

void window_manager_toggle_window_native_fullscreen(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    uint32_t sid = window_space(window);

    if (!window_is_fullscreen(window)) {
        window_manager_focus_window_with_raise(&window->application->psn, window->id, window->ref);
        while (sid != space_manager_active_space()) { /* maybe spin lock */ }
        AXUIElementSetAttributeValue(window->ref, kAXFullscreenAttribute, kCFBooleanTrue);
        while (sid == space_manager_active_space()) { /* maybe spin lock */ }
    } else {
        AXUIElementSetAttributeValue(window->ref, kAXFullscreenAttribute, kCFBooleanFalse);
        while (sid == space_manager_active_space()) { /* maybe spin lock */ }
    }
}

void window_manager_toggle_window_parent(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (!view || view->layout != VIEW_BSP) return;

    struct window_node *node = view_find_window_node(view, window->id);
    if (node->zoom) {
        float offset = window_node_border_window_offset(window);
        window_manager_move_window(window, node->area.x + offset, node->area.y + offset);
        window_manager_resize_window(window, node->area.w - 2*offset, node->area.h - 2*offset);
        node->zoom = NULL;
    } else if (node->parent) {
        float offset = window_node_border_window_offset(window);
        window_manager_move_window(window, node->parent->area.x + offset, node->parent->area.y + offset);
        window_manager_resize_window(window, node->parent->area.w - 2*offset, node->parent->area.h - 2*offset);
        node->zoom = node->parent;
    }
}

void window_manager_toggle_window_fullscreen(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (!view || view->layout != VIEW_BSP) return;

    struct window_node *node = view_find_window_node(view, window->id);
    if (node->zoom) {
        float offset = window_node_border_window_offset(window);
        window_manager_move_window(window, node->area.x + offset, node->area.y + offset);
        window_manager_resize_window(window, node->area.w - 2*offset, node->area.h - 2*offset);
        node->zoom = NULL;
    } else {
        float offset = window_node_border_window_offset(window);
        window_manager_move_window(window, view->root->area.x + offset, view->root->area.y + offset);
        window_manager_resize_window(window, view->root->area.w - 2*offset, view->root->area.h - 2*offset);
        node->zoom = view->root;
    }
}

void window_manager_toggle_window_border(struct window_manager *wm, struct window *window)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    struct window_node *node = view ? view_find_window_node(view, window->id) : NULL;

    if (window->border.enabled) {
        border_window_hide(window);
        window->border.enabled = false;
    } else {
        window->border.enabled = true;
        border_window_refresh(window);
    }

    if (node) window_node_flush(node);
}

void window_manager_toggle_window_expose(struct window_manager *wm, struct window *window)
{
    window_manager_focus_window_with_raise(&window->application->psn, window->id, window->ref);
    CoreDockSendNotification(CFSTR("com.apple.expose.front.awake"), 0);
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
            if (view_window_list[i] == window_list[j]) {
                found = true;
                break;
            }
        }

        if (!found) {
            struct window *window = window_manager_find_window(wm, view_window_list[i]);
            if (!window) continue;

            space_manager_untile_window(sm, view, window);
            window_manager_remove_managed_window(wm, window->id);
            window_manager_purify_window(wm, window);
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
        struct window *window = window_manager_find_window(wm, window_list[i]);
        if (!window || !window_manager_should_manage_window(window)) continue;
        if (window->is_minimized || window->application->is_hidden)  continue;

        struct view *existing_view = window_manager_find_managed_window(wm, window);
        if (existing_view && existing_view->sid != sid) {
            space_manager_untile_window(sm, existing_view, window);
            window_manager_remove_managed_window(wm, window->id);
            window_manager_purify_window(wm, window);
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
        struct window *window = window_manager_find_window(wm, window_list[i]);
        if (!window || !window_manager_should_manage_window(window)) continue;
        if (window->is_minimized || window->application->is_hidden)  continue;

        struct view *existing_view = window_manager_find_managed_window(wm, window);
        if (existing_view && existing_view->layout == VIEW_BSP && existing_view->sid != space_list[0]) {
            space_manager_untile_window(sm, existing_view, window);
            window_manager_remove_managed_window(wm, window->id);
            window_manager_purify_window(wm, window);
        }

        if (!existing_view || (existing_view->layout == VIEW_BSP && existing_view->sid != space_list[0])) {
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
    wm->purify_mode = PURIFY_DISABLED;
    wm->enable_mff = false;
    wm->enable_window_opacity = false;
    wm->enable_window_topmost = false;
    wm->enable_window_border = false;
    wm->window_border_placement = BORDER_PLACEMENT_INSET;
    wm->window_border_width = 4;
    wm->window_border_radius = -1;
    wm->active_window_border_topmost = false;
    wm->active_window_border_color = 0xff775759;
    wm->normal_window_border_color = 0xff555555;
    wm->insert_window_border_color = 0xfff57f7f;
    wm->active_window_opacity = 1.0f;
    wm->normal_window_opacity = 1.0f;
    wm->window_opacity_duration = 0.2f;

    table_init(&wm->application, 150, hash_wm, compare_wm);
    table_init(&wm->window, 150, hash_wm, compare_wm);
    table_init(&wm->managed_window, 150, hash_wm, compare_wm);
    table_init(&wm->window_lost_focused_event, 150, hash_wm, compare_wm);
    table_init(&wm->application_lost_front_switched_event, 150, hash_wm, compare_wm);
}

void window_manager_begin(struct space_manager *sm, struct window_manager *wm)
{
    for (int process_index = 0; process_index < g_process_manager.process.capacity; ++process_index) {
        struct bucket *bucket = g_process_manager.process.buckets[process_index];
        while (bucket) {
            if (bucket->value) {
                struct process *process = bucket->value;
                struct application *application = application_create(process);

                if (application_observe(application)) {
                    window_manager_add_application(wm, application);
                    window_manager_add_application_windows(sm, wm, application);
                } else {
                    application_unobserve(application);
                    application_destroy(application);
                }
            }

            bucket = bucket->next;
        }
    }

    struct window *window = window_manager_focused_window(wm);
    if (window) {
        wm->last_window_id = window->id;
        wm->focused_window_id = window->id;
        wm->focused_window_psn = window->application->psn;
        border_window_activate(window);
        window_manager_set_window_opacity(wm, window, wm->active_window_opacity);
    }
}

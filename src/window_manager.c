extern struct event_loop g_event_loop;
extern struct process_manager g_process_manager;
extern struct mouse_state g_mouse_state;

static TABLE_HASH_FUNC(hash_wm)
{
    return *(uint32_t *) key;
}

static TABLE_COMPARE_FUNC(compare_wm)
{
    return *(uint32_t *) key_a == *(uint32_t *) key_b;
}

void window_manager_query_window_rules(FILE *rsp)
{
    fprintf(rsp, "[");
    for (int i = 0; i < buf_len(g_window_manager.rules); ++i) {
        struct rule *rule = &g_window_manager.rules[i];
        rule_serialize(rsp, rule, i);
        if (i < buf_len(g_window_manager.rules) - 1) fprintf(rsp, ",");
    }
    fprintf(rsp, "]\n");
}

static struct window **window_manager_find_windows_for_spaces(uint64_t *space_list, int space_count, int *window_aggregate_count)
{
    int window_count = 0;
    uint32_t *window_list = NULL;

    for (int i = 0; i < space_count; ++i) {
        int count;
        uint32_t *list = space_window_list(space_list[i], &count, true);
        if (!list) continue;

        //
        // NOTE(koekeishiya): space_window_list(..) uses a linear allocator,
        // and so we only need to track the beginning of the first list along
        // with the total number of windows that have been allocated.
        //

        if (!window_list) window_list = list;
        window_count += count;
    }

    *window_aggregate_count = 0;
    struct window **window_aggregate_list = ts_alloc_aligned(sizeof(struct window *), window_count);

    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_manager_find_window(&g_window_manager, window_list[i]);
        if (window) window_aggregate_list[(*window_aggregate_count)++] = window;
    }

    return window_aggregate_list;
}

void window_manager_query_windows_for_spaces(FILE *rsp, uint64_t *space_list, int space_count)
{
    int window_count = 0;
    struct window **window_list = window_manager_find_windows_for_spaces(space_list, space_count, &window_count);

    fprintf(rsp, "[");
    for (int i = 0; i < window_count; ++i) {
        window_serialize(rsp, window_list[i]);
        if (i < window_count - 1) fprintf(rsp, ",");
    }
    fprintf(rsp, "]\n");
}

void window_manager_query_windows_for_display(FILE *rsp, uint32_t did)
{
    int space_count = 0;
    uint64_t *space_list = display_space_list(did, &space_count);
    window_manager_query_windows_for_spaces(rsp, space_list, space_count);
}

void window_manager_query_windows_for_displays(FILE *rsp)
{
    uint32_t display_count = 0;
    uint32_t *display_list = display_manager_active_display_list(&display_count);

    int space_count = 0;
    uint64_t *space_list = NULL;

    for (int i = 0; i < display_count; ++i) {
        int count;
        uint64_t *list = display_space_list(display_list[i], &count);
        if (!list) continue;

        //
        // NOTE(koekeishiya): display_space_list(..) uses a linear allocator,
        // and so we only need to track the beginning of the first list along
        // with the total number of spaces that have been allocated.
        //

        if (!space_list) space_list = list;
        space_count += count;
    }

    window_manager_query_windows_for_spaces(rsp, space_list, space_count);
}

void window_manager_apply_rule_to_window(struct space_manager *sm, struct window_manager *wm, struct window *window, struct rule *rule)
{
    int regex_match_app = rule->app_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
    if (regex_match(rule->app_regex_valid,   &rule->app_regex,   window->application->name) == regex_match_app)   return;

    int regex_match_title = rule->title_regex_exclude ? REGEX_MATCH_YES : REGEX_MATCH_NO;
    if (regex_match(rule->title_regex_valid, &rule->title_regex, window_title(window)) == regex_match_title)      return;

    if (rule->sid || rule->did) {
        if (!window_is_fullscreen(window) && !space_is_fullscreen(window_space(window))) {
            uint64_t sid = rule->did ? display_space_id(rule->did) : rule->sid;
            window_manager_send_window_to_space(sm, wm, window, sid, true);
            if (rule->follow_space || rule->fullscreen == RULE_PROP_ON) {
                space_manager_focus_space(sid);
            }
        }
    }

    if (in_range_ei(rule->alpha, 0.0f, 1.0f)) {
        window->opacity = rule->alpha;
        window_manager_set_opacity(wm, window, rule->alpha);
    }

    if (rule->manage == RULE_PROP_ON) {
        window->rule_manage = true;
        window_manager_make_window_floating(sm, wm, window, false);
    } else if (rule->manage == RULE_PROP_OFF) {
        window->rule_manage = false;
        window_manager_make_window_floating(sm, wm, window, true);
    }

    if (rule->sticky == RULE_PROP_ON) {
        window_manager_make_window_sticky(sm, wm, window, true);
    } else if (rule->sticky == RULE_PROP_OFF) {
        window_manager_make_window_sticky(sm, wm, window, false);
    }

    if (rule->mff == RULE_PROP_ON) {
        window->rule_mff = true;
        window->enable_mff = true;
    } else if (rule->mff == RULE_PROP_OFF) {
        window->rule_mff = true;
        window->enable_mff = false;
    }

    if (rule->layer) {
        window_manager_set_window_layer(window, rule->layer);
    }

    if (rule->border == RULE_PROP_ON) {
        border_create(window);
    } else if (rule->border == RULE_PROP_OFF) {
        border_destroy(window);
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
        window_manager_apply_rule_to_window(sm, wm, window, &wm->rules[i]);
    }
}

void window_manager_set_window_border_enabled(struct window_manager *wm, bool enabled)
{
    wm->enable_window_border = enabled;

    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if (enabled) {
                    border_create(window);
                } else {
                    border_destroy(window);
                }
            }

            bucket = bucket->next;
        }
    }

    struct window *window = window_manager_focused_window(wm);
    if (window) border_activate(window);
}

void window_manager_set_window_border_width(struct window_manager *wm, int width)
{
    wm->border_width = width;
    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if (window->border.id) {
                    CGContextSetLineWidth(window->border.context, width);
                    border_redraw(window);
                }
            }

            bucket = bucket->next;
        }
    }
}

void window_manager_set_active_window_border_color(struct window_manager *wm, uint32_t color)
{
    wm->active_border_color = rgba_color_from_hex(color);
    struct window *window = window_manager_focused_window(wm);
    if (window) border_activate(window);
}

void window_manager_set_normal_window_border_color(struct window_manager *wm, uint32_t color)
{
    wm->normal_border_color = rgba_color_from_hex(color);
    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if (window->id != wm->focused_window_id) {
                    border_deactivate(window);
                }
            }

            bucket = bucket->next;
        }
    }
}

void window_manager_set_window_opacity_enabled(struct window_manager *wm, bool enabled)
{
    wm->enable_window_opacity = enabled;
    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                window_manager_set_opacity(wm, window, enabled ? window->opacity : 1.0f);
            }

            bucket = bucket->next;
        }
    }
}

void window_manager_center_mouse(struct window_manager *wm, struct window *window)
{
    if (window->rule_mff) {
        if (!window->enable_mff) return;
    } else {
        if (!wm->enable_mff)     return;
    }

    CGPoint cursor;
    SLSGetCurrentCursorLocation(g_connection, &cursor);
    if (CGRectContainsPoint(window->frame, cursor)) return;

    uint32_t did = window_display_id(window);
    if (!did) return;

    CGPoint center = {
        window->frame.origin.x + window->frame.size.width / 2,
        window->frame.origin.y + window->frame.size.height / 2
    };

    CGRect bounds = display_bounds(did);
    if (!CGRectContainsPoint(bounds, center)) return;

    CGWarpMouseCursorPosition(center);
}

bool window_manager_should_manage_window(struct window *window)
{
    if (window->is_floating)      return false;
    if (window_is_sticky(window)) return false;
    if (window->rule_manage)      return true;

    return ((window_level_is_standard(window)) &&
            (window_is_standard(window)) &&
            (window_can_move(window)));
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
    if (view->layout == VIEW_FLOAT) return;
    table_add(&wm->managed_window, &window->id, view);
    window_manager_purify_window(wm, window);
}

enum window_op_error window_manager_adjust_window_ratio(struct window_manager *wm, struct window *window, int type, float ratio)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (!view) return WINDOW_OP_ERROR_INVALID_SRC_VIEW;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node || !node->parent) return WINDOW_OP_ERROR_INVALID_SRC_NODE;

    switch (type) {
    case TYPE_REL: {
        node->parent->ratio = clampf_range(node->parent->ratio + ratio, 0.1f, 0.9f);
    } break;
    case TYPE_ABS: {
        node->parent->ratio = clampf_range(ratio, 0.1f, 0.9f);
    } break;
    }

    window_node_update(view, node->parent);
    window_node_flush(node->parent);

    return WINDOW_OP_ERROR_SUCCESS;
}

enum window_op_error window_manager_move_window_relative(struct window_manager *wm, struct window *window, int type, float dx, float dy)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) return WINDOW_OP_ERROR_INVALID_SRC_VIEW;

    if (type == TYPE_REL) {
        dx += window->frame.origin.x;
        dy += window->frame.origin.y;
    }

    AX_ENHANCED_UI_WORKAROUND(window->application->ref, {
        window_manager_move_window(window, dx, dy);
    });

    return WINDOW_OP_ERROR_SUCCESS;
}

void window_manager_resize_window_relative_internal(struct window *window, CGRect frame, int direction, float dx, float dy)
{
    int x_mod = (direction & HANDLE_LEFT) ? -1 : (direction & HANDLE_RIGHT)  ? 1 : 0;
    int y_mod = (direction & HANDLE_TOP)  ? -1 : (direction & HANDLE_BOTTOM) ? 1 : 0;

    float fw = max(1, frame.size.width  + dx * x_mod);
    float fh = max(1, frame.size.height + dy * y_mod);
    float fx = (direction & HANDLE_LEFT) ? frame.origin.x + frame.size.width  - fw : frame.origin.x;
    float fy = (direction & HANDLE_TOP)  ? frame.origin.y + frame.size.height - fh : frame.origin.y;

    AX_ENHANCED_UI_WORKAROUND(window->application->ref, {
        window_manager_move_window(window, fx, fy);
        window_manager_resize_window(window, fw, fh);
    });
}

enum window_op_error window_manager_resize_window_relative(struct window_manager *wm, struct window *window, int direction, float dx, float dy)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) {
        if (direction == HANDLE_ABS) return WINDOW_OP_ERROR_INVALID_OPERATION;

        struct window_node *node = view_find_window_node(view, window->id);
        if (!node) return WINDOW_OP_ERROR_INVALID_SRC_NODE;

        struct window_node *x_fence = NULL;
        struct window_node *y_fence = NULL;

        if (direction & HANDLE_TOP)    x_fence = window_node_fence(node, DIR_NORTH);
        if (direction & HANDLE_BOTTOM) x_fence = window_node_fence(node, DIR_SOUTH);
        if (direction & HANDLE_LEFT)   y_fence = window_node_fence(node, DIR_WEST);
        if (direction & HANDLE_RIGHT)  y_fence = window_node_fence(node, DIR_EAST);
        if (!x_fence && !y_fence)      return WINDOW_OP_ERROR_INVALID_DST_NODE;

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
            AX_ENHANCED_UI_WORKAROUND(window->application->ref, {
                window_manager_resize_window(window, dx, dy);
            });
        } else {
            window_manager_resize_window_relative_internal(window, window->frame, direction, dx, dy);
        }
    }

    return WINDOW_OP_ERROR_SUCCESS;
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
    AX_ENHANCED_UI_WORKAROUND(window->application->ref, {
        window_manager_resize_window(window, width, height);
        window_manager_move_window(window, x, y);
        window_manager_resize_window(window, width, height);
    });
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

bool window_manager_set_opacity(struct window_manager *wm, struct window *window, float opacity)
{
    if (opacity == 0.0f) {
        if (wm->enable_window_opacity) {
            opacity = window->id == wm->focused_window_id ? wm->active_window_opacity : wm->normal_window_opacity;
        } else {
            opacity = 1.0f;
        }
    }

    return scripting_addition_set_opacity(window->id, opacity, wm->window_opacity_duration);
}

void window_manager_set_window_opacity(struct window_manager *wm, struct window *window, float opacity)
{
    if (!wm->enable_window_opacity) return;
    if (window->opacity != 0.0f)    return;
    if ((!window->rule_manage) && (!window_is_standard(window)) && (!window_is_dialog(window))) return;

    window_manager_set_opacity(wm, window, opacity);
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
                if (window->id == wm->focused_window_id) goto next;
                window_manager_set_window_opacity(wm, window, wm->normal_window_opacity);
            }

next:
            bucket = bucket->next;
        }
    }
}

bool window_manager_set_window_layer(struct window *window, int layer)
{
    bool result = scripting_addition_set_layer(window->id, layer);

    CFArrayRef window_list = SLSCopyAssociatedWindows(g_connection, window->id);
    if (!window_list) return result;

    int window_count = CFArrayGetCount(window_list);
    CFTypeRef query = SLSWindowQueryWindows(g_connection, window_list, window_count);
    CFTypeRef iterator = SLSWindowQueryResultCopyWindows(query);

    int relation_count = 0;
    uint32_t parent_list[window_count];
    uint32_t child_list[window_count];

    while (SLSWindowIteratorAdvance(iterator)) {
        parent_list[relation_count] = SLSWindowIteratorGetParentID(iterator);
        child_list[relation_count] = SLSWindowIteratorGetWindowID(iterator);
        ++relation_count;
    }

    int check_count = 1;
    uint32_t check_list[window_count];
    check_list[0] = window->id;

    for (int i = 0; i < check_count; ++i) {
        for (int j = 0; j < window_count; ++j) {
            if (parent_list[j] != check_list[i]) continue;
            scripting_addition_set_layer(child_list[j], layer);
            check_list[check_count++] = child_list[j];
        }
    }

    CFRelease(query);
    CFRelease(iterator);
    CFRelease(window_list);

    return result;
}

void window_manager_make_window_topmost(struct window_manager *wm, struct window *window, bool topmost)
{
    if (!wm->enable_window_topmost) return;

    int layer = topmost ? LAYER_ABOVE : LAYER_NORMAL;
    window_manager_set_window_layer(window, layer);
}

void window_manager_purify_window(struct window_manager *wm, struct window *window)
{
    int value;

    if (wm->purify_mode == PURIFY_DISABLED) {
        value = 1;
    } else if (wm->purify_mode == PURIFY_MANAGED) {
        value = window_manager_find_managed_window(wm, window) ? 0 : 1;
    } else /*if (wm->purify_mode == PURIFY_ALWAYS) */ {
        value = 0;
    }

    if (scripting_addition_set_shadow(window->id, value)) window->has_shadow = value;
}

struct window *window_manager_find_window_on_space_by_rank(struct window_manager *wm, uint64_t sid, int rank)
{
    int count;
    uint32_t *window_list = space_window_list(sid, &count, false);
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

    return result;
}

struct window *window_manager_find_window_at_point_filtering_window(struct window_manager *wm, CGPoint point, uint32_t filter_wid)
{
    CGPoint window_point;
    uint32_t window_id;
    int window_cid;

    SLSFindWindowByGeometry(g_connection, filter_wid, -1, 0, &point, &window_point, &window_id, &window_cid);
    return window_manager_find_window(wm, window_id);
}

struct window *window_manager_find_window_at_point(struct window_manager *wm, CGPoint point)
{
    CGPoint window_point;
    uint32_t window_id;
    int window_cid;

    SLSFindWindowByGeometry(g_connection, 0, 1, 0, &point, &window_point, &window_id, &window_cid);
    if (g_connection == window_cid) SLSFindWindowByGeometry(g_connection, window_id, -1, 0, &point, &window_point, &window_id, &window_cid);

    return window_manager_find_window(wm, window_id);
}

struct window *window_manager_find_window_below_cursor(struct window_manager *wm)
{
    CGPoint cursor;
    SLSGetCurrentCursorLocation(g_connection, &cursor);
    return window_manager_find_window_at_point(wm, cursor);
}

struct window *window_manager_find_closest_managed_window_in_direction(struct window_manager *wm, struct window *window, int direction)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

    struct window_node *closest = view_find_window_node_in_direction(view, node, direction);
    if (!closest) return NULL;

    return window_manager_find_window(wm, closest->window_order[0]);
}

struct window *window_manager_find_prev_managed_window(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

    struct window_node *prev = window_node_find_prev_leaf(node);
    if (!prev) return NULL;

    return window_manager_find_window(wm, prev->window_order[0]);
}

struct window *window_manager_find_next_managed_window(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

    struct window_node *next = window_node_find_next_leaf(node);
    if (!next) return NULL;

    return window_manager_find_window(wm, next->window_order[0]);
}

struct window *window_manager_find_first_managed_window(struct space_manager *sm, struct window_manager *wm)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *first = window_node_find_first_leaf(view->root);
    if (!first) return NULL;

    return window_manager_find_window(wm, first->window_order[0]);
}

struct window *window_manager_find_last_managed_window(struct space_manager *sm, struct window_manager *wm)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *last = window_node_find_last_leaf(view->root);
    if (!last) return NULL;

    return window_manager_find_window(wm, last->window_order[0]);
}

struct window *window_manager_find_recent_managed_window(struct space_manager *sm, struct window_manager *wm)
{
    struct window *window = window_manager_find_window(wm, wm->last_window_id);
    if (!window) return NULL;

    struct view *view = window_manager_find_managed_window(wm, window);
    if (!view) return NULL;

    return window;
}

struct window *window_manager_find_prev_window_in_stack(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

    for (int i = 1; i < node->window_count; ++i) {
        if (node->window_list[i] == window->id) {
            return window_manager_find_window(wm, node->window_list[i-1]);
        }
    }

    return NULL;
}

struct window *window_manager_find_next_window_in_stack(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

    for (int i = 0; i < node->window_count - 1; ++i) {
        if (node->window_list[i] == window->id) {
            return window_manager_find_window(wm, node->window_list[i+1]);
        }
    }

    return NULL;
}

struct window *window_manager_find_first_window_in_stack(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

    return window_manager_find_window(wm, node->window_list[0]);
}

struct window *window_manager_find_last_window_in_stack(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

    return window_manager_find_window(wm, node->window_list[node->window_count-1]);
}

struct window *window_manager_find_recent_window_in_stack(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (!view) return NULL;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

    if (node->window_count > 1) {
        return window_manager_find_window(wm, node->window_order[1]);
    }

    return NULL;
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
            best_id   = node->window_order[0];
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
            best_id   = node->window_order[0];
            best_area = area;
        }
    }

    return best_id ? window_manager_find_window(wm, best_id) : NULL;
}

static void window_manager_make_key_window(ProcessSerialNumber *window_psn, uint32_t window_id)
{
    uint8_t bytes1[0xf8] = { [0x04] = 0xf8, [0x08] = 0x01, [0x3a] = 0x10 };
    uint8_t bytes2[0xf8] = { [0x04] = 0xf8, [0x08] = 0x02, [0x3a] = 0x10 };

    memcpy(bytes1 + 0x3c, &window_id, sizeof(uint32_t));
    memset(bytes1 + 0x20, 0xFF, 0x10);

    memcpy(bytes2 + 0x3c, &window_id, sizeof(uint32_t));
    memset(bytes2 + 0x20, 0xFF, 0x10);

    SLPSPostEventRecordTo(window_psn, bytes1);
    SLPSPostEventRecordTo(window_psn, bytes2);
}

void window_manager_focus_window_without_raise(ProcessSerialNumber *window_psn, uint32_t window_id)
{
    if (psn_equals(window_psn, &g_window_manager.focused_window_psn)) {
        uint8_t bytes1[0xf8] = { [0x04] = 0xf8, [0x08] = 0x0d, [0x8a] = 0x02 };
        memcpy(bytes1 + 0x3c, &g_window_manager.focused_window_id, sizeof(uint32_t));
        SLPSPostEventRecordTo(&g_window_manager.focused_window_psn, bytes1);

        // @hack
        // Artificially delay the activation by 1ms. This is necessary
        // because some applications appear to be confused if both of
        // the events appear instantaneously.
        usleep(10000);

        uint8_t bytes2[0xf8] = { [0x04] = 0xf8, [0x08] = 0x0d, [0x8a] = 0x01 };
        memcpy(bytes2 + 0x3c, &window_id, sizeof(uint32_t));
        SLPSPostEventRecordTo(window_psn, bytes2);
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
    scripting_addition_focus_window(window_id);
#endif
}

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

struct window *window_manager_focused_window(struct window_manager *wm)
{
    struct application *application = window_manager_focused_application(wm);
    if (!application) return NULL;

    uint32_t window_id = application_focused_window(application);
    return window_manager_find_window(wm, window_id);
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

struct window **window_manager_find_application_windows(struct window_manager *wm, struct application *application, int *window_count)
{
    *window_count = 0;
    struct window **window_list = ts_alloc_aligned(sizeof(struct window *), wm->window.count);

    for (int window_index = 0; window_index < wm->window.capacity; ++window_index) {
        struct bucket *bucket = wm->window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if (window->application == application) {
                    window_list[(*window_count)++] = window;
                }
            }

            bucket = bucket->next;
        }
    }

    return window_list;
}

struct window *window_manager_create_and_add_window(struct space_manager *sm, struct window_manager *wm, struct application *application, AXUIElementRef window_ref, uint32_t window_id)
{
    struct window *window = window_create(application, window_ref, window_id);

    if (window_is_unknown(window)) {
        debug("%s: ignoring AXUnknown window %s %d\n", __FUNCTION__, window->application->name, window->id);
        window_manager_remove_lost_focused_event(wm, window->id);
        window_destroy(window);
        return NULL;
    }

    if (window_is_popover(window)) {
        debug("%s: ignoring AXPopover window %s %d\n", __FUNCTION__, window->application->name, window->id);
        window_manager_remove_lost_focused_event(wm, window->id);
        window_destroy(window);
        return NULL;
    }

    window_manager_purify_window(wm, window);
    window_manager_set_window_opacity(wm, window, wm->normal_window_opacity);

    if (!window_observe(window)) {
        debug("%s: could not observe %s %d\n", __FUNCTION__, window->application->name, window->id);
        window_manager_make_window_topmost(wm, window, true);
        window_manager_remove_lost_focused_event(wm, window->id);
        window_unobserve(window);
        window_destroy(window);
        return NULL;
    }

    if (window_manager_find_lost_focused_event(wm, window->id)) {
        event_loop_post(&g_event_loop, WINDOW_FOCUSED, (void *)(intptr_t) window->id, 0, NULL);
        window_manager_remove_lost_focused_event(wm, window->id);
    }

    debug("%s: %s %d\n", __FUNCTION__, window->application->name, window->id);
    window_manager_add_window(wm, window);
    window_manager_apply_rules_to_window(sm, wm, window);

    if ((!application->is_hidden) && (!window->is_minimized) && (!window->is_fullscreen) && (!window->rule_manage)) {
        if (window->rule_fullscreen) {
            window->rule_fullscreen = false;
        } else if ((!window_level_is_standard(window)) ||
                   (!window_is_standard(window)) ||
                   (!window_can_move(window)) ||
                   (window_is_sticky(window)) ||
                   (!window_can_resize(window) && window_is_undersized(window))) {
            window_manager_make_window_topmost(wm, window, true);
            window->is_floating = true;
        }
    }

    return window;
}

void window_manager_add_application_windows(struct space_manager *sm, struct window_manager *wm, struct application *application)
{
    CFArrayRef window_list_ref = application_window_list(application);
    if (!window_list_ref) return;

    int window_count = CFArrayGetCount(window_list_ref);
    for (int i = 0; i < window_count; ++i) {
        AXUIElementRef window_ref = CFArrayGetValueAtIndex(window_list_ref, i);
        uint32_t window_id = ax_window_id(window_ref);
        if (!window_id || window_manager_find_window(wm, window_id)) continue;
        window_manager_create_and_add_window(sm, wm, application, CFRetain(window_ref), window_id);
    }

    CFRelease(window_list_ref);
}

enum window_op_error window_manager_set_window_insertion(struct space_manager *sm, struct window_manager *wm, struct window *window, int direction)
{
    uint64_t sid = window_space(window);
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return WINDOW_OP_ERROR_INVALID_SRC_VIEW;

    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return WINDOW_OP_ERROR_INVALID_SRC_NODE;

    if (view->insertion_point && view->insertion_point != window->id) {
        struct window_node *insert_node = view_find_window_node(view, view->insertion_point);
        if (insert_node) {
            insert_feedback_destroy(insert_node);
            insert_node->split = SPLIT_NONE;
            insert_node->child = CHILD_NONE;
            insert_node->insert_dir = 0;
        }
    }

    if (direction == node->insert_dir) {
        insert_feedback_destroy(node);
        node->split = SPLIT_NONE;
        node->child = CHILD_NONE;
        node->insert_dir = 0;
        view->insertion_point = 0;
        return WINDOW_OP_ERROR_SUCCESS;
    }

    if (direction == DIR_NORTH) {
        node->split = SPLIT_X;
        node->child = CHILD_FIRST;
    } else if (direction == DIR_EAST) {
        node->split = SPLIT_Y;
        node->child = CHILD_SECOND;
    } else if (direction == DIR_SOUTH) {
        node->split = SPLIT_X;
        node->child = CHILD_SECOND;
    } else if (direction == DIR_WEST) {
        node->split = SPLIT_Y;
        node->child = CHILD_FIRST;
    }

    node->insert_dir = direction;
    view->insertion_point = node->window_order[0];
    insert_feedback_show(node);

    return WINDOW_OP_ERROR_SUCCESS;
}

enum window_op_error window_manager_stack_window(struct space_manager *sm, struct window_manager *wm, struct window *a, struct window *b)
{
    if (a->id == b->id) return WINDOW_OP_ERROR_SAME_WINDOW;

    struct view *a_view = window_manager_find_managed_window(wm, a);
    if (!a_view) return WINDOW_OP_ERROR_INVALID_SRC_NODE;

    struct view *b_view = window_manager_find_managed_window(wm, b);
    if (b_view) {
        space_manager_untile_window(sm, b_view, b);
        window_manager_remove_managed_window(wm, b->id);
        window_manager_purify_window(wm, b);
    } else if (b->is_floating) {
        b->is_floating = false;
        window_manager_make_window_topmost(wm, b, false);
        if (b->is_sticky) window_manager_make_window_sticky(sm, wm, b, false);
    }

    struct window_node *a_node = view_find_window_node(a_view, a->id);
    if (a_node->window_count+1 >= NODE_MAX_WINDOW_COUNT) return WINDOW_OP_ERROR_MAX_STACK;

    view_stack_window_node(a_view, a_node, b);
    window_manager_add_managed_window(wm, b, a_view);

    return WINDOW_OP_ERROR_SUCCESS;
}

enum window_op_error window_manager_warp_window(struct space_manager *sm, struct window_manager *wm, struct window *a, struct window *b)
{
    if (a->id == b->id) return WINDOW_OP_ERROR_SAME_WINDOW;

    uint64_t a_sid = window_space(a);
    struct view *a_view = space_manager_find_view(sm, a_sid);
    if (a_view->layout != VIEW_BSP) return WINDOW_OP_ERROR_INVALID_SRC_VIEW;

    uint64_t b_sid = window_space(b);
    struct view *b_view = space_manager_find_view(sm, b_sid);
    if (b_view->layout != VIEW_BSP) return WINDOW_OP_ERROR_INVALID_DST_VIEW;

    struct window_node *a_node = view_find_window_node(a_view, a->id);
    if (!a_node) return WINDOW_OP_ERROR_INVALID_SRC_NODE;

    struct window_node *b_node = view_find_window_node(b_view, b->id);
    if (!b_node) return WINDOW_OP_ERROR_INVALID_DST_NODE;

    if (a_node == b_node) return WINDOW_OP_ERROR_SAME_STACK;

    if (a_node->parent == b_node->parent && a_node->window_count == 1) {
        if (window_node_contains_window(b_node, b_view->insertion_point)) {
            b_node->parent->split = b_node->split;
            b_node->parent->child = b_node->child;
            space_manager_untile_window(sm, a_view, a);
            window_manager_remove_managed_window(wm, a->id);
            window_manager_add_managed_window(wm, a, b_view);
            space_manager_tile_window_on_space_with_insertion_point(sm, a, b_view->sid, b->id);
        } else {
            if (window_node_contains_window(a_node, a_view->insertion_point)) {
                a_view->insertion_point = b->id;
            }

            window_node_swap_window_list(a_node, b_node);

            window_node_flush(a_node);
            window_node_flush(b_node);
        }
    } else {
        space_manager_untile_window(sm, a_view, a);

        if (a_view->sid != b_view->sid) {
            window_manager_remove_managed_window(wm, a->id);
            window_manager_add_managed_window(wm, a, b_view);

            if (wm->focused_window_id == a->id) {
                struct window *next = window_manager_find_window_on_space_by_rank(wm, a_view->sid, 2);
                if (next) {
                    window_manager_focus_window_with_raise(&next->application->psn, next->id, next->ref);
                } else {
                    _SLPSSetFrontProcessWithOptions(&g_process_manager.finder_psn, 0, kCPSNoWindows);
                }
            }

            space_manager_move_window_to_space(b_view->sid, a);
        }

        space_manager_tile_window_on_space_with_insertion_point(sm, a, b_view->sid, b->id);
    }

    return WINDOW_OP_ERROR_SUCCESS;
}

enum window_op_error window_manager_swap_window(struct space_manager *sm, struct window_manager *wm, struct window *a, struct window *b)
{
    if (a->id == b->id) return WINDOW_OP_ERROR_SAME_WINDOW;

    uint64_t a_sid = window_space(a);
    struct view *a_view = space_manager_find_view(sm, a_sid);
    if (a_view->layout != VIEW_BSP) return WINDOW_OP_ERROR_INVALID_SRC_VIEW;

    uint64_t b_sid = window_space(b);
    struct view *b_view = space_manager_find_view(sm, b_sid);
    if (b_view->layout != VIEW_BSP) return WINDOW_OP_ERROR_INVALID_DST_VIEW;

    struct window_node *a_node = view_find_window_node(a_view, a->id);
    if (!a_node) return WINDOW_OP_ERROR_INVALID_SRC_NODE;

    struct window_node *b_node = view_find_window_node(b_view, b->id);
    if (!b_node) return WINDOW_OP_ERROR_INVALID_DST_NODE;

    if (a_node == b_node) return WINDOW_OP_ERROR_SAME_STACK;

    if (window_node_contains_window(a_node, a_view->insertion_point)) {
        a_view->insertion_point = b->id;
    } else if (window_node_contains_window(b_node, b_view->insertion_point)) {
        b_view->insertion_point = a->id;
    }

    window_node_swap_window_list(a_node, b_node);

    if (a_view->sid != b_view->sid) {
        for (int i = 0; i < a_node->window_count; ++i) {
            struct window *window = window_manager_find_window(wm, a_node->window_list[i]);
            window_manager_remove_managed_window(wm, a_node->window_list[i]);
            window_manager_add_managed_window(wm, window, a_view);
            space_manager_move_window_to_space(a_view->sid, window);
        }

        for (int i = 0; i < b_node->window_count; ++i) {
            struct window *window = window_manager_find_window(wm, b_node->window_list[i]);
            window_manager_remove_managed_window(wm, b_node->window_list[i]);
            window_manager_add_managed_window(wm, window, b_view);
            space_manager_move_window_to_space(b_view->sid, window);
        }

        if (window_node_contains_window(a_node, wm->focused_window_id)) {
            window_manager_focus_window_with_raise(&b->application->psn, b->id, b->ref);
        } else if (window_node_contains_window(b_node, wm->focused_window_id)) {
            window_manager_focus_window_with_raise(&a->application->psn, a->id, a->ref);
        }
    }

    window_node_flush(a_node);
    window_node_flush(b_node);

    return WINDOW_OP_ERROR_SUCCESS;
}

enum window_op_error window_manager_minimize_window(struct window *window)
{
    if (!window_can_minimize(window)) return WINDOW_OP_ERROR_CANT_MINIMIZE;
    if (window->is_minimized)         return WINDOW_OP_ERROR_ALREADY_MINIMIZED;

    AXError result = AXUIElementSetAttributeValue(window->ref, kAXMinimizedAttribute, kCFBooleanTrue);
    return result == kAXErrorSuccess ? WINDOW_OP_ERROR_SUCCESS : WINDOW_OP_ERROR_MINIMIZE_FAILED;
}

enum window_op_error window_manager_deminimize_window(struct window *window)
{
    if (!window->is_minimized) return WINDOW_OP_ERROR_NOT_MINIMIZED;

    AXError result = AXUIElementSetAttributeValue(window->ref, kAXMinimizedAttribute, kCFBooleanFalse);
    return result == kAXErrorSuccess ? WINDOW_OP_ERROR_SUCCESS : WINDOW_OP_ERROR_DEMINIMIZE_FAILED;
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

void window_manager_send_window_to_space(struct space_manager *sm, struct window_manager *wm, struct window *window, uint64_t dst_sid, bool moved_by_rule)
{
    uint64_t src_sid = window_space(window);
    if (src_sid == dst_sid) return;

    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) {
        space_manager_untile_window(sm, view, window);
        window_manager_remove_managed_window(wm, window->id);
        window_manager_purify_window(wm, window);
    }

    if ((space_is_visible(src_sid) && (moved_by_rule || wm->focused_window_id == window->id))) {
        struct window *next = window_manager_find_window_on_space_by_rank(wm, src_sid, 2);
        if (next) {
            window_manager_focus_window_with_raise(&next->application->psn, next->id, next->ref);
        } else {
            _SLPSSetFrontProcessWithOptions(&g_process_manager.finder_psn, 0, kCPSNoWindows);
        }
    }

    space_manager_move_window_to_space(dst_sid, window);

    if (window_manager_should_manage_window(window) && !window->is_minimized) {
        struct view *view = space_manager_tile_window_on_space(sm, window, dst_sid);
        window_manager_add_managed_window(wm, window, view);
    }
}

enum window_op_error window_manager_apply_grid(struct space_manager *sm, struct window_manager *wm, struct window *window, unsigned r, unsigned c, unsigned x, unsigned y, unsigned w, unsigned h)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (view) return WINDOW_OP_ERROR_INVALID_SRC_VIEW;

    uint32_t did = window_display_id(window);
    if (!did) return WINDOW_OP_ERROR_INVALID_SRC_VIEW;

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

    float cw = bounds.size.width / c;
    float ch = bounds.size.height / r;
    float fx = bounds.origin.x + bounds.size.width  - cw * (c - x);
    float fy = bounds.origin.y + bounds.size.height - ch * (r - y);
    float fw = cw * w;
    float fh = ch * h;

    AX_ENHANCED_UI_WORKAROUND(window->application->ref, {
        window_manager_move_window(window, fx, fy);
        window_manager_resize_window(window, fw, fh);
    });

    return WINDOW_OP_ERROR_SUCCESS;
}

void window_manager_toggle_window_topmost(struct window *window)
{
    bool is_topmost = window_is_topmost(window);
    window_manager_set_window_layer(window, is_topmost ? LAYER_NORMAL : LAYER_ABOVE);
}

void window_manager_make_window_floating(struct space_manager *sm, struct window_manager *wm, struct window *window, bool should_float)
{
    if (should_float) {
        struct view *view = window_manager_find_managed_window(wm, window);
        if (view) {
            space_manager_untile_window(sm, view, window);
            window_manager_remove_managed_window(wm, window->id);
            window_manager_purify_window(wm, window);
        }
        window_manager_make_window_topmost(wm, window, true);
        window->is_floating = true;
    } else {
        window->is_floating = false;

        if (!window->is_sticky) {
            window_manager_make_window_topmost(wm, window, false);
            if ((window_manager_should_manage_window(window)) && (!window_manager_find_managed_window(wm, window))) {
                struct view *view = space_manager_tile_window_on_space(sm, window, space_manager_active_space());
                window_manager_add_managed_window(wm, window, view);
            }
        }
    }
}

void window_manager_make_window_sticky(struct space_manager *sm, struct window_manager *wm, struct window *window, bool should_sticky)
{
    if (should_sticky) {
        if (scripting_addition_set_sticky(window->id, true)) {
            struct view *view = window_manager_find_managed_window(wm, window);
            if (view) {
                space_manager_untile_window(sm, view, window);
                window_manager_remove_managed_window(wm, window->id);
                window_manager_purify_window(wm, window);
            }
            window_manager_make_window_topmost(wm, window, true);
            if (window->border.id) border_ensure_same_space(window);
            window->is_sticky = true;
        }
    } else {
        if (scripting_addition_set_sticky(window->id, false)) {
            if (window->border.id) border_ensure_same_space(window);
            window->is_sticky = false;

            if (!window->is_floating) {
                window_manager_make_window_topmost(wm, window, false);

                if ((window_manager_should_manage_window(window)) && (!window_manager_find_managed_window(wm, window))) {
                    struct view *view = space_manager_tile_window_on_space(sm, window, space_manager_active_space());
                    window_manager_add_managed_window(wm, window, view);
                }
            }
        }
    }
}

void window_manager_toggle_window_shadow(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    bool shadow = !window->has_shadow;
    if (scripting_addition_set_shadow(window->id, shadow)) window->has_shadow = shadow;
}

void window_manager_wait_for_native_fullscreen_transition(struct window *window)
{
    if (workspace_is_macos_mojave()) {
        while (!space_is_user(space_manager_active_space())) {

            //
            // NOTE(koekeishiya): Window has exited native-fullscreen mode.
            // We need to spin lock until the display is finished animating
            // because we are not actually able to interact with the window.
            //
            // macOS Mojave freezes fullscreen applications when using the
            // display_manager API to check for animation status:
            //
            //  - https://github.com/koekeishiya/yabai/issues/690
            //

            usleep(100000);
        }
    } else {
        uint32_t did = window_display_id(window);

        do {

            //
            // NOTE(koekeishiya): Window has exited native-fullscreen mode.
            // We need to spin lock until the display is finished animating
            // because we are not actually able to interact with the window.
            //

            usleep(100000);
        } while (display_manager_display_is_animating(did));
    }
}

void window_manager_toggle_window_native_fullscreen(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    uint32_t sid = window_space(window);

    // NOTE(koekeishiya): The window must become the focused window
    // before we can change its fullscreen attribute. We focus the
    // window and spin lock until a potential space animation has finished.
    window_manager_focus_window_with_raise(&window->application->psn, window->id, window->ref);
    while (sid != space_manager_active_space()) { usleep(100000); }

    if (!window_is_fullscreen(window)) {
        AXUIElementSetAttributeValue(window->ref, kAXFullscreenAttribute, kCFBooleanTrue);
    } else {
        AXUIElementSetAttributeValue(window->ref, kAXFullscreenAttribute, kCFBooleanFalse);
    }

    // NOTE(koekeishiya): We toggled the fullscreen attribute and must
    // now spin lock until the post-exit space animation has finished.
    while (sid == space_manager_active_space()) { usleep(100000); }
}

void window_manager_toggle_window_parent(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (!view || view->layout != VIEW_BSP) return;

    struct window_node *node = view_find_window_node(view, window->id);
    assert(node);

    if (node->zoom) {
        node->zoom = NULL;
    } else if (node->parent) {
        node->zoom = node->parent;
    }

    window_node_flush(node);
}

void window_manager_toggle_window_fullscreen(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    struct view *view = window_manager_find_managed_window(wm, window);
    if (!view || view->layout != VIEW_BSP) return;

    struct window_node *node = view_find_window_node(view, window->id);
    assert(node);

    if (node->zoom) {
        node->zoom = NULL;
    } else {
        node->zoom = view->root;
    }

    window_node_flush(node);
}

void window_manager_toggle_window_expose(struct window_manager *wm, struct window *window)
{
    window_manager_focus_window_with_raise(&window->application->psn, window->id, window->ref);
    CoreDockSendNotification(CFSTR("com.apple.expose.front.awake"), 0);
}

void window_manager_toggle_window_pip(struct space_manager *sm, struct window_manager *wm, struct window *window)
{
    uint32_t did = window_display_id(window);
    if (!did) return;

    uint64_t sid = display_space_id(did);
    struct view *dview = space_manager_find_view(sm, sid);

    CGRect bounds = display_bounds_constrained(did);
    if (dview && dview->enable_padding) {
        bounds.origin.x    += dview->left_padding;
        bounds.size.width  -= (dview->left_padding + dview->right_padding);
        bounds.origin.y    += dview->top_padding;
        bounds.size.height -= (dview->top_padding + dview->bottom_padding);
    }

    scripting_addition_scale_window(window->id, bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
}

void window_manager_toggle_window_border(struct window_manager *wm, struct window *window)
{
    if (window->border.id) {
        border_destroy(window);
    } else {
        border_create(window);
        if (window->id == wm->focused_window_id) border_activate(window);
    }
}

static void window_manager_validate_windows_on_space(struct space_manager *sm, struct window_manager *wm, uint64_t sid, uint32_t *window_list, int window_count)
{
    int view_window_count;
    struct view *view = space_manager_find_view(sm, sid);
    uint32_t *view_window_list = view_find_window_list(view, &view_window_count);

    for (int i = 0; i < view_window_count; ++i) {
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
}

static void window_manager_check_for_windows_on_space(struct space_manager *sm, struct window_manager *wm, uint64_t sid, uint32_t *window_list, int window_count)
{
    for (int i = 0; i < window_count; ++i) {
        struct window *window = window_manager_find_window(wm, window_list[i]);
        if (!window || !window_manager_should_manage_window(window)) continue;
        if (window->is_minimized || window->application->is_hidden)  continue;

        struct view *existing_view = window_manager_find_managed_window(wm, window);
        if (existing_view && existing_view->layout != VIEW_FLOAT && existing_view->sid != sid) {
            space_manager_untile_window(sm, existing_view, window);
            window_manager_remove_managed_window(wm, window->id);
            window_manager_purify_window(wm, window);
        }

        if (!existing_view || (existing_view->layout != VIEW_FLOAT && existing_view->sid != sid)) {
            struct view *view = space_manager_tile_window_on_space(sm, window, sid);
            window_manager_add_managed_window(wm, window, view);
        }
    }
}

void window_manager_validate_and_check_for_windows_on_space(struct space_manager *sm, struct window_manager *wm, uint64_t sid)
{
    int window_count;
    uint32_t *window_list = space_window_list(sid, &window_count, false);
    if (!window_list) return;

    window_manager_validate_windows_on_space(sm, wm, sid, window_list, window_count);
    window_manager_check_for_windows_on_space(sm, wm, sid, window_list, window_count);
}

void window_manager_correct_for_mission_control_changes(struct space_manager *sm, struct window_manager *wm)
{
    uint32_t display_count = 0;
    uint32_t *display_list = display_manager_active_display_list(&display_count);
    if (!display_list) return;

    for (int i = 0; i < display_count; ++i) {
        uint32_t did = display_list[i];

        int space_count;
        uint64_t *space_list = display_space_list(did, &space_count);
        if (!space_list) continue;

        uint64_t sid = display_space_id(did);
        for (int i = 0; i < space_count; ++i) {
            if (space_list[i] == sid) {
                window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, sid);
                space_manager_refresh_view(sm, sid);
            } else {
                space_manager_mark_view_invalid(sm, space_list[i]);
            }
        }
    }
}

void window_manager_handle_display_add_and_remove(struct space_manager *sm, struct window_manager *wm, uint32_t did)
{
    int space_count;
    uint64_t *space_list = display_space_list(did, &space_count);
    if (!space_list) return;

    for (int i = 0; i < space_count; ++i) {
        if (space_is_user(space_list[i])) {
            int window_count;
            uint32_t *window_list = space_window_list(space_list[i], &window_count, false);
            if (window_list) {
                window_manager_check_for_windows_on_space(sm, wm, space_list[i], window_list, window_count);
            }
            break;
        }
    }

    uint64_t sid = display_space_id(did);
    for (int i = 0; i < space_count; ++i) {
        if (space_list[i] == sid) {
            space_manager_refresh_view(sm, sid);
        } else {
            space_manager_mark_view_invalid(sm, space_list[i]);
        }
    }
}

void window_manager_init(struct window_manager *wm)
{
    wm->system_element = AXUIElementCreateSystemWide();
    AXUIElementSetMessagingTimeout(wm->system_element, 1.0);

    wm->ffm_mode = FFM_DISABLED;
    wm->purify_mode = PURIFY_DISABLED;
    wm->window_origin_mode = WINDOW_ORIGIN_DEFAULT;
    wm->enable_mff = false;
    wm->enable_window_border = false;
    wm->enable_window_opacity = false;
    wm->enable_window_topmost = false;
    wm->active_window_opacity = 1.0f;
    wm->normal_window_opacity = 1.0f;
    wm->window_opacity_duration = 0.0f;
    wm->insert_feedback_windows = NULL;
    wm->insert_feedback_color = rgba_color_from_hex(0xffd75f5f);
    wm->active_border_color = rgba_color_from_hex(0xff775759);
    wm->normal_border_color = rgba_color_from_hex(0xff555555);
    wm->border_width = 6;

    table_init(&wm->application, 150, hash_wm, compare_wm);
    table_init(&wm->window, 150, hash_wm, compare_wm);
    table_init(&wm->managed_window, 150, hash_wm, compare_wm);
    table_init(&wm->window_lost_focused_event, 150, hash_wm, compare_wm);
    table_init(&wm->application_lost_front_switched_event, 150, hash_wm, compare_wm);
}

void window_manager_begin(struct space_manager *sm, struct window_manager *wm)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
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
    [pool drain];

    struct window *window = window_manager_focused_window(wm);
    if (window) {
        wm->last_window_id = window->id;
        wm->focused_window_id = window->id;
        wm->focused_window_psn = window->application->psn;
        window_manager_set_window_opacity(wm, window, wm->active_window_opacity);
        border_activate(window);
    }
}

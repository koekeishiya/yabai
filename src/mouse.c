void mouse_window_info_populate(struct mouse_state *ms, struct mouse_window_info *info)
{
    CGRect frame = ms->window->frame;

    info->dx = frame.origin.x    - ms->window_frame.origin.x;
    info->dy = frame.origin.y    - ms->window_frame.origin.y;
    info->dw = frame.size.width  - ms->window_frame.size.width;
    info->dh = frame.size.height - ms->window_frame.size.height;

    info->changed_x = info->dx != 0.0f;
    info->changed_y = info->dy != 0.0f;
    info->changed_w = info->dw != 0.0f;
    info->changed_h = info->dh != 0.0f;

    info->changed_position = info->changed_x || info->changed_y;
    info->changed_size     = info->changed_w || info->changed_h;
}

enum mouse_drop_action mouse_determine_drop_action(struct mouse_state *ms, struct window_node *src_node, struct window *dst_window, CGPoint point)
{
    CGRect  f    = dst_window->frame;
    CGPoint wp   = { point.x - f.origin.x, point.y - f.origin.y };
    CGRect  c    = {{ 0.25f * f.size.width, 0.25f * f.size.height }, { 0.50f * f.size.width, 0.50f * f.size.height }};
    CGPoint t[3] = {{ 0.0f, 0.0f}, { 0.5f * f.size.width, 0.5f * f.size.height }, { f.size.width, 0.0f }};
    CGPoint r[3] = {{ f.size.width, 0.0f }, { 0.5f * f.size.width, 0.5f * f.size.height }, { f.size.width, f.size.height }};
    CGPoint b[3] = {{ f.size.width, f.size.height }, { 0.5f * f.size.width, 0.5f * f.size.height }, { 0.0f, f.size.height }};
    CGPoint l[3] = {{ 0.0f, f.size.height }, { 0.5f * f.size.width, 0.5f * f.size.height }, { 0.0f, 0.0f }};

    if ((CGRectContainsPoint(c, wp)) && (src_node->window_count == 1)) {
        return ms->drop_action == MOUSE_MODE_STACK ? MOUSE_DROP_ACTION_STACK : MOUSE_DROP_ACTION_SWAP;
    } else if (triangle_contains_point(t, wp)) {
        return MOUSE_DROP_ACTION_WARP_TOP;
    } else if (triangle_contains_point(r, wp)) {
        return MOUSE_DROP_ACTION_WARP_RIGHT;
    } else if (triangle_contains_point(b, wp)) {
        return MOUSE_DROP_ACTION_WARP_BOTTOM;
    } else if (triangle_contains_point(l, wp)) {
        return MOUSE_DROP_ACTION_WARP_LEFT;
    }

    return MOUSE_DROP_ACTION_NONE;
}

void mouse_drop_action_stack(struct space_manager *sm, struct window_manager *wm, struct view *src_view, struct window *src_window, struct view *dst_view, struct window *dst_window)
{
    space_manager_untile_window(sm, src_view, src_window);
    window_manager_remove_managed_window(wm, src_window->id);

    struct window_node *dst_node = view_find_window_node(dst_view, dst_window->id);
    if (dst_node->window_count+1 < NODE_MAX_WINDOW_COUNT) {
        view_stack_window_node(dst_view, dst_node, src_window);
        window_manager_add_managed_window(wm, src_window, dst_view);

        if (dst_node->zoom) {
            window_manager_animate_window((struct window_capture) { src_window, dst_node->zoom->area.x, dst_node->zoom->area.y, dst_node->zoom->area.w, dst_node->zoom->area.h });
        } else {
            window_manager_animate_window((struct window_capture) { src_window, dst_node->area.x, dst_node->area.y, dst_node->area.w, dst_node->area.h });
        }
    }
}

void mouse_drop_action_swap(struct window_manager *wm, struct view *src_view, struct window_node *src_node, struct window *src_window, struct view *dst_view, struct window_node *dst_node, struct window *dst_window)
{
    if (window_node_contains_window(src_node, src_view->insertion_point)) {
        src_view->insertion_point = dst_window->id;
    } else if (window_node_contains_window(dst_node, dst_view->insertion_point)) {
        dst_view->insertion_point = src_window->id;
    }

    window_node_swap_window_list(src_node, dst_node);

    if (src_view->sid != dst_view->sid) {
        for (int i = 0; i < src_node->window_count; ++i) {
            window_manager_remove_managed_window(wm, src_node->window_list[i]);
            window_manager_add_managed_window(wm, window_manager_find_window(wm, src_node->window_list[i]), src_view);
        }

        for (int i = 0; i < dst_node->window_count; ++i) {
            window_manager_remove_managed_window(wm, dst_node->window_list[i]);
            window_manager_add_managed_window(wm, window_manager_find_window(wm, dst_node->window_list[i]), dst_view);
        }
    }

    struct window_capture *window_list = NULL;
    window_node_capture_windows(src_node, &window_list);
    window_node_capture_windows(dst_node, &window_list);
    window_manager_animate_window_list(window_list, ts_buf_len(window_list));
}

void mouse_drop_action_warp(struct space_manager *sm, struct window_manager *wm, struct view *src_view, struct window_node *src_node, struct window *src_window, struct view *dst_view, struct window_node *dst_node, struct window *dst_window, enum window_node_split split, enum window_node_child child)
{
    if ((src_node->parent && dst_node->parent) &&
        (src_node->parent == dst_node->parent) &&
        (src_node->window_count == 1)) {
        if (dst_node->parent->split == split) {
            mouse_drop_action_swap(wm, src_view, src_node, src_window, dst_view, dst_node, dst_window);
            return;
        } else {
            dst_node->parent->split = split;
            dst_node->parent->child = child;
        }
    } else {
        dst_node->split = split;
        dst_node->child = child;
    }

    struct window_node *src_node_rm = view_remove_window_node(src_view, src_window);
    window_manager_remove_managed_window(wm, src_window->id);
    window_manager_purify_window(wm, src_window);

    struct window_node *src_node_add = view_add_window_node_with_insertion_point(dst_view, src_window, dst_window->id);
    window_manager_add_managed_window(wm, src_window, dst_view);

    struct window_capture *window_list = NULL;

    if (src_node_rm) {
        window_node_capture_windows(src_node_rm, &window_list);
    }

    if (src_node_rm != src_node_add && src_node_rm != src_node_add->parent) {
        window_node_capture_windows(src_node_add, &window_list);
    }

    window_manager_animate_window_list(window_list, ts_buf_len(window_list));
}

void mouse_drop_no_target(struct space_manager *sm, struct window_manager *wm, struct view *src_view, struct view *dst_view, struct window *window, struct window_node *node)
{
    if (src_view->sid == dst_view->sid) {
        window_node_flush(node);
    } else {
        space_manager_untile_window(sm, src_view, window);
        window_manager_remove_managed_window(wm, window->id);
        window_manager_purify_window(wm, window);

        struct view *view = space_manager_tile_window_on_space(sm, window, dst_view->sid);
        window_manager_add_managed_window(wm, window, view);
    }
}

void mouse_drop_try_adjust_bsp_grid(struct window_manager *wm, struct view *view, struct window *window, struct mouse_window_info *info)
{
    bool success = true;

    if (view->layout != VIEW_BSP) {
        success = false;
        goto end;
    }

    if (info->changed_position) {
        uint8_t direction = 0;
        if (info->changed_x) direction |= HANDLE_LEFT;
        if (info->changed_y) direction |= HANDLE_TOP;
        if (window_manager_resize_window_relative(wm, window, direction, info->dx, info->dy, true) == WINDOW_OP_ERROR_INVALID_DST_NODE) {
            success = false;
        }
    }

    if (info->changed_size) {
        uint8_t direction = 0;
        if (info->changed_w && !info->changed_x) direction |= HANDLE_RIGHT;
        if (info->changed_h && !info->changed_y) direction |= HANDLE_BOTTOM;
        if (window_manager_resize_window_relative(wm, window, direction, info->dw, info->dh, true) == WINDOW_OP_ERROR_INVALID_DST_NODE) {
            success = false;
        }
    }

end:
    if (!success) {
        struct window_node *node = view_find_window_node(view, window->id);
        if (node) window_node_flush(node);
    }
}

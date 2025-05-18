extern struct window_manager g_window_manager;
extern int g_connection;

static TABLE_HASH_FUNC(hash_view)
{
    return *(uint64_t *) key;
}

static TABLE_COMPARE_FUNC(compare_view)
{
    return *(uint64_t *) key_a == *(uint64_t *) key_b;
}

bool space_manager_query_space(FILE *rsp, uint64_t sid, uint64_t flags)
{
    TIME_FUNCTION;

    struct view *view = space_manager_query_view(&g_space_manager, sid);
    if (!view) return false;

    view_serialize(rsp, view, flags);
    fprintf(rsp, "\n");
    return true;
}

bool space_manager_query_spaces_for_window(FILE *rsp, struct window *window, uint64_t flags)
{
    TIME_FUNCTION;

    int space_count;
    uint64_t *space_list = window_space_list(window->id, &space_count);
    if (!space_list) return false;

    fprintf(rsp, "[");
    for (int i = 0; i < space_count; ++i) {
        struct view *view = space_manager_query_view(&g_space_manager, space_list[i]);
        if (!view) continue;

        view_serialize(rsp, view, flags);
        fprintf(rsp, "%c", i < space_count - 1 ? ',' : ']');
    }
    fprintf(rsp, "\n");

    return true;
}

bool space_manager_query_spaces_for_display(FILE *rsp, uint32_t did, uint64_t flags)
{
    TIME_FUNCTION;

    int space_count;
    uint64_t *space_list = display_space_list(did, &space_count);
    if (!space_list) return false;

    fprintf(rsp, "[");
    for (int i = 0; i < space_count; ++i) {
        struct view *view = space_manager_query_view(&g_space_manager, space_list[i]);
        if (!view) continue;

        view_serialize(rsp, view, flags);
        fprintf(rsp, "%c", i < space_count - 1 ? ',' : ']');
    }
    fprintf(rsp, "\n");

    return true;
}

bool space_manager_query_spaces_for_displays(FILE *rsp, uint64_t flags)
{
    TIME_FUNCTION;

    int display_count;
    uint32_t *display_list = display_manager_active_display_list(&display_count);
    if (!display_list) return false;

    fprintf(rsp, "[");
    for (int i = 0; i < display_count; ++i) {
        int space_count;
        uint64_t *space_list = display_space_list(display_list[i], &space_count);
        if (!space_list) continue;

        for (int j = 0; j < space_count; ++j) {
            struct view *view = space_manager_query_view(&g_space_manager, space_list[j]);
            if (!view) continue;

            view_serialize(rsp, view, flags);
            if (j < space_count - 1) fprintf(rsp, ",");
        }

        fprintf(rsp, "%c", i < display_count - 1 ? ',' : ']');
    }
    fprintf(rsp, "\n");

    return true;
}

struct view *space_manager_query_view(struct space_manager *sm, uint64_t sid)
{
    if (sm->did_begin) return space_manager_find_view(sm, sid);
    return table_find(&sm->view, &sid);
}

struct view *space_manager_find_view(struct space_manager *sm, uint64_t sid)
{
    struct view *view = table_find(&sm->view, &sid);
    if (!view) {
        view = view_create(sid);
        table_add(&sm->view, &sid, view);
    }
    return view;
}

void space_manager_refresh_view(struct space_manager *sm, uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout == VIEW_FLOAT) return;

    view_update(view);
    view_flush(view);
}

void space_manager_mark_view_invalid(struct space_manager *sm,  uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout == VIEW_FLOAT) return;

    view_clear_flag(view, VIEW_IS_VALID);
}

void space_manager_untile_window(struct view *view, struct window *window)
{
    if (view->layout == VIEW_FLOAT) return;

    window_manager_adjust_layer(window, LAYER_NORMAL);
    struct window_node *node = view_remove_window_node(view, window);
    if (!node) return;

    if (space_is_visible(view->sid)) {
        window_node_flush(node);
    } else {
        view_set_flag(view, VIEW_IS_DIRTY);
    }
}

struct space_label *space_manager_get_label_for_space(struct space_manager *sm, uint64_t sid)
{
    for (int i = 0; i < buf_len(sm->labels); ++i) {
        struct space_label *space_label = &sm->labels[i];
        if (space_label->sid == sid) {
            return space_label;
        }
    }

    return NULL;
}

struct space_label *space_manager_get_space_for_label(struct space_manager *sm, char *label)
{
    for (int i = 0; i < buf_len(sm->labels); ++i) {
        struct space_label *space_label = &sm->labels[i];
        if (string_equals(label, space_label->label)) {
            return space_label;
        }
    }

    return NULL;
}

bool space_manager_remove_label_for_space(struct space_manager *sm, uint64_t sid)
{
    for (int i = 0; i < buf_len(sm->labels); ++i) {
        struct space_label *space_label = &sm->labels[i];
        if (space_label->sid == sid) {
            free(space_label->label);
            buf_del(sm->labels, i);
            return true;
        }
    }

    return false;
}

void space_manager_set_label_for_space(struct space_manager *sm, uint64_t sid, char *label)
{
    space_manager_remove_label_for_space(sm, sid);

    for (int i = 0; i < buf_len(sm->labels); ++i) {
        struct space_label *space_label = &sm->labels[i];
        if (string_equals(space_label->label, label)) {
            free(space_label->label);
            buf_del(sm->labels, i);
            break;
        }
    }

    buf_push(sm->labels, ((struct space_label) {
        .sid   = sid,
        .label = label
    }));
}

void space_manager_set_layout_for_space(struct space_manager *sm, uint64_t sid, enum view_type layout)
{
    struct view *view = space_manager_find_view(sm, sid);
    view->layout = layout;
    view_clear(view);

    if (view->layout != VIEW_FLOAT) {
        window_manager_validate_and_check_for_windows_on_space(sm, &g_window_manager, sid);
    }
}

bool space_manager_set_gap_for_space(struct space_manager *sm, uint64_t sid, int type, int gap)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout == VIEW_FLOAT) return false;

    if (type == TYPE_ABS) {
        view->window_gap = gap;
    } else if (type == TYPE_REL) {
        view->window_gap = add_and_clamp_to_zero(view->window_gap, gap);
    }

    view_update(view);
    view_flush(view);

    return true;
}

bool space_manager_toggle_gap_for_space(struct space_manager *sm, uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout == VIEW_FLOAT) return false;

    if (view_check_flag(view, VIEW_ENABLE_GAP)) {
        view_clear_flag(view, VIEW_ENABLE_GAP);
    } else {
        view_set_flag(view, VIEW_ENABLE_GAP);
    }

    view_update(view);
    view_flush(view);

    return true;
}

void space_manager_toggle_mission_control(uint64_t sid)
{
    space_manager_focus_space(sid);
    CoreDockSendNotification(CFSTR("com.apple.expose.awake"), 0);
}

void space_manager_toggle_show_desktop(uint64_t sid)
{
    space_manager_focus_space(sid);
    CoreDockSendNotification(CFSTR("com.apple.showdesktop.awake"), 0);
}

void space_manager_set_layout_for_all_spaces(struct space_manager *sm, enum view_type layout)
{
    sm->layout = layout;
    table_for (struct view *view, sm->view, {
        if (!view_check_flag(view, VIEW_LAYOUT)) {
            if (space_is_user(view->sid)) {
                view->layout = layout;
                view_clear(view);

                if (view->layout != VIEW_FLOAT) {
                    window_manager_validate_and_check_for_windows_on_space(sm, &g_window_manager, view->sid);
                }
            }
        }
    })
}

void space_manager_set_window_gap_for_all_spaces(struct space_manager *sm, int window_gap)
{
    sm->window_gap = window_gap;
    table_for (struct view *view, sm->view, {
        if (!view_check_flag(view, VIEW_WINDOW_GAP)) {
            view->window_gap = window_gap;
            view_update(view);
            view_flush(view);
        }
    })
}

void space_manager_set_top_padding_for_all_spaces(struct space_manager *sm, int top_padding)
{
    sm->top_padding = top_padding;
    table_for (struct view *view, sm->view, {
        if (!view_check_flag(view, VIEW_TOP_PADDING)) {
            view->top_padding = top_padding;
            view_update(view);
            view_flush(view);
        }
    })
}

void space_manager_set_bottom_padding_for_all_spaces(struct space_manager *sm, int bottom_padding)
{
    sm->bottom_padding = bottom_padding;
    table_for (struct view *view, sm->view, {
        if (!view_check_flag(view, VIEW_BOTTOM_PADDING)) {
            view->bottom_padding = bottom_padding;
            view_update(view);
            view_flush(view);
        }
    })
}

void space_manager_set_left_padding_for_all_spaces(struct space_manager *sm, int left_padding)
{
    sm->left_padding = left_padding;
    table_for (struct view *view, sm->view, {
        if (!view_check_flag(view, VIEW_LEFT_PADDING)) {
            view->left_padding = left_padding;
            view_update(view);
            view_flush(view);
        }
    })
}

void space_manager_set_right_padding_for_all_spaces(struct space_manager *sm, int right_padding)
{
    sm->right_padding = right_padding;
    table_for (struct view *view, sm->view, {
        if (!view_check_flag(view, VIEW_RIGHT_PADDING)) {
            view->right_padding = right_padding;
            view_update(view);
            view_flush(view);
        }
    })
}

void space_manager_set_split_type_for_all_spaces(struct space_manager *sm, enum window_node_split split_type)
{
    sm->split_type = split_type;
    table_for (struct view *view, sm->view, {
        if (!view_check_flag(view, VIEW_SPLIT_TYPE)) {
            view->split_type = split_type;
        }
    })
}

void space_manager_set_auto_balance_for_all_spaces(struct space_manager *sm, uint32_t auto_balance)
{
    sm->auto_balance = auto_balance;
    table_for (struct view *view, sm->view, {
        if (!view_check_flag(view, VIEW_AUTO_BALANCE)) {
            view->auto_balance = auto_balance;
        }
    })
}

bool space_manager_set_padding_for_space(struct space_manager *sm, uint64_t sid, int type, int top, int bottom, int left, int right)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout == VIEW_FLOAT) return false;

    if (type == TYPE_ABS) {
        view->top_padding    = top;
        view->bottom_padding = bottom;
        view->left_padding   = left;
        view->right_padding  = right;
    } else if (type == TYPE_REL) {
        view->top_padding    = add_and_clamp_to_zero(view->top_padding, top);
        view->bottom_padding = add_and_clamp_to_zero(view->bottom_padding, bottom);
        view->left_padding   = add_and_clamp_to_zero(view->left_padding, left);
        view->right_padding  = add_and_clamp_to_zero(view->right_padding, right);
    }

    view_update(view);
    view_flush(view);

    return true;
}

bool space_manager_toggle_padding_for_space(struct space_manager *sm, uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout == VIEW_FLOAT) return false;

    if (view_check_flag(view, VIEW_ENABLE_PADDING)) {
        view_clear_flag(view, VIEW_ENABLE_PADDING);
    } else {
        view_set_flag(view, VIEW_ENABLE_PADDING);
    }

    view_update(view);
    view_flush(view);

    return true;
}

bool space_manager_rotate_space(struct space_manager *sm, uint64_t sid, int degrees)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return false;

    window_node_rotate(view->root, degrees);
    view_update(view);
    view_flush(view);

    return true;
}

bool space_manager_mirror_space(struct space_manager *sm, uint64_t sid, enum window_node_split axis)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return false;

    window_node_mirror(view->root, axis);
    view_update(view);
    view_flush(view);

    return true;
}

bool space_manager_equalize_space(struct space_manager *sm, uint64_t sid, uint32_t axis_flag)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return false;

    window_node_equalize(view->root, axis_flag);
    view_update(view);
    view_flush(view);

    return true;
}

bool space_manager_balance_space(struct space_manager *sm, uint64_t sid, uint32_t axis_flag)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return false;

    window_node_balance(view->root, axis_flag);
    view_update(view);
    view_flush(view);

    return true;
}

struct view *space_manager_tile_window_on_space_with_insertion_point(struct space_manager *sm, struct window *window, uint64_t sid, uint32_t insertion_point)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout == VIEW_FLOAT) return view;

    window_manager_adjust_layer(window, LAYER_BELOW);
    struct window_node *node = view_add_window_node_with_insertion_point(view, window, insertion_point);
    assert(node);

    if (space_is_visible(view->sid)) {
        window_node_flush(node);
    } else {
        view_set_flag(view, VIEW_IS_DIRTY);
    }

    return view;
}

struct view *space_manager_tile_window_on_space(struct space_manager *sm, struct window *window, uint64_t sid)
{
    return space_manager_tile_window_on_space_with_insertion_point(sm, window, sid, 0);
}

void space_manager_toggle_window_split(struct space_manager *sm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, window_space(window->id));
    if (view->layout != VIEW_BSP) return;

    struct window_node *node = view_find_window_node(view, window->id);
    if (node && window_node_is_intermediate(node)) {
        node->parent->split = node->parent->split == SPLIT_Y ? SPLIT_X : SPLIT_Y;

        if (view->auto_balance != SPLIT_NONE) {
            window_node_balance(view->root, view->auto_balance);
            view_update(view);
            view_flush(view);
        } else {
            window_node_update(view, node->parent);
            if (space_is_visible(view->sid)) {
                window_node_flush(node->parent);
            } else {
                view_set_flag(view, VIEW_IS_DIRTY);
            }
        }
    }
}

int space_manager_mission_control_index(uint64_t sid)
{
    uint64_t result = 0;
    int desktop_cnt = 1;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    int display_spaces_count = CFArrayGetCount(display_spaces_ref);

    for (int i = 0; i < display_spaces_count; ++i) {
        CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
        CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
        int spaces_count = CFArrayGetCount(spaces_ref);

        for (int j = 0; j < spaces_count; ++j) {
            CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
            CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
            CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &result);
            if (sid == result) goto out;

            ++desktop_cnt;
        }
    }

    desktop_cnt = 0;
out:
    CFRelease(display_spaces_ref);
    return desktop_cnt;
}

uint64_t space_manager_mission_control_space(int desktop_id)
{
    uint64_t result = 0;
    int desktop_cnt = 1;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    int display_spaces_count = CFArrayGetCount(display_spaces_ref);

    for (int i = 0; i < display_spaces_count; ++i) {
        CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
        CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
        int spaces_count = CFArrayGetCount(spaces_ref);

        for (int j = 0; j < spaces_count; ++j) {
            CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
            CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
            CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &result);
            if (desktop_cnt == desktop_id) goto out;

            ++desktop_cnt;
        }
    }

    result = 0;
out:
    CFRelease(display_spaces_ref);
    return result;
}

uint64_t space_manager_cursor_space(void)
{
    uint32_t did = display_manager_cursor_display_id();
    return display_space_id(did);
}

uint64_t space_manager_prev_space(uint64_t sid)
{
    uint64_t p_sid = 0;
    uint64_t n_sid = 0;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    int display_spaces_count = CFArrayGetCount(display_spaces_ref);

    for (int i = 0; i < display_spaces_count; ++i) {
        CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
        CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
        int spaces_count = CFArrayGetCount(spaces_ref);

        for (int j = 0; j < spaces_count; ++j) {
            CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
            CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
            CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &n_sid);
            if (n_sid == sid) goto out;

            p_sid = n_sid;
        }
    }

out:
    CFRelease(display_spaces_ref);
    return p_sid != sid ? p_sid : 0;
}

uint64_t space_manager_next_space(uint64_t sid)
{
    uint64_t n_sid = 0;
    bool found_sid = false;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    int display_spaces_count = CFArrayGetCount(display_spaces_ref);

    for (int i = 0; i < display_spaces_count; ++i) {
        CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
        CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
        int spaces_count = CFArrayGetCount(spaces_ref);

        for (int j = 0; j < spaces_count; ++j) {
            CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
            CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
            CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &n_sid);
            if (found_sid) goto out;

            found_sid = n_sid == sid;
        }
    }

out:
    CFRelease(display_spaces_ref);
    return n_sid != sid ? n_sid : 0;
}

uint64_t space_manager_grid_space(uint64_t sid, char *direction)
{
    uint64_t n_sid = 0;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    int display_spaces_count = CFArrayGetCount(display_spaces_ref);

    for (int i = 0; i < display_spaces_count; ++i) {
        CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
        CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
        uint64_t spaces_count = CFArrayGetCount(spaces_ref);

        uint64_t grid_columns = INT_MAX;
        if (NULL != g_space_manager.grid_columns
            && i < CFArrayGetCount(g_space_manager.grid_columns)) {
            int value = CFStringGetIntValue(CFArrayGetValueAtIndex(g_space_manager.grid_columns, i));
            if (value > 0) {
                grid_columns = value;
            }
        }

        for (uint64_t j = 0; j < spaces_count; ++j) {
            CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
            CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
            CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &n_sid);
            if (n_sid == sid) {
              uint64_t target_space_index = j;
              uint64_t focused_column = j%grid_columns;

              if (string_equals(direction, ARGUMENT_COMMON_SEL_NORTH)) {
                target_space_index -= grid_columns;
              } else if (string_equals(direction, ARGUMENT_COMMON_SEL_EAST)) {
                if (focused_column < grid_columns - 1) {
                  target_space_index += 1;
                }
              } else if (string_equals(direction, ARGUMENT_COMMON_SEL_SOUTH)) {
                target_space_index += grid_columns;
              } else if (string_equals(direction, ARGUMENT_COMMON_SEL_WEST)) {
                if (0 < focused_column) {
                  target_space_index -= 1;
                }
              }

              if (0 <= target_space_index && target_space_index < spaces_count) {
                CFDictionaryRef target_space_ref = CFArrayGetValueAtIndex(spaces_ref, target_space_index);
                CFNumberRef target_space_id_ref = CFDictionaryGetValue(target_space_ref, CFSTR("id64"));
                CFNumberGetValue(target_space_id_ref, CFNumberGetType(target_space_id_ref), &n_sid);
              }

              goto out;
            }
        }
    }

out:
    CFRelease(display_spaces_ref);
    return n_sid != sid ? n_sid : 0;
}

uint64_t space_manager_first_space(void)
{
    uint64_t sid = 0;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, 0);
    CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));

    CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, 0);
    CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
    CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &sid);

    CFRelease(display_spaces_ref);
    return sid;
}

uint64_t space_manager_last_space(void)
{
    uint64_t sid = 0;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    int display_spaces_count = CFArrayGetCount(display_spaces_ref);

    CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, display_spaces_count-1);
    CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
    int spaces_count = CFArrayGetCount(spaces_ref);

    CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, spaces_count-1);
    CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
    CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &sid);

    CFRelease(display_spaces_ref);
    return sid;
}

uint64_t space_manager_active_space(void)
{
    uint32_t did = 0;
    struct window *window = window_manager_focused_window(&g_window_manager);

    if (window) did = window_display_id(window->id);
    if (!did)   did = display_manager_active_display_id();
    if (!did)   return 0;

    return display_space_id(did);
}

void space_manager_move_window_list_to_space(uint64_t sid, uint32_t *window_list, int window_count)
{
    if (!workspace_use_macos_space_workaround()) {
        CFArrayRef window_list_ref = cfarray_of_cfnumbers(window_list, sizeof(uint32_t), window_count, kCFNumberSInt32Type);
        SLSMoveWindowsToManagedSpace(g_connection, window_list_ref, sid);
        CFRelease(window_list_ref);
    } else if (!scripting_addition_move_window_list_to_space(sid, window_list, window_count)) {
        SLSSpaceSetCompatID(g_connection, sid, 0x79616265);
        SLSSetWindowListWorkspace(g_connection, window_list, window_count, 0x79616265);
        SLSSpaceSetCompatID(g_connection, sid, 0x0);
    }
}

void space_manager_move_window_to_space(uint64_t sid, struct window *window)
{
    if (!workspace_use_macos_space_workaround()) {
        CFArrayRef window_list_ref = cfarray_of_cfnumbers(&window->id, sizeof(uint32_t), 1, kCFNumberSInt32Type);
        SLSMoveWindowsToManagedSpace(g_connection, window_list_ref, sid);
        CFRelease(window_list_ref);
    } else if (!scripting_addition_move_window_to_space(sid, window->id)) {
        SLSSpaceSetCompatID(g_connection, sid, 0x79616265);
        SLSSetWindowListWorkspace(g_connection, &window->id, 1, 0x79616265);
        SLSSpaceSetCompatID(g_connection, sid, 0x0);
    }
}

static inline uint64_t space_manager_find_first_user_space_for_display(uint32_t did)
{
    int count;
    uint64_t *space_list = display_space_list(did, &count);
    if (!space_list) return 0;

    for (int i = 0; i < count; ++i) {
        uint64_t sid = space_list[i];

        if (space_is_user(sid)) {
            return sid;
        }
    }

    return 0;
}

static inline bool space_manager_is_space_last_user_space(uint64_t sid)
{
    bool result = true;

    int count;
    uint64_t *space_list = display_space_list(space_display_id(sid), &count);
    if (!space_list) return true;

    for (int i = 0; i < count; ++i) {
        uint64_t c_sid = space_list[i];
        if (sid == c_sid) continue;

        if (space_is_user(c_sid)) {
            result = false;
            break;
        }
    }

    return result;
}

static enum space_op_error space_manager_swap_space_with_space_on_display(uint32_t a_did, uint64_t a_sid, uint32_t b_did, uint64_t b_sid)
{
    if (display_manager_display_is_animating(a_did)) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;
    if (display_manager_display_is_animating(b_did)) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;

    float window_animation_duration = g_window_manager.window_animation_duration;
    g_window_manager.window_animation_duration = 0.0f;
    __asm__ __volatile__ ("" ::: "memory");

    int a_window_list_count = 0;
    uint32_t *a_window_list = space_window_list(a_sid, &a_window_list_count, true);

    int b_window_list_count = 0;
    uint32_t *b_window_list = space_window_list(b_sid, &b_window_list_count, true);

    struct view *a_view = table_find(&g_space_manager.view, &a_sid);
    struct view *b_view = table_find(&g_space_manager.view, &b_sid);

    table_remove(&g_space_manager.view, &a_sid);
    table_remove(&g_space_manager.view, &b_sid);

    a_view->sid = b_sid;
    b_view->sid = a_sid;

    CFStringRef tmp = a_view->uuid;
    a_view->uuid    = b_view->uuid;
    b_view->uuid    = tmp;

    table_add(&g_space_manager.view, &a_sid, b_view);
    table_add(&g_space_manager.view, &b_sid, a_view);

    if (a_window_list_count) {
        space_manager_move_window_list_to_space(b_sid, a_window_list, a_window_list_count);
    }

    if (b_window_list_count) {
        space_manager_move_window_list_to_space(a_sid, b_window_list, b_window_list_count);
    }

    for (int i = 0; i < buf_len(g_space_manager.labels); ++i) {
        struct space_label *label = &g_space_manager.labels[i];
        if      (label->sid == a_sid) label->sid = b_sid;
        else if (label->sid == b_sid) label->sid = a_sid;
    }

    view_update(a_view);
    view_update(b_view);

    view_flush(a_view);
    view_flush(b_view);

    __asm__ __volatile__ ("" ::: "memory");
    g_window_manager.window_animation_duration = window_animation_duration;
    return SPACE_OP_ERROR_SUCCESS;
}

enum space_op_error space_manager_swap_space_with_space(uint64_t acting_sid, uint64_t selector_sid)
{
    bool is_in_mc = mission_control_is_active();
    if (is_in_mc) return SPACE_OP_ERROR_IN_MISSION_CONTROL;

    uint32_t acting_did = space_display_id(acting_sid);
    uint32_t selector_did = space_display_id(selector_sid);

    if (acting_sid == selector_sid) return SPACE_OP_ERROR_SAME_SPACE;
    if (acting_did != selector_did) return space_manager_swap_space_with_space_on_display(acting_did, acting_sid, selector_did, selector_sid);

    bool is_animating = display_manager_display_is_animating(acting_did);
    if (is_animating) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;

    uint64_t acting_prev_sid = space_manager_prev_space(acting_sid);
    uint64_t selector_prev_sid = space_manager_prev_space(selector_sid);

    uint32_t acting_prev_did = acting_prev_sid ? space_display_id(acting_prev_sid) : 0;
    uint32_t selector_prev_did = selector_prev_sid ? space_display_id(selector_prev_sid) : 0;

    bool acting_sid_is_first = !acting_prev_sid || acting_prev_did != acting_did;
    bool selector_sid_is_first = !selector_prev_sid || selector_prev_did != selector_did;

    int acting_mci = space_manager_mission_control_index(acting_sid);
    int selector_mci = space_manager_mission_control_index(selector_sid);
    bool success = true;

    if (acting_sid_is_first && !selector_sid_is_first && selector_mci - acting_mci == 1) {
        success = scripting_addition_move_space_after_space(acting_sid, selector_sid, acting_sid == space_manager_active_space());
    } else if (!acting_sid_is_first && selector_sid_is_first && acting_mci - selector_mci == 1) {
        success = scripting_addition_move_space_after_space(selector_sid, acting_sid, selector_sid == space_manager_active_space());
    } else if (acting_sid_is_first && !selector_sid_is_first) {
        success  = scripting_addition_move_space_after_space(selector_sid, acting_sid, false);
        success &= scripting_addition_move_space_after_space(acting_sid, selector_prev_sid, acting_sid == space_manager_active_space());
    } else if (!acting_sid_is_first && selector_sid_is_first) {
        success  = scripting_addition_move_space_after_space(acting_sid, selector_sid, acting_sid == space_manager_active_space());
        success &= scripting_addition_move_space_after_space(selector_sid, acting_prev_sid, false);
    } else if (!acting_sid_is_first && !selector_sid_is_first) {
        if (acting_mci > selector_mci) {
            success  = scripting_addition_move_space_after_space(selector_sid, acting_sid, false);
            success &= scripting_addition_move_space_after_space(acting_sid, selector_prev_sid, acting_sid == space_manager_active_space());
        } else {
            success  = scripting_addition_move_space_after_space(acting_sid, selector_sid, acting_sid == space_manager_active_space());
            success &= scripting_addition_move_space_after_space(selector_sid, acting_prev_sid, false);
        }
    }

    return success ? SPACE_OP_ERROR_SUCCESS : SPACE_OP_ERROR_SCRIPTING_ADDITION;
}

enum space_op_error space_manager_move_space_to_space(uint64_t acting_sid, uint64_t selector_sid)
{
    bool is_in_mc = mission_control_is_active();
    if (is_in_mc) return SPACE_OP_ERROR_IN_MISSION_CONTROL;

    uint32_t acting_did = space_display_id(acting_sid);
    uint32_t selector_did = space_display_id(selector_sid);

    if (acting_sid == selector_sid) return SPACE_OP_ERROR_SAME_SPACE;
    if (acting_did != selector_did) return SPACE_OP_ERROR_SAME_DISPLAY;

    bool is_animating = display_manager_display_is_animating(acting_did);
    if (is_animating) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;

    uint64_t acting_prev_sid = space_manager_prev_space(acting_sid);
    uint64_t selector_prev_sid = space_manager_prev_space(selector_sid);

    uint32_t acting_prev_did = acting_prev_sid ? space_display_id(acting_prev_sid) : 0;
    uint32_t selector_prev_did = selector_prev_sid ? space_display_id(selector_prev_sid) : 0;

    bool acting_sid_is_first = !acting_prev_sid || acting_prev_did != acting_did;
    bool selector_sid_is_first = !selector_prev_sid || selector_prev_did != selector_did;
    bool success = true;

    if (acting_sid_is_first && !selector_sid_is_first) {
        success = scripting_addition_move_space_after_space(acting_sid, selector_sid, acting_sid == space_manager_active_space());
    } else if (!acting_sid_is_first && selector_sid_is_first) {
        success  = scripting_addition_move_space_after_space(acting_sid, selector_sid, acting_sid == space_manager_active_space());
        success &= scripting_addition_move_space_after_space(selector_sid, acting_sid, false);
    } else if (!acting_sid_is_first && !selector_sid_is_first) {
        if (space_manager_mission_control_index(acting_sid) > space_manager_mission_control_index(selector_sid)) {
            success = scripting_addition_move_space_after_space(acting_sid, selector_prev_sid, acting_sid == space_manager_active_space());
        } else {
            success = scripting_addition_move_space_after_space(acting_sid, selector_sid, acting_sid == space_manager_active_space());
        }
    }

    return success ? SPACE_OP_ERROR_SUCCESS : SPACE_OP_ERROR_SCRIPTING_ADDITION;
}

enum space_op_error space_manager_move_space_to_display(struct space_manager *sm, uint64_t sid, uint32_t did)
{
    bool is_in_mc = mission_control_is_active();
    if (is_in_mc) return SPACE_OP_ERROR_IN_MISSION_CONTROL;
    if (!sid)     return SPACE_OP_ERROR_MISSING_SRC;

    uint32_t s_did = space_display_id(sid);
    if (s_did == did) return SPACE_OP_ERROR_INVALID_DST;

    bool is_src_animating = display_manager_display_is_animating(s_did);
    if (is_src_animating) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;

    bool last_space = space_manager_is_space_last_user_space(sid);
    if (last_space) return SPACE_OP_ERROR_INVALID_SRC;

    bool is_dst_animating = display_manager_display_is_animating(did);
    if (is_dst_animating) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;

    uint64_t d_sid = display_space_id(did);
    if (!d_sid) return SPACE_OP_ERROR_MISSING_DST;

    bool focus_space = sid == space_manager_active_space();

    if (scripting_addition_move_space_to_display(sid, d_sid,  focus_space ? space_manager_prev_space(sid) : 0, focus_space ? 1 : 0)) {
        space_manager_mark_view_invalid(sm, sid);
        if (focus_space) {
            space_manager_focus_space(sid);
        }
        return SPACE_OP_ERROR_SUCCESS;
    }

    return SPACE_OP_ERROR_SCRIPTING_ADDITION;
}

enum space_op_error space_manager_focus_space(uint64_t sid)
{
    bool is_in_mc = mission_control_is_active();
    if (is_in_mc) return SPACE_OP_ERROR_IN_MISSION_CONTROL;

    uint64_t cur_sid = space_manager_active_space();
    if (cur_sid == sid) return SPACE_OP_ERROR_SAME_SPACE;

    uint32_t cur_did = space_display_id(cur_sid);
    uint32_t new_did = space_display_id(sid);
    bool focus_display = cur_did != new_did;

    bool is_animating = display_manager_display_is_animating(new_did);
    if (is_animating) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;

    if (scripting_addition_focus_space(sid)) {
        if (focus_display) {
            display_manager_focus_display(new_did, sid);
        }
    } else {
        return SPACE_OP_ERROR_SCRIPTING_ADDITION;
    }

    return SPACE_OP_ERROR_SUCCESS;
}

enum space_op_error space_manager_switch_space(uint64_t sid)
{
    bool is_in_mc = mission_control_is_active();
    if (is_in_mc) return SPACE_OP_ERROR_IN_MISSION_CONTROL;

    uint64_t cur_sid = space_manager_active_space();
    if (cur_sid == sid) return SPACE_OP_ERROR_SAME_SPACE;

    uint32_t cur_did = space_display_id(cur_sid);
    uint32_t did     = space_display_id(sid);

    bool is_src_animating = display_manager_display_is_animating(cur_did);
    if (is_src_animating) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;

    bool is_dst_animating = display_manager_display_is_animating(did);
    if (is_dst_animating) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;

    if (cur_did != did) {
        space_manager_swap_space_with_space_on_display(cur_did, cur_sid, did, sid);
        display_manager_focus_display(cur_did, cur_sid);
        return SPACE_OP_ERROR_SUCCESS;
    }

    return scripting_addition_focus_space(sid) ? SPACE_OP_ERROR_SUCCESS : SPACE_OP_ERROR_SCRIPTING_ADDITION;
}

enum space_op_error space_manager_destroy_space(uint64_t sid)
{
    bool is_in_mc = mission_control_is_active();
    if (is_in_mc) return SPACE_OP_ERROR_IN_MISSION_CONTROL;

    if (!sid) return SPACE_OP_ERROR_MISSING_SRC;
    if (!space_is_user(sid)) return SPACE_OP_ERROR_INVALID_TYPE;
    if (space_manager_is_space_last_user_space(sid)) return SPACE_OP_ERROR_INVALID_SRC;

    uint32_t did = space_display_id(sid);
    uint64_t first_sid = space_manager_find_first_user_space_for_display(did);

    bool is_animating = display_manager_display_is_animating(did);
    if (is_animating) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;

    bool success = scripting_addition_destroy_space(sid);
    if (!success) return SPACE_OP_ERROR_SCRIPTING_ADDITION;

    if (first_sid) {
        window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, first_sid);
    }

    return SPACE_OP_ERROR_SUCCESS;
}

enum space_op_error space_manager_add_space(uint64_t sid)
{
    bool is_in_mc = mission_control_is_active();
    if (is_in_mc) return SPACE_OP_ERROR_IN_MISSION_CONTROL;
    if (!sid)     return SPACE_OP_ERROR_MISSING_SRC;

    bool is_animating = display_manager_display_is_animating(space_display_id(sid));
    if (is_animating) return SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;

    return scripting_addition_create_space(sid) ? SPACE_OP_ERROR_SUCCESS : SPACE_OP_ERROR_SCRIPTING_ADDITION;
}

void space_manager_assign_process_to_space(pid_t pid, uint64_t sid)
{
    SLSProcessAssignToSpace(g_connection, pid, sid);
}

void space_manager_assign_process_to_all_spaces(pid_t pid)
{
    SLSProcessAssignToAllSpaces(g_connection, pid);
}

bool space_manager_is_window_on_active_space(struct window *window)
{
    uint64_t sid = space_manager_active_space();
    bool result = space_manager_is_window_on_space(sid, window);
    return result;
}

bool space_manager_is_window_on_space(uint64_t sid, struct window *window)
{
    int space_count;
    uint64_t *space_list = window_space_list(window->id, &space_count);
    if (!space_list) return false;

    for (int i = 0; i < space_count; ++i) {
        if (sid == space_list[i]) {
            return true;
        }
    }

    return false;
}

void space_manager_mark_spaces_invalid_for_display(struct space_manager *sm, uint32_t did)
{
    int space_count;
    uint64_t *space_list = display_space_list(did, &space_count);
    if (!space_list) return;

    uint64_t sid = display_space_id(did);
    for (int i = 0; i < space_count; ++i) {
        if (space_list[i] == sid) {
            space_manager_refresh_view(sm, sid);
        } else {
            space_manager_mark_view_invalid(sm, space_list[i]);
        }
    }
}

void space_manager_mark_spaces_invalid(struct space_manager *sm)
{
    int display_count;
    uint32_t *display_list = display_manager_active_display_list(&display_count);
    if (!display_list) return;

    for (int i = 0; i < display_count; ++i) {
        space_manager_mark_spaces_invalid_for_display(sm, display_list[i]);
    }
}

bool space_manager_refresh_application_windows(struct space_manager *sm)
{
    int refresh_count = buf_len(g_window_manager.applications_to_refresh);
    if (!refresh_count) return false;
    int window_count = g_window_manager.window.count;
    for (int i = 0; i < refresh_count; ++i) {
        struct application *application = g_window_manager.applications_to_refresh[i];
        debug("%s: %s has windows that are not yet resolved\n", __FUNCTION__, application->name);
        bool result = window_manager_add_existing_application_windows(sm, &g_window_manager, application, i);
        if (result) {
            --refresh_count;
            --i;
        }
    }
    return window_count != g_window_manager.window.count;
}

void space_manager_handle_display_add(struct space_manager *sm, uint32_t did)
{
    int space_count;
    uint64_t *space_list = display_space_list(did, &space_count);
    if (!space_list) return;

    int list_count = 0;
    struct view *view_list[sm->view.count];
    CFStringRef uuid_list[sm->view.count];

    table_for (struct view *view, sm->view, {
        view_list[list_count] = view;
        uuid_list[list_count] = view->uuid;
        ++list_count;
    })

    for (int i = 0; i < space_count; ++i) {
        uint64_t sid = space_list[i];
        CFStringRef uuid = SLSSpaceCopyName(g_connection, sid);
        if (!uuid) continue;

        for (int j = 0; j < list_count; ++j) {
            CFStringRef view_uuid = uuid_list[j];
            if (!view_uuid) continue;

            if (CFEqual(view_uuid, uuid)) {
                struct view *view = view_list[j];

                uuid_list[j] = NULL;
                view_list[j] = NULL;

                table_remove(&sm->view, &view->sid);
                CFRelease(view->uuid);

                struct space_label *label = space_manager_get_label_for_space(sm, view->sid);
                if (label) label->sid = sid;

                view->sid = sid;
                view->uuid = CFRetain(uuid);

                table_add(&sm->view, &sid, view);
                break;
            }
        }

        CFRelease(uuid);
    }

    sm->current_space_id = space_manager_active_space();
    sm->last_space_id = sm->current_space_id;
}

void space_manager_begin(struct space_manager *sm)
{
    sm->layout = VIEW_FLOAT;
    sm->split_ratio = 0.5f;
    sm->auto_balance = SPLIT_NONE;
    sm->split_type = SPLIT_AUTO;
    sm->window_placement = CHILD_SECOND;
    sm->window_insertion_point = INSERT_FOCUSED;
    sm->window_zoom_persist = true;
    sm->labels = NULL;
    table_init(&sm->view, 23, hash_view, compare_view);

    int display_count;
    uint32_t *display_list = display_manager_active_display_list(&display_count);
    if (!display_list) return;

    sm->grid_columns = NULL;

    for (int i = 0; i < display_count; ++i) {
        int space_count;
        uint64_t *space_list = display_space_list(display_list[i], &space_count);
        if (!space_list) continue;

        for (int j = 0; j < space_count; ++j) {
            struct view *view = view_create(space_list[j]);
            table_add(&sm->view, &space_list[j], view);
        }
    }

    sm->current_space_id = space_manager_active_space();
    sm->last_space_id = sm->current_space_id;
    sm->did_begin = true;
}

#include "space_manager.h"

extern struct window_manager g_window_manager;
extern char g_sa_socket_file[MAXLEN];
extern int g_connection;

static TABLE_HASH_FUNC(hash_view)
{
    unsigned long result = *(uint64_t *) key;
    result = (result + 0x7ed55d16) + (result << 12);
    result = (result ^ 0xc761c23c) ^ (result >> 19);
    result = (result + 0x165667b1) + (result << 5);
    result = (result + 0xd3a2646c) ^ (result << 9);
    result = (result + 0xfd7046c5) + (result << 3);
    result = (result ^ 0xb55a4f09) ^ (result >> 16);
    return result;
}

static TABLE_COMPARE_FUNC(compare_view)
{
    return *(uint64_t *) key_a == *(uint64_t *) key_b;
}

bool space_manager_has_separate_spaces(void)
{
    return SLSGetSpaceManagementMode(g_connection) == 1;
}

bool space_manager_query_active_space(FILE *rsp)
{
    struct view *view = space_manager_query_view(&g_space_manager, space_manager_active_space());
    if (!view) return false;

    view_serialize(rsp, view);
    fprintf(rsp, "\n");
    return true;
}

bool space_manager_query_spaces_for_window(FILE *rsp, struct window *window)
{
    int space_count;
    uint64_t *space_list = window_space_list(window, &space_count);
    if (!space_list) return false;

    fprintf(rsp, "[");
    for (int i = 0; i < space_count; ++i) {
        struct view *view = space_manager_query_view(&g_space_manager, space_list[i]);
        if (!view) continue;

        view_serialize(rsp, view);
        fprintf(rsp, "%c", i < space_count - 1 ? ',' : ']');
    }
    fprintf(rsp, "\n");

    free(space_list);
    return true;
}

bool space_manager_query_spaces_for_display(FILE *rsp, uint32_t did)
{
    int space_count;
    uint64_t *space_list = display_space_list(did, &space_count);
    if (!space_list) return false;

    fprintf(rsp, "[");
    for (int i = 0; i < space_count; ++i) {
        struct view *view = space_manager_query_view(&g_space_manager, space_list[i]);
        if (!view) continue;

        view_serialize(rsp, view);
        fprintf(rsp, "%c", i < space_count - 1 ? ',' : ']');
    }
    fprintf(rsp, "\n");

    free(space_list);
    return true;
}

bool space_manager_query_spaces_for_displays(FILE *rsp)
{
    uint32_t display_count;
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

            view_serialize(rsp, view);
            if (j < space_count - 1) fprintf(rsp, ",");
        }

        free(space_list);
        fprintf(rsp, "%c", i < display_count - 1 ? ',' : ']');
    }
    fprintf(rsp, "\n");

    free(display_list);
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
    if (view->layout != VIEW_BSP) return;

    view_update(view);
    view_flush(view);
}

void space_manager_mark_view_invalid(struct space_manager *sm,  uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return;

    view->is_valid = false;
}

void space_manager_mark_view_dirty(struct space_manager *sm,  uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return;

    view->is_dirty = true;
}

void space_manager_untile_window(struct space_manager *sm, struct view *view, struct window *window)
{
    if (view->layout != VIEW_BSP) return;

    view_remove_window_node(view, window);
    view_flush(view);

    if (!space_is_visible(view->sid)) {
        view->is_dirty = true;
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

struct space_label *space_manager_get_space_for_label(struct space_manager *sm, struct token label)
{
    for (int i = 0; i < buf_len(sm->labels); ++i) {
        struct space_label *space_label = &sm->labels[i];
        if (token_equals(label, space_label->label)) {
            return space_label;
        }
    }

    return NULL;
}

void space_manager_set_label_for_space(struct space_manager *sm, uint64_t sid, char *label)
{
    for (int i = 0; i < buf_len(sm->labels); ++i) {
        struct space_label *space_label = &sm->labels[i];
        if (space_label->sid == sid) {
            free(space_label->label);
            buf_del(sm->labels, i);
            break;
        }
    }

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

    if (view->layout == VIEW_BSP) {
        window_manager_check_for_windows_on_space(sm, &g_window_manager, sid);
    } else if (view->layout == VIEW_FLOAT) {
        view_clear(view);
    }
}

void space_manager_set_gap_for_space(struct space_manager *sm, uint64_t sid, int type, int gap)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return;

    if (type == TYPE_ABS) {
        view->window_gap = gap;
    } else if (type == TYPE_REL) {
        view->window_gap = add_and_clamp_to_zero(view->window_gap, gap);
    }

    view_update(view);
    view_flush(view);
}

void space_manager_toggle_gap_for_space(struct space_manager *sm, uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return;

    view->enable_gap = !view->enable_gap;
    view_update(view);
    view_flush(view);
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
    for (int i = 0; i < sm->view.capacity; ++i) {
        struct bucket *bucket = sm->view.buckets[i];
        while (bucket) {
            if (bucket->value) {
                struct view *view = bucket->value;
                if (!view->custom_layout) {
                    if (space_is_user(view->sid)) {
                        view->layout = layout;
                        if (view->layout == VIEW_BSP) {
                            window_manager_check_for_windows_on_space(sm, &g_window_manager, view->sid);
                        } else if (view->layout == VIEW_FLOAT) {
                            view_clear(view);
                        }
                    }
                }
            }
            bucket = bucket->next;
        }
    }
}

#define VIEW_SET_PROPERTY(p) \
    sm->p = p; \
    for (int i = 0; i < sm->view.capacity; ++i) { \
        struct bucket *bucket = sm->view.buckets[i]; \
        while (bucket) { \
            if (bucket->value) { \
                struct view *view = bucket->value; \
                if (!view->custom_##p) view->p = p; \
                view_update(view); \
                view_flush(view); \
            } \
            bucket = bucket->next; \
        } \
    }

void space_manager_set_window_gap_for_all_spaces(struct space_manager *sm, int window_gap)
{
    VIEW_SET_PROPERTY(window_gap);
}

void space_manager_set_top_padding_for_all_spaces(struct space_manager *sm, int top_padding)
{
    VIEW_SET_PROPERTY(top_padding);
}

void space_manager_set_bottom_padding_for_all_spaces(struct space_manager *sm, int bottom_padding)
{
    VIEW_SET_PROPERTY(bottom_padding);
}

void space_manager_set_left_padding_for_all_spaces(struct space_manager *sm, int left_padding)
{
    VIEW_SET_PROPERTY(left_padding);
}

void space_manager_set_right_padding_for_all_spaces(struct space_manager *sm, int right_padding)
{
    VIEW_SET_PROPERTY(right_padding);
}

#undef VIEW_SET_PROPERTY

void space_manager_set_padding_for_space(struct space_manager *sm, uint64_t sid, int type, int top, int bottom, int left, int right)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return;

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
}

void space_manager_toggle_padding_for_space(struct space_manager *sm, uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return;

    view->enable_padding = !view->enable_padding;
    view_update(view);
    view_flush(view);
}

void space_manager_rotate_space(struct space_manager *sm, uint64_t sid, int degrees)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return;

    window_node_rotate(view->root, degrees);
    view_update(view);
    view_flush(view);
}

void space_manager_mirror_space(struct space_manager *sm, uint64_t sid, enum window_node_split axis)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return;

    window_node_mirror(view->root, axis);
    view_update(view);
    view_flush(view);
}

void space_manager_balance_space(struct space_manager *sm, uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return;

    window_node_equalize(view->root);
    view_update(view);
    view_flush(view);
}

struct view *space_manager_tile_window_on_space_with_insertion_point(struct space_manager *sm, struct window *window, uint64_t sid, uint32_t insertion_point)
{
    struct view *view = space_manager_find_view(sm, sid);
    if (view->layout != VIEW_BSP) return view;

    if (insertion_point) view->insertion_point = insertion_point;
    view_add_window_node(view, window);
    view_flush(view);

    if (!space_is_visible(view->sid)) {
        view->is_dirty = true;
    }

    return view;
}

struct view *space_manager_tile_window_on_space(struct space_manager *sm, struct window *window, uint64_t sid)
{
    return space_manager_tile_window_on_space_with_insertion_point(sm, window, sid, 0);
}

void space_manager_toggle_window_split(struct space_manager *sm, struct window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    if (view->layout != VIEW_BSP) return;

    struct window_node *node = view_find_window_node(view, window->id);
    if (node && window_node_is_intermediate(node)) {
        node->parent->split = node->parent->split == SPLIT_Y ? SPLIT_X : SPLIT_Y;

        if (g_space_manager.auto_balance) {
            window_node_equalize(view->root);
            view_update(view);
            view_flush(view);
        } else {
            window_node_update(view, node->parent);
            window_node_flush(node->parent);
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

    if (window) did = window_display_id(window);
    if (!did)   did = display_manager_active_display_id();
    if (!did)   return 0;

    return display_space_id(did);
}

static void space_manager_move_border_to_space(uint64_t sid, struct window *window)
{
    if (!window->border.id) return;
    CFNumberRef border_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &window->border.id);
    CFArrayRef window_list_ref = CFArrayCreate(NULL, (void *)&border_id_ref, 1, NULL);
    SLSMoveWindowsToManagedSpace(g_connection, window_list_ref, sid);
    CFRelease(window_list_ref);
    CFRelease(border_id_ref);
}

void space_manager_move_window_to_space(uint64_t sid, struct window *window)
{
    space_manager_move_border_to_space(sid, window);
    CFNumberRef window_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &window->id);
    CFArrayRef window_list_ref = CFArrayCreate(NULL, (void *)&window_id_ref, 1, NULL);
    SLSMoveWindowsToManagedSpace(g_connection, window_list_ref, sid);
    CFRelease(window_list_ref);
    CFRelease(window_id_ref);
}

void space_manager_remove_window_from_space(uint64_t sid, struct window *window)
{
    CFNumberRef window_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &window->id);
    CFArrayRef window_list_ref = CFArrayCreate(NULL, (void *)&window_id_ref, 1, NULL);
    CFNumberRef space_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &sid);
    CFArrayRef space_list_ref = CFArrayCreate(NULL, (void *)&space_id_ref, 1, NULL);
    SLSRemoveWindowsFromSpaces(g_connection, window_list_ref, space_list_ref);
    CFRelease(space_list_ref);
    CFRelease(space_id_ref);
    CFRelease(window_list_ref);
    CFRelease(window_id_ref);
}

void space_manager_add_window_to_space(uint64_t sid, struct window *window)
{
    CFNumberRef window_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &window->id);
    CFArrayRef window_list_ref = CFArrayCreate(NULL, (void *)&window_id_ref, 1, NULL);
    CFNumberRef space_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &sid);
    CFArrayRef space_list_ref = CFArrayCreate(NULL, (void *)&space_id_ref, 1, NULL);
    SLSAddWindowsToSpaces(g_connection, window_list_ref, space_list_ref);
    CFRelease(space_list_ref);
    CFRelease(space_id_ref);
    CFRelease(window_list_ref);
    CFRelease(window_id_ref);
}

void space_manager_focus_space(uint64_t sid)
{
    int sockfd;
    char message[MAXLEN];

    uint64_t cur_sid = space_manager_active_space();
    uint32_t cur_did = space_display_id(cur_sid);
    uint32_t new_did = space_display_id(sid);

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "space %lld", sid);
        socket_write(sockfd, message);
        socket_wait(sockfd);

        if (cur_did != new_did) {
            display_manager_focus_display(new_did);
        }
    }
    socket_close(sockfd);
}

void space_manager_move_space_after_space(uint64_t src_sid, uint64_t dst_sid, bool focus)
{
    int sockfd;
    char message[MAXLEN];

    if (!src_sid) return;
    if (!dst_sid) return;

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "space_move %lld %lld %d", src_sid, dst_sid, focus);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
}

static inline bool
space_manager_is_space_last_user_space(uint64_t sid)
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
    free(space_list);

    return result;
}

enum space_op_error space_manager_move_space_to_display(struct space_manager *sm, uint64_t sid, uint32_t did)
{
    int sockfd;
    uint64_t d_sid;
    char message[MAXLEN];

    if (!sid) return SPACE_OP_ERROR_MISSING_SRC;
    if (space_display_id(sid) == did) return SPACE_OP_ERROR_INVALID_DST;
    if (space_manager_is_space_last_user_space(sid)) return SPACE_OP_ERROR_INVALID_SRC;

    d_sid = display_space_id(did);
    if (!d_sid) return SPACE_OP_ERROR_MISSING_DST;

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "space_move %lld %lld 1", sid, d_sid);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);

    space_manager_mark_view_invalid(sm, sid);
    space_manager_focus_space(sid);

    return SPACE_OP_ERROR_SUCCESS;
}

enum space_op_error space_manager_destroy_space(uint64_t sid)
{
    int sockfd;
    char message[MAXLEN];

    if (!sid) return SPACE_OP_ERROR_MISSING_SRC;
    if (!space_is_user(sid)) return SPACE_OP_ERROR_INVALID_TYPE;
    if (space_manager_is_space_last_user_space(sid)) return SPACE_OP_ERROR_INVALID_SRC;

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "space_destroy %lld", sid);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);

    return SPACE_OP_ERROR_SUCCESS;
}

void space_manager_add_space(uint64_t sid)
{
    int sockfd;
    char message[MAXLEN];

    if (!sid) return;

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "space_create %lld", sid);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
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
    bool result = false;

    int space_count;
    uint64_t *space_list = window_space_list(window, &space_count);
    if (!space_list) goto out;

    for (int i = 0; i < space_count; ++i) {
        if (sid == space_list[i]) {
            result = true;
            break;
        }
    }

    free(space_list);
out:
    return result;
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

    free(space_list);
}

void space_manager_mark_spaces_invalid(struct space_manager *sm)
{
    uint32_t display_count = 0;
    uint32_t *display_list = display_manager_active_display_list(&display_count);
    if (!display_list) return;

    for (int i = 0; i < display_count; ++i) {
        space_manager_mark_spaces_invalid_for_display(sm, display_list[i]);
    }

    free(display_list);
}

bool space_manager_refresh_application_windows(struct space_manager *sm)
{
    int window_count = g_window_manager.window.count;
    for (int i = 0; i < g_window_manager.application.capacity; ++i) {
        struct bucket *bucket = g_window_manager.application.buckets[i];
        while (bucket) {
            if (bucket->value) {
                struct application *application = bucket->value;
                window_manager_add_application_windows(sm, &g_window_manager, application);
            }
            bucket = bucket->next;
        }
    }

    return window_count != g_window_manager.window.count;
}

void space_manager_init(struct space_manager *sm)
{
    sm->layout = VIEW_FLOAT;
    sm->split_ratio = 0.5f;
    sm->auto_balance = false;
    sm->window_placement = CHILD_SECOND;
    sm->labels = NULL;

    table_init(&sm->view, 23, hash_view, compare_view);

    uint32_t display_count;
    uint32_t *display_list = display_manager_active_display_list(&display_count);
    if (!display_list) return;

    for (int i = 0; i < display_count; ++i) {
        int space_count;
        uint64_t *space_list = display_space_list(display_list[i], &space_count);
        if (!space_list) continue;

        for (int j = 0; j < space_count; ++j) {
            struct view *view = view_create(space_list[j]);
            table_add(&sm->view, &space_list[j], view);
        }

        free(space_list);
    }

    free(display_list);
}

void space_manager_begin(struct space_manager *sm)
{
    sm->current_space_id = space_manager_active_space();
    sm->last_space_id = sm->current_space_id;
    sm->did_begin = true;
}

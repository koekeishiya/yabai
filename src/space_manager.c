#include "space_manager.h"

extern struct window_manager g_window_manager;
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

bool space_manager_menu_bar_hidden(void)
{
    int status = 0;
    SLSGetMenuBarAutohideEnabled(g_connection, &status);
    // printf("autohide = %d\n", status);
    return status;
}

CGRect space_manager_menu_bar_rect(void)
{
    CGRect bounds = {};
    SLSGetRevealedMenuBarBounds(&bounds, g_connection, space_manager_active_space());
    // printf("%.2f, %.2f, %.2f, %.2f\n", bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
    return bounds;
}

bool space_manager_dock_hidden(void)
{
    return CoreDockGetAutoHideEnabled();
}

int space_manager_dock_orientation(void)
{
    int pinning = 0;
    int orientation = 0;
    CoreDockGetOrientationAndPinning(&orientation, &pinning);
    return orientation;
}

CGRect space_manager_dock_rect(void)
{
    int reason = 0;
    CGRect bounds = {};
    SLSGetDockRectWithReason(g_connection, &bounds, &reason);
    // printf("%.2f, %.2f, %.2f, %.2f, %d\n", bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height, reason);
    return bounds;
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
    view_update(view);
    view_flush(view);
}

void space_manager_mark_view_invalid(struct space_manager *sm,  uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    view->is_valid = false;
}

void space_manager_mark_view_dirty(struct space_manager *sm,  uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    view->is_dirty = true;
}

void space_manager_untile_window(struct space_manager *sm, struct view *view, struct ax_window *window)
{
    view_remove_window_node(view, window);
    if (space_is_visible(view->sid)) {
        view_flush(view);
    } else {
        view->is_dirty = true;
    }
}

void space_manager_set_padding_for_space(struct space_manager *sm, uint64_t sid, unsigned top, unsigned bottom, unsigned left, unsigned right)
{
    struct view *view = space_manager_find_view(sm, sid);
    view->top_padding = top;
    view->bottom_padding = bottom;
    view->left_padding = left;
    view->right_padding = right;
    view_update(view);
    view_flush(view);
}

void space_manager_toggle_padding_for_space(struct space_manager *sm, uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    view->enable_padding = !view->enable_padding;
    view_update(view);
    view_flush(view);
}

void space_manager_rotate_space(struct space_manager *sm, uint64_t sid, int degrees)
{
    struct view *view = space_manager_find_view(sm, sid);
    window_node_rotate(view->root, degrees);
    view_update(view);
    view_flush(view);
}

void space_manager_mirror_space(struct space_manager *sm, uint64_t sid, enum window_node_split axis)
{
    struct view *view = space_manager_find_view(sm, sid);
    window_node_mirror(view->root, axis);
    view_update(view);
    view_flush(view);
}

void space_manager_balance_space(struct space_manager *sm, uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    window_node_equalize(view->root);
    view_update(view);
    view_flush(view);
}

struct view *space_manager_tile_window_on_space(struct space_manager *sm, struct ax_window *window, uint64_t sid)
{
    struct view *view = space_manager_find_view(sm, sid);
    view_add_window_node(view, window);
    if (space_is_visible(view->sid)) {
        view_flush(view);
    } else {
        view->is_dirty = true;
    }
    return view;
}

void space_manager_toggle_window_split(struct space_manager *sm, struct ax_window *window)
{
    struct view *view = space_manager_find_view(sm, space_manager_active_space());
    struct window_node *node = view_find_window_node(view->root, window->id);
    if (node && window_node_is_intermediate(node)) {
        node->parent->split = node->parent->split == SPLIT_Y ? SPLIT_X : SPLIT_Y;

        if (g_space_manager.auto_balance) {
            window_node_equalize(view->root);
            view_update(view);
            view_flush(view);
        } else {
            window_node_update(node->parent);
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
            if (!space_is_user(result)) continue;
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
            if (!space_is_user(result))    continue;
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

uint64_t space_manager_active_space(void)
{
    uint32_t did = 0;
    struct ax_window *window = window_manager_focused_window(&g_window_manager);

    if (window) did = window_display_id(window);
    if (!did)   did = display_manager_active_display_id();
    if (!did)   return 0;

    return display_space_id(did);
}

void space_manager_move_window_to_space(uint64_t sid, struct ax_window *window)
{
    CFNumberRef window_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &window->id);
    CFArrayRef window_list_ref = CFArrayCreate(NULL, (void *)&window_id_ref, 1, NULL);
    CGSMoveWindowsToManagedSpace(g_connection, window_list_ref, sid);
    CFRelease(window_list_ref);
    CFRelease(window_id_ref);
}

void space_manager_remove_window_from_space(uint64_t sid, struct ax_window *window)
{
    CFNumberRef window_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &window->id);
    CFArrayRef window_list_ref = CFArrayCreate(NULL, (void *)&window_id_ref, 1, NULL);
    CFNumberRef space_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &sid);
    CFArrayRef space_list_ref = CFArrayCreate(NULL, (void *)&space_id_ref, 1, NULL);
    CGSRemoveWindowsFromSpaces(g_connection, window_list_ref, space_list_ref);
    CFRelease(space_list_ref);
    CFRelease(space_id_ref);
    CFRelease(window_list_ref);
    CFRelease(window_id_ref);
}

void space_manager_add_window_to_space(uint64_t sid, struct ax_window *window)
{
    CFNumberRef window_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &window->id);
    CFArrayRef window_list_ref = CFArrayCreate(NULL, (void *)&window_id_ref, 1, NULL);
    CFNumberRef space_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &sid);
    CFArrayRef space_list_ref = CFArrayCreate(NULL, (void *)&space_id_ref, 1, NULL);
    CGSAddWindowsToSpaces(g_connection, window_list_ref, space_list_ref);
    CFRelease(space_list_ref);
    CFRelease(space_id_ref);
    CFRelease(window_list_ref);
    CFRelease(window_id_ref);
}

void space_manager_focus_space(uint64_t sid)
{
    int sockfd;
    char message[255];

    uint64_t cur_sid = space_manager_active_space();
    uint32_t cur_did = space_display_id(cur_sid);
    uint32_t new_did = space_display_id(sid);

    if (socket_connect_in(&sockfd, 5050)) {
        snprintf(message, sizeof(message), "space %lld", sid);
        socket_write(sockfd, message);
        socket_wait(sockfd);

        if (cur_did != new_did) {
            display_manager_focus_display(new_did);
        }
    }
    socket_close(sockfd);
}

void space_manager_move_space_to_display(struct space_manager *sm, uint32_t did)
{
    int sockfd;
    uint64_t sid;
    uint64_t d_sid;
    char message[255];

    sid = space_manager_active_space();
    if (!sid || !space_is_user(sid)) return;

    d_sid = display_space_id(did);
    if (!d_sid) return;

    if (socket_connect_in(&sockfd, 5050)) {
        snprintf(message, sizeof(message), "space_move %lld %lld", sid, d_sid);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);

    space_manager_mark_view_invalid(sm, sid);
    space_manager_focus_space(sid);
}

void space_manager_destroy_space(void)
{
    int sockfd;
    uint64_t sid;
    char message[255];

    sid = space_manager_active_space();
    if (!sid || !space_is_user(sid)) return;

    if (socket_connect_in(&sockfd, 5050)) {
        snprintf(message, sizeof(message), "space_destroy %lld", sid);
        socket_write(sockfd, message);
        socket_wait(sockfd);
    }
    socket_close(sockfd);
}

void space_manager_add_space(void)
{
    int sockfd;
    uint64_t sid;
    char message[255];

    sid = space_manager_active_space();
    if (!sid) return;

    if (socket_connect_in(&sockfd, 5050)) {
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

bool space_manager_is_window_on_active_space(struct ax_window *window)
{
    uint64_t sid = space_manager_active_space();
    bool result = space_manager_is_window_on_space(sid, window);
    return result;
}

bool space_manager_is_window_on_space(uint64_t sid, struct ax_window *window)
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

bool space_manager_refresh_application_windows(void)
{
    int window_count = g_window_manager.window.count;
    for (int i = 0; i < g_window_manager.application.capacity; ++i) {
        struct bucket *bucket = g_window_manager.application.buckets[i];
        while (bucket) {
            struct ax_application **application = bucket->value;
            window_manager_add_application_windows(&g_window_manager, *application);
            bucket = bucket->next;
        }
    }

    return window_count != g_window_manager.window.count;
}

void space_manager_init(struct space_manager *sm)
{
    for (int i = 1; i < 255 ; ++i) {
        sm->top_padding[i]    = -1;
        sm->bottom_padding[i] = -1;
        sm->left_padding[i]   = -1;
        sm->right_padding[i]  = -1;
    }

    sm->window_gap = 0;
    sm->split_ratio = 0.5f;
    sm->auto_balance = false;

    table_init(&sm->view, 150, hash_view, compare_view);
}

void space_manager_begin(struct space_manager *sm)
{
    sm->current_space_id = space_manager_active_space();
    sm->last_space_id = sm->current_space_id;
}

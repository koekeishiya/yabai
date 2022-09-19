extern struct window_manager g_window_manager;
extern int g_normal_window_level;
extern int g_floating_window_level;
extern int g_connection;

bool window_observe(struct window *window)
{
    for (int i = 0; i < array_count(ax_window_notification); ++i) {
        AXError result = AXObserverAddNotification(window->application->observer_ref, window->ref, ax_window_notification[i], window);
        if (result == kAXErrorSuccess || result == kAXErrorNotificationAlreadyRegistered) {
            window->notification |= 1 << i;
        } else {
            debug("%s: %s failed with error %s\n", __FUNCTION__, ax_window_notification_str[i], ax_error_str[-result]);
        }
    }

    return (window->notification & AX_WINDOW_ALL) == AX_WINDOW_ALL;
}

void window_unobserve(struct window *window)
{
    for (int i = 0; i < array_count(ax_window_notification); ++i) {
        if (!(window->notification & (1 << i))) continue;

        AXObserverRemoveNotification(window->application->observer_ref, window->ref, ax_window_notification[i]);
        window->notification &= ~(1 << i);
    }
}

CFStringRef window_display_uuid(struct window *window)
{
    CFStringRef uuid = SLSCopyManagedDisplayForWindow(g_connection, window->id);
    if (!uuid) {
        uuid = SLSCopyBestManagedDisplayForRect(g_connection, window->frame);
    }
    return uuid;
}

uint32_t window_display_id(struct window *window)
{
    CFStringRef uuid_string = window_display_uuid(window);
    if (!uuid_string) return 0;

    CFUUIDRef uuid = CFUUIDCreateFromString(NULL, uuid_string);
    int id = CGDisplayGetDisplayIDFromUUID(uuid);

    CFRelease(uuid);
    CFRelease(uuid_string);

    return id;
}

static uint64_t window_display_space(struct window *window)
{
    CFStringRef uuid = window_display_uuid(window);
    if (!uuid) return 0;

    uint64_t sid = SLSManagedDisplayGetCurrentSpace(g_connection, uuid);
    CFRelease(uuid);

    return sid;
}

uint64_t window_space(struct window *window)
{
    uint64_t sid = 0;

    CFArrayRef window_list_ref = cfarray_of_cfnumbers(&window->id, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    CFArrayRef space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, window_list_ref);
    if (!space_list_ref) goto err;

    int count = CFArrayGetCount(space_list_ref);
    if (!count) goto free;

    CFNumberRef id_ref = CFArrayGetValueAtIndex(space_list_ref, 0);
    CFNumberGetValue(id_ref, CFNumberGetType(id_ref), &sid);

free:
    CFRelease(space_list_ref);
err:
    CFRelease(window_list_ref);

    return sid ? sid : window_display_space(window);
}

uint64_t *window_space_list(struct window *window, int *count)
{
    uint64_t *space_list = NULL;
    CFArrayRef window_list_ref = cfarray_of_cfnumbers(&window->id, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    CFArrayRef space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, window_list_ref);
    if (!space_list_ref) goto err;

    *count = CFArrayGetCount(space_list_ref);
    if (!*count) goto out;

    space_list = ts_alloc_aligned(sizeof(uint64_t), *count);

    for (int i = 0; i < *count; ++i) {
        CFNumberRef id_ref = CFArrayGetValueAtIndex(space_list_ref, i);
        CFNumberGetValue(id_ref, CFNumberGetType(id_ref), space_list + i);
    }

out:
    CFRelease(space_list_ref);
err:
    CFRelease(window_list_ref);
    return space_list;
}

void window_serialize(FILE *rsp, struct window *window)
{
    char *role = window_role_ts(window);
    char *subrole = window_subrole_ts(window);
    char *title = window_title_ts(window);
    char *escaped_title = ts_string_escape(title);
    uint64_t sid = window_space(window);
    int space = space_manager_mission_control_index(sid);
    int display = display_arrangement(space_display_id(sid));
    bool is_topmost = window_is_topmost(window);
    bool is_minimized = window_is_minimized(window);
    bool visible = !is_minimized && !window->application->is_hidden && (window_check_flag(window, WINDOW_STICKY) || space_is_visible(sid));
    bool border = window->border.id ? 1 : 0;
    float opacity = window_opacity(window);
    bool grabbed = window == g_mouse_state.window;
    uint64_t tags = window_tags(window);

    struct view *view = window_manager_find_managed_window(&g_window_manager, window);
    struct window_node *node = view ? view_find_window_node(view, window->id) : NULL;

    char split[MAXLEN];
    snprintf(split, sizeof(split), "%s", window_node_split_str[node && node->parent ? node->parent->split : 0]);
    bool zoom_parent = node && node->zoom && node->zoom == node->parent;
    bool zoom_fullscreen = node && node->zoom && node->zoom == view->root;
    int stack_index = node && node->window_count > 1 ? window_node_index_of_window(node, window->id)+1 : 0;

    fprintf(rsp,
            "{\n"
            "\t\"id\":%d,\n"
            "\t\"pid\":%d,\n"
            "\t\"app\":\"%s\",\n"
            "\t\"title\":\"%s\",\n"
            "\t\"frame\":{\n\t\t\"x\":%.4f,\n\t\t\"y\":%.4f,\n\t\t\"w\":%.4f,\n\t\t\"h\":%.4f\n\t},\n"
            "\t\"role\":\"%s\",\n"
            "\t\"subrole\":\"%s\",\n"
            "\t\"tags\":\"0x%016llx\",\n"
            "\t\"display\":%d,\n"
            "\t\"space\":%d,\n"
            "\t\"level\":%d,\n"
            "\t\"opacity\":%.4f,\n"
            "\t\"split-type\":\"%s\",\n"
            "\t\"stack-index\":%d,\n"
            "\t\"can-move\":%s,\n"
            "\t\"can-resize\":%s,\n"
            "\t\"has-focus\":%s,\n"
            "\t\"has-shadow\":%s,\n"
            "\t\"has-border\":%s,\n"
            "\t\"has-parent-zoom\":%s,\n"
            "\t\"has-fullscreen-zoom\":%s,\n"
            "\t\"is-native-fullscreen\":%s,\n"
            "\t\"is-visible\":%s,\n"
            "\t\"is-minimized\":%s,\n"
            "\t\"is-hidden\":%s,\n"
            "\t\"is-floating\":%s,\n"
            "\t\"is-sticky\":%s,\n"
            "\t\"is-topmost\":%s,\n"
            "\t\"is-grabbed\":%s\n"
            "}",
            window->id,
            window->application->pid,
            window->application->name,
            escaped_title ? escaped_title : title,
            window->frame.origin.x, window->frame.origin.y, window->frame.size.width, window->frame.size.height,
            role,
            subrole,
            tags,
            display,
            space,
            window_level(window),
            opacity,
            split,
            stack_index,
            json_bool(window_can_move(window)),
            json_bool(window_can_resize(window)),
            json_bool(window->id == g_window_manager.focused_window_id),
            json_bool(window_check_flag(window, WINDOW_SHADOW)),
            json_bool(border),
            json_bool(zoom_parent),
            json_bool(zoom_fullscreen),
            json_bool(window_is_fullscreen(window)),
            json_bool(visible),
            json_bool(is_minimized),
            json_bool(window->application->is_hidden),
            json_bool(window_check_flag(window, WINDOW_FLOAT)),
            json_bool(window_check_flag(window, WINDOW_STICKY)),
            json_bool(is_topmost),
            json_bool(grabbed));
}

char *window_title_ts(struct window *window)
{
    CFTypeRef value = NULL;
    AXUIElementCopyAttributeValue(window->ref, kAXTitleAttribute, &value);
    if (!value) return ts_string_copy("");

    char *result = ts_cfstring_copy(value);
    CFRelease(value);
    return result;
}

CGPoint window_ax_origin(struct window *window)
{
    CGPoint origin = {};
    CFTypeRef position_ref = NULL;

    AXUIElementCopyAttributeValue(window->ref, kAXPositionAttribute, &position_ref);

    if (position_ref) {
        AXValueGetValue(position_ref, kAXValueTypeCGPoint, &origin);
        CFRelease(position_ref);
    }

    return origin;
}

CGRect window_ax_frame(struct window *window)
{
    CGRect frame = {};
    CFTypeRef position_ref = NULL;
    CFTypeRef size_ref = NULL;

    AXUIElementCopyAttributeValue(window->ref, kAXPositionAttribute, &position_ref);
    AXUIElementCopyAttributeValue(window->ref, kAXSizeAttribute, &size_ref);

    if (position_ref) {
        AXValueGetValue(position_ref, kAXValueTypeCGPoint, &frame.origin);
        CFRelease(position_ref);
    }

    if (size_ref) {
        AXValueGetValue(size_ref, kAXValueTypeCGSize, &frame.size);
        CFRelease(size_ref);
    }

    return frame;
}

bool window_can_move(struct window *window)
{
    Boolean result;
    if (AXUIElementIsAttributeSettable(window->ref, kAXPositionAttribute, &result) != kAXErrorSuccess) {
        result = 0;
    }
    return result;
}

bool window_can_resize(struct window *window)
{
    Boolean result;
    if (AXUIElementIsAttributeSettable(window->ref, kAXSizeAttribute, &result) != kAXErrorSuccess) {
        result = 0;
    }
    return result;
}

bool window_can_minimize(struct window *window)
{
    Boolean result;
    if (AXUIElementIsAttributeSettable(window->ref, kAXMinimizedAttribute, &result) != kAXErrorSuccess) {
        result = 0;
    }
    return result;
}

bool window_is_undersized(struct window *window)
{
    if (window->frame.size.width  < 200.0f) return true;
    if (window->frame.size.height < 200.0f) return true;
    return false;
}

bool window_is_minimized(struct window *window)
{
    Boolean result = 0;
    CFTypeRef value;

    if (AXUIElementCopyAttributeValue(window->ref, kAXMinimizedAttribute, &value) == kAXErrorSuccess) {
        result = CFBooleanGetValue(value);
        CFRelease(value);
    }

    return result || window_check_flag(window, WINDOW_MINIMIZE);
}

bool window_is_fullscreen(struct window *window)
{
    Boolean result = 0;
    CFTypeRef value;

    if (AXUIElementCopyAttributeValue(window->ref, kAXFullscreenAttribute, &value) == kAXErrorSuccess) {
        result = CFBooleanGetValue(value);
        CFRelease(value);
    }

    return result;
}

bool window_is_sticky(struct window *window)
{
    bool result = false;
    CFArrayRef window_list_ref = cfarray_of_cfnumbers(&window->id, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    CFArrayRef space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, window_list_ref);
    if (!space_list_ref) goto err;

    result = CFArrayGetCount(space_list_ref) > 1;

    CFRelease(space_list_ref);
err:
    CFRelease(window_list_ref);
    return result;
}

bool window_is_topmost(struct window *window)
{
    bool is_topmost = window_level(window) == CGWindowLevelForKey(LAYER_ABOVE);
    return is_topmost;
}

float window_opacity(struct window *window)
{
    float alpha = 0.0f;
    SLSGetWindowAlpha(g_connection, window->id, &alpha);
    return alpha;
}

int window_level(struct window *window)
{
    int level = 0;
    SLSGetWindowLevel(g_connection, window->id, &level);
    return level;
}

uint64_t window_tags(struct window *window)
{
    uint64_t tags = 0;
    CFArrayRef window_ref = cfarray_of_cfnumbers(&window->id, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    CFTypeRef query = SLSWindowQueryWindows(g_connection, window_ref, 1);
    CFTypeRef iterator = SLSWindowQueryResultCopyWindows(query);

    if (SLSWindowIteratorAdvance(iterator)) {
        tags = SLSWindowIteratorGetTags(iterator);
    }

    CFRelease(query);
    CFRelease(iterator);
    CFRelease(window_ref);
    return tags;
}

CFStringRef window_role(struct window *window)
{
    const void *role = NULL;
    AXUIElementCopyAttributeValue(window->ref, kAXRoleAttribute, &role);
    return role;
}

char *window_role_ts(struct window *window)
{
    CFStringRef role = window_role(window);
    if (!role) return ts_string_copy("");

    char *result = ts_cfstring_copy(role);
    CFRelease(role);
    return result;
}

CFStringRef window_subrole(struct window *window)
{
    const void *srole = NULL;
    AXUIElementCopyAttributeValue(window->ref, kAXSubroleAttribute, &srole);
    return srole;
}

char *window_subrole_ts(struct window *window)
{
    CFStringRef subrole = window_subrole(window);
    if (!subrole) return ts_string_copy("");

    char *result = ts_cfstring_copy(subrole);
    CFRelease(subrole);
    return result;
}

bool window_level_is_standard(struct window *window)
{
    int level = window_level(window);
    return level == g_normal_window_level || level == g_floating_window_level;
}

bool window_is_standard(struct window *window)
{
    bool standard_win = false;
    CFStringRef role  = NULL;
    CFStringRef srole = NULL;

    if (!(role  = window_role(window)))    goto out;
    if (!(srole = window_subrole(window))) goto role;

    standard_win = CFEqual(role, kAXWindowRole) &&
                   CFEqual(srole, kAXStandardWindowSubrole);

    CFRelease(srole);
role:
    CFRelease(role);
out:
    return standard_win;
}

bool window_is_dialog(struct window *window)
{
    bool standard_win = false;
    CFStringRef role  = NULL;
    CFStringRef srole = NULL;

    if (!(role  = window_role(window)))    goto out;
    if (!(srole = window_subrole(window))) goto role;

    standard_win = CFEqual(role, kAXWindowRole) &&
                   CFEqual(srole, kAXDialogSubrole);

    CFRelease(srole);
role:
    CFRelease(role);
out:
    return standard_win;
}

bool window_is_popover(struct window *window)
{
    CFStringRef role = window_role(window);
    if (!role) return false;

    bool result = CFEqual(role, kAXPopoverRole);
    CFRelease(role);

    return result;
}

bool window_is_unknown(struct window *window)
{
    CFStringRef subrole = window_subrole(window);
    if (!subrole) return false;

    bool result = CFEqual(subrole, kAXUnknownSubrole);
    CFRelease(subrole);

    return result;
}

struct window *window_create(struct application *application, AXUIElementRef window_ref, uint32_t window_id)
{
    struct window *window = malloc(sizeof(struct window));
    memset(window, 0, sizeof(struct window));

    window->application = application;
    window->ref = window_ref;
    window->id = window_id;
    window->id_ptr = &window->id;
    window->frame = window_ax_frame(window);
    window_set_flag(window, WINDOW_SHADOW);

    if (window_is_minimized(window)) {
        window_set_flag(window, WINDOW_MINIMIZE);
    }

    if ((window_is_fullscreen(window)) ||
        (space_is_fullscreen(window_space(window)))) {
        window_set_flag(window, WINDOW_FULLSCREEN);
    }

    if (window_is_sticky(window)) {
        window_set_flag(window, WINDOW_STICKY);
    }

    return window;
}

void window_destroy(struct window *window)
{
    CFRelease(window->ref);
    free(window);
}

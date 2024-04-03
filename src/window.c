extern struct window_manager g_window_manager;
extern int g_layer_normal_window_level;
extern int g_layer_below_window_level;
extern int g_layer_above_window_level;
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

CFStringRef window_display_uuid(uint32_t wid)
{
    CFStringRef uuid = SLSCopyManagedDisplayForWindow(g_connection, wid);
    if (!uuid) {
        CGRect frame;
        SLSGetWindowBounds(g_connection, wid, &frame);
        uuid = SLSCopyBestManagedDisplayForRect(g_connection, frame);
    }
    return uuid;
}

uint32_t window_display_id(uint32_t wid)
{
    CFStringRef uuid_string = window_display_uuid(wid);
    if (!uuid_string) return 0;

    CFUUIDRef uuid = CFUUIDCreateFromString(NULL, uuid_string);
    int id = CGDisplayGetDisplayIDFromUUID(uuid);

    CFRelease(uuid);
    CFRelease(uuid_string);

    return id;
}

static uint64_t window_display_space(uint32_t wid)
{
    CFStringRef uuid = window_display_uuid(wid);
    if (!uuid) return 0;

    uint64_t sid = SLSManagedDisplayGetCurrentSpace(g_connection, uuid);
    CFRelease(uuid);

    return sid;
}

uint64_t window_space(uint32_t wid)
{
    uint64_t sid = 0;

    CFArrayRef window_list_ref = cfarray_of_cfnumbers(&wid, sizeof(uint32_t), 1, kCFNumberSInt32Type);
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

    return sid ? sid : window_display_space(wid);
}

uint64_t *window_space_list(uint32_t wid, int *count)
{
    uint64_t *space_list = NULL;
    CFArrayRef window_list_ref = cfarray_of_cfnumbers(&wid, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    CFArrayRef space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, window_list_ref);
    if (!space_list_ref) goto err;

    *count = CFArrayGetCount(space_list_ref);
    if (!*count) goto out;

    space_list = ts_alloc_list(uint64_t, *count);

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

static inline const char *window_layer(int level)
{
    if (level == g_layer_below_window_level)  return layer_str[LAYER_BELOW];
    if (level == g_layer_normal_window_level) return layer_str[LAYER_NORMAL];
    if (level == g_layer_above_window_level)  return layer_str[LAYER_ABOVE];
    return "unknown";
}

void window_nonax_serialize(FILE *rsp, uint32_t wid, uint64_t flags)
{
    TIME_FUNCTION;

    if (flags == 0x0) flags |= ~flags;

    int connection;
    pid_t pid;
    uint64_t sid;
    int level;
    int sub_level;

    if ((flags & WINDOW_PROPERTY_PID) ||
        (flags & WINDOW_PROPERTY_APP)) {
        SLSGetWindowOwner(g_connection, wid, &connection);
        SLSConnectionGetPID(connection, &pid);
    }

    if ((flags & WINDOW_PROPERTY_DISPLAY) ||
        (flags & WINDOW_PROPERTY_SPACE) ||
        (flags & WINDOW_PROPERTY_IS_FULLSCREEN)) {
        sid = window_space(wid);
    }

    if ((flags & WINDOW_PROPERTY_LEVEL) ||
        (flags & WINDOW_PROPERTY_LAYER)) {
        level = window_level(wid);
    }

    if ((flags & WINDOW_PROPERTY_SUB_LEVEL) ||
        (flags & WINDOW_PROPERTY_SUB_LAYER)) {
        sub_level = window_sub_level(wid);
    }

    bool did_output = false;
    fprintf(rsp, "{\n");

    if (flags & WINDOW_PROPERTY_ID) {
        fprintf(rsp, "\t\"id\":%d", wid);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_PID) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"pid\":%d", pid);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_APP) {
        if (did_output) fprintf(rsp, ",\n");

        static char process_name[PROC_PIDPATHINFO_MAXSIZE];
        proc_name(pid, process_name, sizeof(process_name));

        char *app = process_name;
        char *escaped_app = ts_string_escape(app);

        fprintf(rsp, "\t\"app\":\"%s\"", escaped_app ? escaped_app : app);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_TITLE) {
        if (did_output) fprintf(rsp, ",\n");

        char *title = window_property_title_ts(wid);
        char *escaped_title = ts_string_escape(title);

        fprintf(rsp, "\t\"title\":\"%s\"", escaped_title ? escaped_title : title);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SCRATCHPAD) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"scratchpad\":\"%s\"", "");
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_FRAME) {
        if (did_output) fprintf(rsp, ",\n");

        CGRect frame;
        SLSGetWindowBounds(g_connection, wid, &frame);

        fprintf(rsp, "\t\"frame\":{\n\t\t\"x\":%.4f,\n\t\t\"y\":%.4f,\n\t\t\"w\":%.4f,\n\t\t\"h\":%.4f\n\t}", frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_ROLE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"role\":\"%s\"", "");
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SUBROLE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"subrole\":\"%s\"", "");
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_ROOT_WINDOW) {
        if (did_output) fprintf(rsp, ",\n");

        uint32_t parent_wid = window_parent(wid);
        fprintf(rsp, "\t\"root-window\":%s", json_bool(parent_wid == 0));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_DISPLAY) {
        if (did_output) fprintf(rsp, ",\n");

        int display = display_manager_display_id_arrangement(space_display_id(sid));
        fprintf(rsp, "\t\"display\":%d", display);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SPACE) {
        if (did_output) fprintf(rsp, ",\n");

        int space = space_manager_mission_control_index(sid);
        fprintf(rsp, "\t\"space\":%d", space);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_LEVEL) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"level\":%d", level);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SUB_LEVEL) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"sub-level\":%d", sub_level);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_LAYER) {
        if (did_output) fprintf(rsp, ",\n");

        const char *layer = window_layer(level);
        fprintf(rsp, "\t\"layer\":\"%s\"", layer);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SUB_LAYER) {
        if (did_output) fprintf(rsp, ",\n");

        const char *sub_layer = window_layer(sub_level);
        fprintf(rsp, "\t\"sub-layer\":\"%s\"", sub_layer);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_OPACITY) {
        if (did_output) fprintf(rsp, ",\n");

        float opacity = window_opacity(wid);
        fprintf(rsp, "\t\"opacity\":%.4f", opacity);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SPLIT_TYPE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"split-type\":\"%s\"", window_node_split_str[0]);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SPLIT_CHILD) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"split-child\":\"%s\"", window_node_child_str[CHILD_NONE]);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_STACK_INDEX) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"stack-index\":%d", 0);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_CAN_MOVE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"can-move\":%s", json_bool(false));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_CAN_RESIZE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"can-resize\":%s", json_bool(false));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_HAS_FOCUS) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"has-focus\":%s", json_bool(false));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_HAS_SHADOW) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"has-shadow\":%s", json_bool(window_shadow(wid)));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_HAS_PARENT_ZOOM) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"has-parent-zoom\":%s", json_bool(false));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_HAS_FULLSCREEN_ZOOM) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"has-fullscreen-zoom\":%s", json_bool(false));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_HAS_AX_REFERENCE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"has-ax-reference\":%s", json_bool(false));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_FULLSCREEN) {
        if (did_output) fprintf(rsp, ",\n");

        bool is_fullscreen = space_is_fullscreen(sid);
        fprintf(rsp, "\t\"is-native-fullscreen\":%s", json_bool(is_fullscreen));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_VISIBLE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"is-visible\":%s", json_bool(false));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_MINIMIZED) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"is-minimized\":%s", json_bool(false));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_HIDDEN) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"is-hidden\":%s", json_bool(false));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_FLOATING) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"is-floating\":%s", json_bool(false));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_STICKY) {
        if (did_output) fprintf(rsp, ",\n");

        bool is_sticky = window_is_sticky(wid);
        fprintf(rsp, "\t\"is-sticky\":%s", json_bool(is_sticky));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_GRABBED) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"is-grabbed\":%s", json_bool(false));
    }

    fprintf(rsp, "\n}");
}

void window_serialize(FILE *rsp, struct window *window, uint64_t flags)
{
    TIME_FUNCTION;

    if (flags == 0x0) flags |= ~flags;

    uint64_t sid;
    int level;
    int sub_level;
    struct view *view;
    struct window_node *node;
    bool is_minimized;
    bool is_sticky;

    if ((flags & WINDOW_PROPERTY_DISPLAY) ||
        (flags & WINDOW_PROPERTY_SPACE) ||
        (flags & WINDOW_PROPERTY_IS_VISIBLE)) {
        sid = window_space(window->id);
    }

    if ((flags & WINDOW_PROPERTY_LEVEL) ||
        (flags & WINDOW_PROPERTY_LAYER)) {
        level = window_level(window->id);
    }

    if ((flags & WINDOW_PROPERTY_SUB_LEVEL) ||
        (flags & WINDOW_PROPERTY_SUB_LAYER)) {
        sub_level = window_sub_level(window->id);
    }

    if ((flags & WINDOW_PROPERTY_SPLIT_TYPE) ||
        (flags & WINDOW_PROPERTY_SPLIT_CHILD) ||
        (flags & WINDOW_PROPERTY_STACK_INDEX) ||
        (flags & WINDOW_PROPERTY_HAS_PARENT_ZOOM) ||
        (flags & WINDOW_PROPERTY_HAS_FULLSCREEN_ZOOM)) {
        view = window_manager_find_managed_window(&g_window_manager, window);
        node = view ? view_find_window_node(view, window->id) : NULL;
    }

    if ((flags & WINDOW_PROPERTY_IS_VISIBLE) ||
        (flags & WINDOW_PROPERTY_IS_MINIMIZED)) {
        is_minimized = window_is_minimized(window);
    }

    if ((flags & WINDOW_PROPERTY_IS_VISIBLE) ||
        (flags & WINDOW_PROPERTY_IS_STICKY)) {
        is_sticky = window_check_flag(window, WINDOW_STICKY) || window_is_sticky(window->id);
    }

    bool did_output = false;
    fprintf(rsp, "{\n");

    if (flags & WINDOW_PROPERTY_ID) {
        fprintf(rsp, "\t\"id\":%d", window->id);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_PID) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"pid\":%d", window->application->pid);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_APP) {
        if (did_output) fprintf(rsp, ",\n");

        char *app = window->application->name;
        char *escaped_app = ts_string_escape(app);

        fprintf(rsp, "\t\"app\":\"%s\"", escaped_app ? escaped_app : app);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_TITLE) {
        if (did_output) fprintf(rsp, ",\n");

        char *title = window_title_ts(window);
        char *escaped_title = ts_string_escape(title);

        fprintf(rsp, "\t\"title\":\"%s\"", escaped_title ? escaped_title : title);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SCRATCHPAD) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"scratchpad\":\"%s\"", window->scratchpad ? window->scratchpad : "");
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_FRAME) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"frame\":{\n\t\t\"x\":%.4f,\n\t\t\"y\":%.4f,\n\t\t\"w\":%.4f,\n\t\t\"h\":%.4f\n\t}", window->frame.origin.x, window->frame.origin.y, window->frame.size.width, window->frame.size.height);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_ROLE) {
        if (did_output) fprintf(rsp, ",\n");

        char *role = window_role_ts(window);
        fprintf(rsp, "\t\"role\":\"%s\"", role);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SUBROLE) {
        if (did_output) fprintf(rsp, ",\n");

        char *subrole = window_subrole_ts(window);
        fprintf(rsp, "\t\"subrole\":\"%s\"", subrole);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_ROOT_WINDOW) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"root-window\":%s", json_bool(window->is_root));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_DISPLAY) {
        if (did_output) fprintf(rsp, ",\n");

        int display = display_manager_display_id_arrangement(space_display_id(sid));
        fprintf(rsp, "\t\"display\":%d", display);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SPACE) {
        if (did_output) fprintf(rsp, ",\n");

        int space = space_manager_mission_control_index(sid);
        fprintf(rsp, "\t\"space\":%d", space);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_LEVEL) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"level\":%d", level);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SUB_LEVEL) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"sub-level\":%d", sub_level);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_LAYER) {
        if (did_output) fprintf(rsp, ",\n");

        const char *layer = window_layer(level);
        fprintf(rsp, "\t\"layer\":\"%s\"", layer);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SUB_LAYER) {
        if (did_output) fprintf(rsp, ",\n");

        const char *sub_layer = window_layer(sub_level);
        fprintf(rsp, "\t\"sub-layer\":\"%s\"", sub_layer);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_OPACITY) {
        if (did_output) fprintf(rsp, ",\n");

        float opacity = window_opacity(window->id);
        fprintf(rsp, "\t\"opacity\":%.4f", opacity);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SPLIT_TYPE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"split-type\":\"%s\"", window_node_split_str[node && node->parent ? node->parent->split : 0]);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_SPLIT_CHILD) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"split-child\":\"%s\"", window_node_child_str[node ? window_node_is_left_child(node) ? CHILD_FIRST : CHILD_SECOND : CHILD_NONE]);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_STACK_INDEX) {
        if (did_output) fprintf(rsp, ",\n");

        int stack_index = node && node->window_count > 1 ? window_node_index_of_window(node, window->id)+1 : 0;
        fprintf(rsp, "\t\"stack-index\":%d", stack_index);
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_CAN_MOVE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"can-move\":%s", json_bool(window_can_move(window)));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_CAN_RESIZE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"can-resize\":%s", json_bool(window_can_resize(window)));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_HAS_FOCUS) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"has-focus\":%s", json_bool(window->id == g_window_manager.focused_window_id));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_HAS_SHADOW) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"has-shadow\":%s", json_bool(window_shadow(window->id)));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_HAS_PARENT_ZOOM) {
        if (did_output) fprintf(rsp, ",\n");

        bool zoom_parent = node && node->zoom && node->zoom == node->parent;
        fprintf(rsp, "\t\"has-parent-zoom\":%s", json_bool(zoom_parent));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_HAS_FULLSCREEN_ZOOM) {
        if (did_output) fprintf(rsp, ",\n");

        bool zoom_fullscreen = node && node->zoom && node->zoom == view->root;
        fprintf(rsp, "\t\"has-fullscreen-zoom\":%s", json_bool(zoom_fullscreen));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_HAS_AX_REFERENCE) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"has-ax-reference\":%s", json_bool(true));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_FULLSCREEN) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"is-native-fullscreen\":%s", json_bool(window_is_fullscreen(window)));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_VISIBLE) {
        if (did_output) fprintf(rsp, ",\n");

        uint8_t ordered_in = 0;
        SLSWindowIsOrderedIn(g_connection, window->id, &ordered_in);

        bool visible = ordered_in && !is_minimized && !window->application->is_hidden && (is_sticky || space_is_visible(sid));
        fprintf(rsp, "\t\"is-visible\":%s", json_bool(visible));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_MINIMIZED) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"is-minimized\":%s", json_bool(is_minimized));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_HIDDEN) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"is-hidden\":%s", json_bool(window->application->is_hidden));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_FLOATING) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"is-floating\":%s", json_bool(window_check_flag(window, WINDOW_FLOAT)));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_STICKY) {
        if (did_output) fprintf(rsp, ",\n");

        fprintf(rsp, "\t\"is-sticky\":%s", json_bool(is_sticky));
        did_output = true;
    }

    if (flags & WINDOW_PROPERTY_IS_GRABBED) {
        if (did_output) fprintf(rsp, ",\n");

        bool grabbed = window == g_mouse_state.window;
        fprintf(rsp, "\t\"is-grabbed\":%s", json_bool(grabbed));
    }

    fprintf(rsp, "\n}");
}

char *window_property_title_ts(uint32_t wid)
{
    CFTypeRef value = NULL;
    SLSCopyWindowProperty(g_connection, wid, CFSTR("kCGSWindowTitle"), &value);
    if (!value) return ts_string_copy("");

    char *result = ts_cfstring_copy(value);
    CFRelease(value);
    return result;
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
    if (window->frame.size.width  <= 500.0f) return true;
    if (window->frame.size.height <= 500.0f) return true;
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

bool window_is_sticky(uint32_t wid)
{
    bool result = false;

    CFArrayRef window_list_ref = cfarray_of_cfnumbers(&wid, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    CFArrayRef space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, window_list_ref);
    if (!space_list_ref) goto err;

    result = CFArrayGetCount(space_list_ref) > 1;
    CFRelease(space_list_ref);

err:
    CFRelease(window_list_ref);
    return result;
}

bool window_shadow(uint32_t wid)
{
    uint64_t tags = window_tags(wid);
    return !(tags & 0x8);
}

float window_opacity(uint32_t wid)
{
    float alpha = 0.0f;
    SLSGetWindowAlpha(g_connection, wid, &alpha);
    return alpha;
}

uint32_t window_parent(uint32_t wid)
{
    uint32_t parent_wid = 0;

    CFArrayRef window_ref = cfarray_of_cfnumbers(&wid, sizeof(uint32_t), 1, kCFNumberSInt32Type);

    CFTypeRef query = SLSWindowQueryWindows(g_connection, window_ref, 1);
    if (!query) goto err2;

    CFTypeRef iterator = SLSWindowQueryResultCopyWindows(query);
    if (!iterator) goto err1;

    if (SLSWindowIteratorGetCount(iterator) == 1) {
        if (SLSWindowIteratorAdvance(iterator)) {
            parent_wid = SLSWindowIteratorGetParentID(iterator);
        }
    }

    CFRelease(iterator);
err1:
    CFRelease(query);
err2:
    CFRelease(window_ref);

    return parent_wid;
}

int window_level(uint32_t wid)
{
    int level = 0;

    if (workspace_is_macos_ventura() || workspace_is_macos_sonoma()) {
        CFArrayRef window_ref = cfarray_of_cfnumbers(&wid, sizeof(uint32_t), 1, kCFNumberSInt32Type);

        CFTypeRef query = SLSWindowQueryWindows(g_connection, window_ref, 1);
        if (!query) goto err2;

        CFTypeRef iterator = SLSWindowQueryResultCopyWindows(query);
        if (!iterator) goto err1;

        if (SLSWindowIteratorGetCount(iterator) == 1) {
            level = SLSWindowIteratorGetLevel(iterator, 0);
        }

        CFRelease(iterator);
    err1:
        CFRelease(query);
    err2:
        CFRelease(window_ref);
    } else {
        SLSGetWindowLevel(g_connection, wid, &level);
    }

    return level;
}

static int SLSGetWindowSubLevel__Internal(int cid, uint32_t wid)
{
    #pragma pack(push,4)
    struct {
        mach_msg_header_t header;
        NDR_record_t NDR_record;
        uint32_t window_id;
        int32_t sub_level;
        int32_t padding1;
        int32_t padding2;
    } msg = {0};
    #pragma pack(pop)

    msg.NDR_record = NDR_record;
    msg.window_id = wid;
    msg.header.msgh_bits = 0x1513;
    msg.header.msgh_remote_port = CGSGetConnectionPortById(cid);
    msg.header.msgh_local_port = mig_get_special_reply_port();
    msg.header.msgh_id = 0x73C3;
    mach_msg(&msg.header, MACH_SEND_MSG|MACH_RCV_MSG, 0x24, 0x30, msg.header.msgh_local_port, 0, 0);

    return msg.sub_level;
}

int window_sub_level(uint32_t wid)
{
    if (CGSGetConnectionPortById) {
        return SLSGetWindowSubLevel__Internal(g_connection, wid);
    } else {
        return SLSGetWindowSubLevel(g_connection, wid);
    }
}

uint64_t window_tags(uint32_t wid)
{
    uint64_t tags = 0;
    CFArrayRef window_ref = cfarray_of_cfnumbers(&wid, sizeof(uint32_t), 1, kCFNumberSInt32Type);

    CFTypeRef query = SLSWindowQueryWindows(g_connection, window_ref, 1);
    if (!query) goto err2;

    CFTypeRef iterator = SLSWindowQueryResultCopyWindows(query);
    if (!iterator) goto err1;

    if (SLSWindowIteratorGetCount(iterator) == 1) {
        if (SLSWindowIteratorAdvance(iterator)) {
            tags = SLSWindowIteratorGetTags(iterator);
        }
    }

    CFRelease(iterator);
err1:
    CFRelease(query);
err2:
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

static bool window_is_root(struct window *window)
{
    bool result = false;
    CFTypeRef value = NULL;

    if (AXUIElementCopyAttributeValue(window->ref, kAXParentAttribute, &value) == kAXErrorSuccess) {
        result = !(value && !CFEqual(value, window->application->ref));
    }

    if (value) CFRelease(value);
    return result;
}

bool window_is_real(struct window *window)
{
    bool win = false;
    CFStringRef role  = NULL;
    CFStringRef srole = NULL;

    if (!(role  = window_role(window)))    goto out;
    if (!(srole = window_subrole(window))) goto role;

    win = CFEqual(role, kAXWindowRole) &&
          (CFEqual(srole, kAXStandardWindowSubrole) ||
           CFEqual(srole, kAXFloatingWindowSubrole) ||
           CFEqual(srole, kAXDialogSubrole));

    CFRelease(srole);
role:
    CFRelease(role);
out:
    return win;
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

bool window_level_is_standard(struct window *window)
{
    int level = window_level(window->id);
    return level == g_layer_normal_window_level;
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
    window->is_root = !window_parent(window->id) || window_is_root(window);
    if (window_shadow(window->id)) window_set_flag(window, WINDOW_SHADOW);

    if (window_is_minimized(window)) {
        window_set_flag(window, WINDOW_MINIMIZE);
    }

    if ((window_is_fullscreen(window)) ||
        (space_is_fullscreen(window_space(window->id)))) {
        window_set_flag(window, WINDOW_FULLSCREEN);
    }

    if (window_is_sticky(window->id)) {
        window_set_flag(window, WINDOW_STICKY);
    }

    return window;
}

void window_destroy(struct window *window)
{
    window->id = 0;
    CFRelease(window->ref);
    free(window);
}

void window_dump_cg_window_levels(void)
{
    for (int i = 0; i < CG_WINDOW_LEVEL_KEY_COUNT; ++i) {
        printf("CGWindowLevelForKey(%s) -> %d\n", cg_window_level_key_str[i], CGWindowLevelForKey(i));
    }
}

#include "window.h"

extern struct window_manager g_window_manager;
extern int g_normal_window_level;
extern int g_floating_window_level;
extern int g_connection;

static void
window_observe_notification(struct window *window, int notification)
{
    AXError result = AXObserverAddNotification(window->application->observer_ref, window->ref, ax_window_notification[notification], window->id_ptr);
    if (result == kAXErrorSuccess || result == kAXErrorNotificationAlreadyRegistered) {
        window->notification |= 1 << notification;
    } else {
        debug("%s: %s failed with error %s\n", __FUNCTION__, ax_window_notification_str[notification], ax_error_str[-result]);
    }
}

static void
window_unobserve_notification(struct window *window, int notification)
{
    AXObserverRemoveNotification(window->application->observer_ref, window->ref, ax_window_notification[notification]);
    window->notification &= ~(1 << notification);
}

bool window_observe(struct window *window)
{
    for (int i = 0; i < array_count(ax_window_notification); ++i) {
        window_observe_notification(window, i);
    }

    return (window->notification & AX_WINDOW_ALL) == AX_WINDOW_ALL;
}

void window_unobserve(struct window *window)
{
    for (int i = 0; i < array_count(ax_window_notification); ++i) {
        if (!(window->notification & (1 << i))) continue;
        window_unobserve_notification(window, i);
    }
}

CFStringRef window_display_uuid(struct window *window)
{
    CFStringRef uuid = SLSCopyManagedDisplayForWindow(g_connection, window->id);
    if (!uuid) {
        CGRect frame = window_frame(window);
        uuid = SLSCopyBestManagedDisplayForRect(g_connection, frame);
    }
    return uuid;
}

int window_display_id(struct window *window)
{
    CFStringRef uuid_string = window_display_uuid(window);
    if (!uuid_string) return 0;

    CFUUIDRef uuid = CFUUIDCreateFromString(NULL, uuid_string);
    int id = CGDisplayGetDisplayIDFromUUID(uuid);

    CFRelease(uuid);
    CFRelease(uuid_string);

    return id;
}

uint64_t window_space(struct window *window)
{
    uint64_t sid = 0;
    CFArrayRef window_list_ref = cfarray_of_cfnumbers(&window->id, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    CFArrayRef space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, window_list_ref);
    if (!space_list_ref) goto err;

    int count = CFArrayGetCount(space_list_ref);
    if (count) {
        CFNumberRef id_ref = CFArrayGetValueAtIndex(space_list_ref, 0);
        CFNumberGetValue(id_ref, CFNumberGetType(id_ref), &sid);
    }

    CFRelease(space_list_ref);
err:
    CFRelease(window_list_ref);
    return sid;
}

uint64_t *window_space_list(struct window *window, int *count)
{
    uint64_t *space_list = NULL;
    CFArrayRef window_list_ref = cfarray_of_cfnumbers(&window->id, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    CFArrayRef space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, window_list_ref);
    if (!space_list_ref) goto err;

    *count = CFArrayGetCount(space_list_ref);
    if (!*count) goto out;

    space_list = ts_alloc(*count * sizeof(uint64_t));
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
    char *role = NULL;
    char *subrole = NULL;
    char *title = window_title(window);
    char *escaped_title = ts_string_escape(title);
    CGRect frame = window_frame(window);
    uint64_t sid = window_space(window);
    int space = space_manager_mission_control_index(sid);
    int display = display_arrangement(space_display_id(sid));
    bool is_topmost = window_is_topmost(window);
    bool is_minimized = window_is_minimized(window);
    bool visible = !is_minimized && (window->is_sticky || space_is_visible(sid));
    bool border = window->border.id ? 1 : 0;
    float opacity = window_opacity(window);

    CFStringRef cfrole = window_role(window);
    if (cfrole) {
        role = ts_cfstring_copy(cfrole);
        CFRelease(cfrole);
    }

    CFStringRef cfsubrole = window_subrole(window);
    if (cfsubrole) {
        subrole = ts_cfstring_copy(cfsubrole);
        CFRelease(cfsubrole);
    }

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
            "\t\"level\":%d,\n"
            "\t\"role\":\"%s\",\n"
            "\t\"subrole\":\"%s\",\n"
            "\t\"movable\":%d,\n"
            "\t\"resizable\":%d,\n"
            "\t\"display\":%d,\n"
            "\t\"space\":%d,\n"
            "\t\"visible\":%d,\n"
            "\t\"focused\":%d,\n"
            "\t\"split\":\"%s\",\n"
            "\t\"floating\":%d,\n"
            "\t\"sticky\":%d,\n"
            "\t\"minimized\":%d,\n"
            "\t\"topmost\":%d,\n"
            "\t\"opacity\":%.4f,\n"
            "\t\"shadow\":%d,\n"
            "\t\"border\":%d,\n"
            "\t\"stack-index\":%d,\n"
            "\t\"zoom-parent\":%d,\n"
            "\t\"zoom-fullscreen\":%d,\n"
            "\t\"native-fullscreen\":%d\n"
            "}",
            window->id,
            window->application->pid,
            window->application->name,
            escaped_title ? escaped_title : title,
            frame.origin.x, frame.origin.y,
            frame.size.width, frame.size.height,
            window_level(window),
            role ? role : "",
            subrole ? subrole : "",
            window_can_move(window),
            window_can_resize(window),
            display,
            space,
            visible,
            window->id == g_window_manager.focused_window_id,
            split,
            window->is_floating,
            window->is_sticky,
            is_minimized,
            is_topmost,
            opacity,
            window->has_shadow,
            border,
            stack_index,
            zoom_parent,
            zoom_fullscreen,
            window_is_fullscreen(window));
}

char *window_title(struct window *window)
{
    char *title = NULL;
    CFTypeRef value = NULL;

#if 0
    SLSCopyWindowProperty(g_connection, window->id, CFSTR("kCGSWindowTitle"), &value);
#else
    AXUIElementCopyAttributeValue(window->ref, kAXTitleAttribute, &value);
#endif

    if (value) {
        title = ts_cfstring_copy(value);
        CFRelease(value);
    } else {
        title = ts_string_copy("");
    }

    return title;
}

CGRect window_ax_frame(struct window *window)
{
    CGRect frame = {};
    CFTypeRef position_ref = NULL;
    CFTypeRef size_ref = NULL;

    AXUIElementCopyAttributeValue(window->ref, kAXPositionAttribute, &position_ref);
    AXUIElementCopyAttributeValue(window->ref, kAXSizeAttribute, &size_ref);

    if (position_ref != NULL) {
        AXValueGetValue(position_ref, kAXValueTypeCGPoint, &frame.origin);
        CFRelease(position_ref);
    }

    if (size_ref != NULL) {
        AXValueGetValue(size_ref, kAXValueTypeCGSize, &frame.size);
        CFRelease(size_ref);
    }

    return frame;
}

CGRect window_frame(struct window *window)
{
    CGRect frame = {};
    SLSGetWindowBounds(g_connection, window->id, &frame);
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
    CGRect frame = window_frame(window);
    if (frame.size.width  < 200.0f) return true;
    if (frame.size.height < 200.0f) return true;
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

    return result || window->is_minimized;
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

CFStringRef window_subrole(struct window *window)
{
    const void *srole = NULL;
    AXUIElementCopyAttributeValue(window->ref, kAXSubroleAttribute, &srole);
    return srole;
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
    SLSGetWindowOwner(g_connection, window->id, &window->connection);
    window->is_minimized = window_is_minimized(window);
    window->is_fullscreen = window_is_fullscreen(window) || space_is_fullscreen(window_space(window));
    window->is_sticky = window_is_sticky(window);
    window->id_ptr = malloc(sizeof(uint32_t *));
    *window->id_ptr = &window->id;
    window->has_shadow = true;

    if (g_window_manager.enable_window_border) border_create(window);

    return window;
}

void window_destroy(struct window *window)
{
    border_destroy(window);
    CFRelease(window->ref);
    free(window->id_ptr);
    free(window);
}

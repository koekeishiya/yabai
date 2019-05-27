#include "window.h"

extern int g_connection;

static void
window_observe_notification(struct ax_window *window, int notification)
{
    AXError result = AXObserverAddNotification(window->application->observer_ref, window->ref, ax_window_notification[notification], window->id_ptr);
    if (result == kAXErrorSuccess || result == kAXErrorNotificationAlreadyRegistered) window->notification |= 1 << notification;
}

static void
window_unobserve_notification(struct ax_window *window, int notification)
{
    AXObserverRemoveNotification(window->application->observer_ref, window->ref, ax_window_notification[notification]);
    window->notification &= ~(1 << notification);
}

bool window_observe(struct ax_window *window)
{
    for (int i = 0; i < array_count(ax_window_notification); ++i) {
        window_observe_notification(window, i);
    }

    return (window->notification & AX_WINDOW_ALL) == AX_WINDOW_ALL;
}

void window_unobserve(struct ax_window *window)
{
    for (int i = 0; i < array_count(ax_window_notification); ++i) {
        if (!(window->notification & (1 << i))) continue;
        window_unobserve_notification(window, i);
    }
}

CFStringRef window_display_uuid(struct ax_window *window)
{
    CFStringRef uuid = SLSCopyManagedDisplayForWindow(g_connection, window->id);
    if (!uuid) {
        CGRect frame = window_frame(window);
        uuid = SLSCopyBestManagedDisplayForRect(g_connection, frame);
    }
    return uuid;
}

int window_display_id(struct ax_window *window)
{
    CFStringRef uuid_string = window_display_uuid(window);
    if (!uuid_string) return 0;

    CFUUIDRef uuid = CFUUIDCreateFromString(NULL, uuid_string);
    int id = CGDisplayGetDisplayIDFromUUID(uuid);

    CFRelease(uuid);
    CFRelease(uuid_string);

    return id;
}

uint64_t *window_space_list(struct ax_window *window, int *count)
{
    uint64_t *space_list = NULL;
    CFNumberRef window_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &window->id);
    CFArrayRef window_list_ref = CFArrayCreate(NULL, (void *)&window_id_ref, 1, NULL);
    CFArrayRef space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, window_list_ref);
    if (!space_list_ref) goto err;

    *count = CFArrayGetCount(space_list_ref);
    if (!*count) goto out;

    space_list = malloc(*count * sizeof(uint64_t));
    for (int i = 0; i < *count; ++i) {
        CFNumberRef id_ref = CFArrayGetValueAtIndex(space_list_ref, i);
        CFNumberGetValue(id_ref, CFNumberGetType(id_ref), space_list + i);
        CFRelease(id_ref);
    }

out:
    CFRelease(space_list_ref);
err:
    CFRelease(window_list_ref);
    CFRelease(window_id_ref);
    return space_list;
}

void window_serialize(struct ax_window *window, FILE *rsp)
{
    CGRect frame = window_frame(window);
    char *title = window_title(window);
    char *role = NULL;
    char *subrole = NULL;

    CFStringRef cfrole = window_role(window);
    if (cfrole) {
        role = cfstring_copy(cfrole);
        CFRelease(cfrole);
    }

    CFStringRef cfsubrole = window_subrole(window);
    if (cfsubrole) {
        subrole = cfstring_copy(cfsubrole);
        CFRelease(cfsubrole);
    }

    fprintf(rsp,
            "{\n"
            "\t\"id\":%d,\n"
            "\t\"app\":\"%s\",\n"
            "\t\"title\":\"%s\",\n"
            "\t\"frame\":{\n\t\t\"x\":%.4f,\n\t\t\"y\":%.4f,\n\t\t\"w\":%.4f,\n\t\t\"h\":%.4f\n\t},\n"
            "\t\"role\":\"%s\",\n"
            "\t\"subrole\":\"%s\",\n"
            "\t\"movable\":%d,\n"
            "\t\"resizable\":%d,\n"
            "\t\"fullscreen\":%d,\n"
            "\t\"floating\":%d\n"
            "}",
            window->id,
            window->application->name,
            title ? title : "",
            frame.origin.x, frame.origin.y,
            frame.size.width, frame.size.height,
            role ? role : "",
            subrole ? subrole : "",
            window_can_move(window),
            window_can_resize(window),
            window_is_fullscreen(window),
            window->is_floating);

    if (subrole) free(subrole);
    if (role) free(role);
    if (title) free(title);
}

char *window_title(struct ax_window *window)
{
    char *title = NULL;
    CFTypeRef value = NULL;

    if (SLSCopyWindowProperty(g_connection, window->id, CFSTR("kCGSWindowTitle"), &value) == kCGErrorSuccess) {
        title = cfstring_copy(value);
        CFRelease(value);
    }

    return title;
}

CGRect window_ax_frame(struct ax_window *window)
{
    CGRect frame = {};

    CFTypeRef position_ref;
    if (AXUIElementCopyAttributeValue(window->ref, kAXPositionAttribute, &position_ref) == kAXErrorSuccess) {
        AXValueGetValue(position_ref, kAXValueTypeCGPoint, &frame.origin);
        CFRelease(position_ref);
    }

    CFTypeRef size_ref;
    if (AXUIElementCopyAttributeValue(window->ref, kAXSizeAttribute, &size_ref) == kAXErrorSuccess) {
        AXValueGetValue(size_ref, kAXValueTypeCGSize, &frame.size);
        CFRelease(size_ref);
    }

    return frame;
}

CGRect window_frame(struct ax_window *window)
{
    CGRect frame = {};
    SLSGetWindowBounds(g_connection, window->id, &frame);
    return frame;
}

bool window_can_move(struct ax_window *window)
{
    Boolean result;
    if (AXUIElementIsAttributeSettable(window->ref, kAXPositionAttribute, &result) != kAXErrorSuccess) {
        result = 0;
    }
    return result;
}

bool window_can_resize(struct ax_window *window)
{
    Boolean result;
    if (AXUIElementIsAttributeSettable(window->ref, kAXSizeAttribute, &result) != kAXErrorSuccess) {
        result = 0;
    }
    return result;
}

bool window_is_minimized(struct ax_window *window)
{
    Boolean result = 0;
    CFTypeRef value;
    if (AXUIElementCopyAttributeValue(window->ref, kAXMinimizedAttribute, &value) == kAXErrorSuccess) {
        result = CFBooleanGetValue(value);
        CFRelease(value);
    }
    return result || window->is_minimized;
}

bool window_is_fullscreen(struct ax_window *window)
{
    Boolean result = 0;
    CFTypeRef value;
    if (AXUIElementCopyAttributeValue(window->ref, kAXFullscreenAttribute, &value) == kAXErrorSuccess) {
        result = CFBooleanGetValue(value);
        CFRelease(value);
    }
    return result;
}

bool window_is_sticky(struct ax_window *window)
{
    bool result = false;
    CFNumberRef window_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &window->id);
    CFArrayRef window_list_ref = CFArrayCreate(NULL, (void *)&window_id_ref, 1, NULL);
    CFArrayRef space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, window_list_ref);
    if (!space_list_ref) goto err;

    result = CFArrayGetCount(space_list_ref) > 1;

    CFRelease(space_list_ref);
err:
    CFRelease(window_list_ref);
    CFRelease(window_id_ref);
    return result;
}

CFStringRef window_role(struct ax_window *window)
{
    const void *role = NULL;
    AXUIElementCopyAttributeValue(window->ref, kAXRoleAttribute, &role);
    return role;
}

CFStringRef window_subrole(struct ax_window *window)
{
    const void *srole = NULL;
    AXUIElementCopyAttributeValue(window->ref, kAXSubroleAttribute, &srole);
    return srole;
}

bool window_is_standard(struct ax_window *window)
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

bool window_is_dialog(struct ax_window *window)
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

struct ax_window *window_create(struct ax_application *application, AXUIElementRef window_ref)
{
    struct ax_window *window = malloc(sizeof(struct ax_window));
    memset(window, 0, sizeof(struct ax_window));
    window->application = application;
    window->ref = window_ref;
    window->id = ax_window_id(window->ref);
    window->is_minimized = window_is_minimized(window);
    window->id_ptr = malloc(sizeof(uint32_t *));
    *window->id_ptr = &window->id;

    if ((window_is_standard(window)) ||
        (window_is_dialog(window))) {
        border_window_create(window);

        if ((!application->is_hidden) &&
            (!window->is_minimized)) {
            border_window_refresh(window);
        }
    }

    return window;
}

void window_destroy(struct ax_window *window)
{
    border_window_destroy(window);
    CFRelease(window->ref);
    free(window->id_ptr);
    free(window);
}

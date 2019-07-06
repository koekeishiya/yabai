#include "display_manager.h"

extern struct window_manager g_window_manager;
extern int g_connection;

bool display_manager_query_displays(FILE *rsp)
{
    uint32_t count = 0;
    uint32_t *display_list = display_manager_active_display_list(&count);
    if (!display_list) return false;

    fprintf(rsp, "[");
    for (int i = 0; i < count; ++i) {
        display_serialize(rsp, display_list[i]);
        fprintf(rsp, "%c", i < count - 1 ? ',' : ']');
    }
    fprintf(rsp, "\n");

    free(display_list);
    return true;
}

CFStringRef display_manager_main_display_uuid(void)
{
    uint32_t display_id = display_manager_main_display_id();
    return display_uuid(display_id);
}

uint32_t display_manager_main_display_id(void)
{
    return CGMainDisplayID();
}

CFStringRef display_manager_active_display_uuid(void)
{
    return SLSCopyActiveMenuBarDisplayIdentifier(g_connection);
}

uint32_t display_manager_active_display_id(void)
{
    uint32_t result = 0;
    CFStringRef uuid = display_manager_active_display_uuid();
    CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL, uuid);
    result = CGDisplayGetDisplayIDFromUUID(uuid_ref);
    CFRelease(uuid_ref);
    CFRelease(uuid);
    return result;
}

CFStringRef display_manager_dock_display_uuid(void)
{
    if (display_manager_dock_hidden()) return NULL;

    CGRect dock = display_manager_dock_rect();
    return SLSCopyBestManagedDisplayForRect(g_connection, dock);
}

uint32_t display_manager_dock_display_id(void)
{
    CFStringRef uuid = display_manager_dock_display_uuid();
    if (!uuid) return 0;

    CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL, uuid);
    uint32_t result = CGDisplayGetDisplayIDFromUUID(uuid_ref);
    CFRelease(uuid_ref);
    CFRelease(uuid);
    return result;
}

CFStringRef display_manager_arrangement_display_uuid(int arrangement)
{
    CFStringRef result = NULL;
    CFArrayRef displays = SLSCopyManagedDisplays(g_connection);

    int displays_count = CFArrayGetCount(displays);
    for (int i = 0; i < displays_count; ++i) {
        if ((i+1) != arrangement) continue;
        result = CFRetain(CFArrayGetValueAtIndex(displays, i));
        break;
    }

    CFRelease(displays);
    return result;
}

uint32_t display_manager_arrangement_display_id(int arrangement)
{
    uint32_t result = 0;
    CFArrayRef displays = SLSCopyManagedDisplays(g_connection);

    int displays_count = CFArrayGetCount(displays);
    for (int i = 0; i < displays_count; ++i) {
        if ((i+1) != arrangement) continue;
        CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL, CFArrayGetValueAtIndex(displays, i));
        result = CGDisplayGetDisplayIDFromUUID(uuid_ref);
        CFRelease(uuid_ref);
        break;
    }

    CFRelease(displays);
    return result;
}

uint32_t display_manager_prev_display_id(uint32_t did)
{
    int arrangement = display_arrangement(did);
    if (arrangement <= 1) return 0;

    return display_manager_arrangement_display_id(arrangement - 1);
}

uint32_t display_manager_next_display_id(uint32_t did)
{
    int arrangement = display_arrangement(did);
    if (arrangement >= display_manager_active_display_count()) return 0;

    return display_manager_arrangement_display_id(arrangement + 1);
}

uint32_t display_manager_first_display_id(void)
{
    return display_manager_arrangement_display_id(1);
}

uint32_t display_manager_last_display_id(void)
{
    int arrangement = display_manager_active_display_count();
    return display_manager_arrangement_display_id(arrangement);
}

bool display_manager_menu_bar_hidden(void)
{
    int status = 0;
    SLSGetMenuBarAutohideEnabled(g_connection, &status);
    return status;
}

CGRect display_manager_menu_bar_rect(void)
{
    CGRect bounds = {};
    SLSGetRevealedMenuBarBounds(&bounds, g_connection, space_manager_active_space());
    return bounds;
}

bool display_manager_dock_hidden(void)
{
    return CoreDockGetAutoHideEnabled();
}

int display_manager_dock_orientation(void)
{
    int pinning = 0;
    int orientation = 0;
    CoreDockGetOrientationAndPinning(&orientation, &pinning);
    return orientation;
}

CGRect display_manager_dock_rect(void)
{
    int reason = 0;
    CGRect bounds = {};
    SLSGetDockRectWithReason(g_connection, &bounds, &reason);
    return bounds;
}

bool display_manager_active_display_is_animating(void)
{
    CFStringRef uuid = display_manager_active_display_uuid();
    bool result = SLSManagedDisplayIsAnimating(g_connection, uuid);
    CFRelease(uuid);
    return result;
}

bool display_manager_display_is_animating(uint32_t did)
{
    CFStringRef uuid = display_uuid(did);
    if (!uuid) return false;

    bool result = SLSManagedDisplayIsAnimating(g_connection, uuid);
    CFRelease(uuid);
    return result;
}

uint32_t display_manager_active_display_count(void)
{
    uint32_t count;
    CGGetActiveDisplayList(0, NULL, &count);
    return count;
}

uint32_t *display_manager_active_display_list(uint32_t *count)
{
    int display_count = display_manager_active_display_count();
    uint32_t *result = malloc(sizeof(uint32_t) * display_count);
    CGGetActiveDisplayList(display_count, result, count);
    return result;
}

static AXUIElementRef display_manager_find_element_at_point(CGPoint point)
{
    CFTypeRef role;
    CFTypeRef window_ref;
    AXUIElementRef element_ref;

    AXUIElementCopyElementAtPosition(g_window_manager.system_element, point.x, point.y, &element_ref);
    if (!element_ref) return NULL;

    AXUIElementCopyAttributeValue(element_ref, kAXRoleAttribute, &role);
    if (CFEqual(role, kAXWindowRole)) {
        window_ref = element_ref;
    } else {
        AXUIElementCopyAttributeValue(element_ref, kAXWindowAttribute, &window_ref);
        CFRelease(element_ref);
    }

    CFRelease(role);
    return window_ref;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
void display_manager_focus_display(uint32_t display_id)
{
    int window_count;
    uint32_t *window_list;
    struct window *window;

    CGRect bounds;
    CGPoint point;
    uint32_t element_id;
    AXUIElementRef element_ref;

    window_list = space_window_list(display_space_id(display_id), &window_count);
    if (!window_list) goto fallback;

    for (int i = 0; i < window_count; ++i) {
        window = window_manager_find_window(&g_window_manager, window_list[i]);
        if (!window || !window_is_standard(window)) continue;

        window_manager_focus_window_with_raise(window->id);
        free(window_list);
        goto out;
    }

    free(window_list);

fallback:
    bounds = display_bounds(display_id);
    point = (CGPoint) { bounds.origin.x + bounds.size.width / 2, bounds.origin.y + bounds.size.height / 2 };
    element_ref = display_manager_find_element_at_point(point);
    element_id = 0;

    if (element_ref) {
        element_id = ax_window_id(element_ref);
        CFRelease(element_ref);
    }

    if (element_id) {
        window_manager_focus_window_with_raise(element_id);
    } else {
        CGPostMouseEvent(point, true, 1, true);
        CGPostMouseEvent(point, true, 1, false);
    }
out:;
}
#pragma clang diagnostic pop

bool display_manager_begin(struct display_manager *dm)
{
    dm->current_display_id = display_manager_active_display_id();
    dm->last_display_id = dm->current_display_id;
    return CGDisplayRegisterReconfigurationCallback(display_handler, NULL) == kCGErrorSuccess;
}

bool display_manager_end(void)
{
    return CGDisplayRemoveReconfigurationCallback(display_handler, NULL) == kCGErrorSuccess;
}

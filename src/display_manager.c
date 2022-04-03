extern struct display_manager g_display_manager;
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

    return true;
}

CFStringRef display_manager_main_display_uuid(void)
{
    uint32_t did = display_manager_main_display_id();
    return display_uuid(did);
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
    CFStringRef uuid = display_manager_active_display_uuid();
    assert(uuid);

    uint32_t result = display_id(uuid);
    CFRelease(uuid);

    return result;
}

CFStringRef display_manager_dock_display_uuid(void)
{
    CGRect dock = display_manager_dock_rect();
    return SLSCopyBestManagedDisplayForRect(g_connection, dock);
}

uint32_t display_manager_dock_display_id(void)
{
    CFStringRef uuid = display_manager_dock_display_uuid();
    if (!uuid) return 0;

    uint32_t result = display_id(uuid);
    CFRelease(uuid);

    return result;
}

CFStringRef display_manager_point_display_uuid(CGPoint point)
{
    return SLSCopyBestManagedDisplayForPoint(g_connection, point);
}

uint32_t display_manager_point_display_id(CGPoint point)
{
    CFStringRef uuid = display_manager_point_display_uuid(point);
    if (!uuid) return 0;

    uint32_t result = display_id(uuid);
    CFRelease(uuid);

    return result;
}

CFStringRef display_manager_arrangement_display_uuid(int arrangement)
{
    CFStringRef result = NULL;
    CFArrayRef displays = SLSCopyManagedDisplays(g_connection);

    int count = CFArrayGetCount(displays);
    int index = arrangement - 1;

    if (in_range_ie(index, 0, count)) {
        result = CFRetain(CFArrayGetValueAtIndex(displays, index));
    }

    CFRelease(displays);
    return result;
}

uint32_t display_manager_arrangement_display_id(int arrangement)
{
    uint32_t result = 0;
    CFArrayRef displays = SLSCopyManagedDisplays(g_connection);

    int count = CFArrayGetCount(displays);
    int index = arrangement - 1;

    if (in_range_ie(index, 0, count)) {
        result = display_id(CFArrayGetValueAtIndex(displays, index));
    }

    CFRelease(displays);
    return result;
}

uint32_t display_manager_cursor_display_id(void)
{
    CGPoint cursor;
    SLSGetCurrentCursorLocation(g_connection, &cursor);
    return display_manager_point_display_id(cursor);
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

uint32_t display_manager_find_closest_display_in_direction(uint32_t source_did, int direction)
{
    uint32_t display_count;
    uint32_t *display_list = display_manager_active_display_list(&display_count);
    if (!display_list) return 0;

    uint32_t best_did = 0;
    int best_distance = INT_MAX;

    struct area source_area = area_from_cgrect(display_bounds(source_did));
    CGPoint source_area_max = { source_area.x + source_area.w, source_area.y + source_area.h };

    for (int i = 0; i < display_count; ++i) {
        uint32_t did = display_list[i];
        if (did == source_did) continue;

        struct area target_area = area_from_cgrect(display_bounds(did));
        CGPoint target_area_max = { target_area.x + target_area.w, target_area.y + target_area.h };

        if (area_is_in_direction(&source_area, source_area_max, &target_area, target_area_max, direction)) {
            int distance = area_distance_in_direction(&source_area, source_area_max, &target_area, target_area_max, direction);
            if (distance < best_distance) {
                best_did = did;
                best_distance = distance;
            }
        }
    }

    return best_did;
}

bool display_manager_menu_bar_hidden(void)
{
    int status = 0;
    SLSGetMenuBarAutohideEnabled(g_connection, &status);
    return status;
}

CGRect display_manager_menu_bar_rect(uint32_t did)
{
    CGRect bounds = {};

#ifdef __x86_64__
    SLSGetRevealedMenuBarBounds(&bounds, g_connection, display_space_id(did));
#elif __arm64__

    //
    // NOTE(koekeishiya): SLSGetRevealedMenuBarBounds is broken on Apple Silicon and always returns an empty rectangle,
    // The menubar height seems to be a constant of 24em always, regardless of the display being a 13" or 16",
    // so we patch it here. For screens with a notch, the height of the notch determines the lowest y-coordinate
    // that windows can be placed at. The width of the menubar should be equal to the width of the display.
    //

    int notch_height = workspace_display_notch_height(did);
    if (notch_height) {
        bounds.size.height = notch_height + 6;
    } else {
        bounds.size.height = 24;
    }

    bounds.size.width = CGDisplayPixelsWide(did);
#endif

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
    assert(uuid);

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
    uint32_t *result = ts_alloc_aligned(sizeof(uint32_t), display_count);
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
void display_manager_focus_display_with_point(uint32_t did, CGPoint point, bool update_cursor_position)
{
    int element_connection;
    ProcessSerialNumber element_psn;

    AXUIElementRef element_ref = display_manager_find_element_at_point(point);
    if (!element_ref) goto click;

    uint32_t element_id = ax_window_id(element_ref);
    if (!element_id) goto err_ref;

    SLSGetWindowOwner(g_connection, element_id, &element_connection);
    SLSGetConnectionPSN(element_connection, &element_psn);
    window_manager_focus_window_with_raise(&element_psn, element_id, element_ref);
    CFRelease(element_ref);
    goto out;

err_ref:
    CFRelease(element_ref);
click:
    CGPostMouseEvent(point, update_cursor_position, 1, true);
    CGPostMouseEvent(point, update_cursor_position, 1, false);
out:;
}
#pragma clang diagnostic pop

void display_manager_focus_display(uint32_t did)
{
    struct window *window = window_manager_find_window_on_space_by_rank(&g_window_manager, display_space_id(did), 1);
    if (window) {
        window_manager_focus_window_with_raise(&window->application->psn, window->id, window->ref);
    } else {
        display_manager_focus_display_with_point(did, display_center(did), true);
    }
}

bool display_manager_begin(struct display_manager *dm)
{
    dm->current_display_id = display_manager_active_display_id();
    dm->last_display_id = dm->current_display_id;
    dm->mode = EXTERNAL_BAR_OFF;
    dm->top_padding = 0;
    dm->bottom_padding = 0;
    return CGDisplayRegisterReconfigurationCallback(display_handler, NULL) == kCGErrorSuccess;
}

bool display_manager_end(void)
{
    return CGDisplayRemoveReconfigurationCallback(display_handler, NULL) == kCGErrorSuccess;
}

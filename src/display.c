extern struct event_loop g_event_loop;
extern int g_connection;

static DISPLAY_EVENT_HANDLER(display_handler)
{
    if (flags & kCGDisplayAddFlag) {
        event_loop_post(&g_event_loop, DISPLAY_ADDED, (void *)(intptr_t) did, 0, NULL);
    } else if (flags & kCGDisplayRemoveFlag) {
        event_loop_post(&g_event_loop, DISPLAY_REMOVED, (void *)(intptr_t) did, 0, NULL);
    } else if (flags & kCGDisplayMovedFlag) {
        event_loop_post(&g_event_loop, DISPLAY_MOVED, (void *)(intptr_t) did, 0, NULL);
    } else if (flags & kCGDisplayDesktopShapeChangedFlag) {
        event_loop_post(&g_event_loop, DISPLAY_RESIZED, (void *)(intptr_t) did, 0, NULL);
    }
}

void display_serialize(FILE *rsp, uint32_t did)
{
    CGRect frame = display_bounds(did);

    char *uuid = NULL;
    CFStringRef uuid_ref = display_uuid(did);
    if (uuid_ref) {
        uuid = ts_cfstring_copy(uuid_ref);
        CFRelease(uuid_ref);
    }

    int buffer_size = MAXLEN;
    size_t bytes_written = 0;
    char buffer[MAXLEN] = {};
    char *cursor = buffer;

    int count;
    uint64_t *space_list = display_space_list(did, &count);
    if (space_list) {
        int first_mci = space_manager_mission_control_index(space_list[0]);
        for (int i = 0; i < count; ++i) {
            if (i < count - 1) {
                bytes_written = snprintf(cursor, buffer_size, "%d, ", first_mci + i);
            } else {
                bytes_written = snprintf(cursor, buffer_size, "%d", first_mci + i);
            }

            cursor += bytes_written;
            buffer_size -= bytes_written;
            if (buffer_size <= 0) break;
        }
    }

    fprintf(rsp,
            "{\n"
            "\t\"id\":%d,\n"
            "\t\"uuid\":\"%s\",\n"
            "\t\"index\":%d,\n"
            "\t\"frame\":{\n\t\t\"x\":%.4f,\n\t\t\"y\":%.4f,\n\t\t\"w\":%.4f,\n\t\t\"h\":%.4f\n\t},\n"
            "\t\"spaces\":[%s]\n"
            "}",
            did,
            uuid ? uuid : "<unknown>",
            display_arrangement(did),
            frame.origin.x, frame.origin.y, frame.size.width, frame.size.height,
            buffer);
}

CFStringRef display_uuid(uint32_t did)
{
    CFUUIDRef uuid_ref = CGDisplayCreateUUIDFromDisplayID(did);
    if (!uuid_ref) return NULL;

    CFStringRef uuid_str = CFUUIDCreateString(NULL, uuid_ref);
    CFRelease(uuid_ref);

    return uuid_str;
}

uint32_t display_id(CFStringRef uuid)
{
    CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL, uuid);
    if (!uuid_ref) return 0;

    uint32_t did = CGDisplayGetDisplayIDFromUUID(uuid_ref);
    CFRelease(uuid_ref);

    return did;
}

CGRect display_bounds(uint32_t did)
{
    return CGDisplayBounds(did);
}

CGRect display_bounds_constrained(uint32_t did)
{
    CGRect frame  = display_bounds(did);
    int effective_ext_top_padding = 0;

    if ((g_display_manager.mode == EXTERNAL_BAR_MAIN &&
         did == display_manager_main_display_id()) ||
        (g_display_manager.mode == EXTERNAL_BAR_ALL)) {
        effective_ext_top_padding = g_display_manager.top_padding;

        frame.origin.y    += effective_ext_top_padding;
        frame.size.height -= effective_ext_top_padding;
        frame.size.height -= g_display_manager.bottom_padding;
    }

    if (display_manager_menu_bar_hidden()) {
        int notch_height = workspace_display_notch_height(did);
        if (notch_height > effective_ext_top_padding) {
             frame.origin.y    += (notch_height - effective_ext_top_padding);
             frame.size.height -= (notch_height - effective_ext_top_padding);
        }
    } else {
        CGRect menu = display_manager_menu_bar_rect(did);
        frame.origin.y    += menu.size.height;
        frame.size.height -= menu.size.height;
    }

    if (!display_manager_dock_hidden()) {
        if (did == display_manager_dock_display_id()) {
            CGRect dock = display_manager_dock_rect();
            switch (display_manager_dock_orientation()) {
            case DOCK_ORIENTATION_LEFT: {
                frame.origin.x   += dock.size.width;
                frame.size.width -= dock.size.width;
            } break;
            case DOCK_ORIENTATION_RIGHT: {
                frame.size.width -= dock.size.width;
            } break;
            case DOCK_ORIENTATION_BOTTOM: {
                frame.size.height -= dock.size.height;
            } break;
            }
        }
    }

    return frame;
}

CGPoint display_center(uint32_t did)
{
    CGRect bounds = display_bounds(did);
    return (CGPoint) { bounds.origin.x + bounds.size.width/2, bounds.origin.y + bounds.size.height/2 };
}

uint64_t display_space_id(uint32_t did)
{
    CFStringRef uuid = display_uuid(did);
    if (!uuid) return 0;

    uint64_t sid = SLSManagedDisplayGetCurrentSpace(g_connection, uuid);
    CFRelease(uuid);

    return sid;
}

int display_space_count(uint32_t did)
{
    int space_count = 0;

    CFStringRef uuid = display_uuid(did);
    if (!uuid) goto out;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    if (!display_spaces_ref) goto err;

    int display_spaces_count = CFArrayGetCount(display_spaces_ref);
    for (int i = 0; i < display_spaces_count; ++i) {
        CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
        CFStringRef identifier = CFDictionaryGetValue(display_ref, CFSTR("Display Identifier"));
        if (!CFEqual(uuid, identifier)) continue;

        CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
        space_count = CFArrayGetCount(spaces_ref);
        break;
    }

    CFRelease(display_spaces_ref);
err:
    CFRelease(uuid);
out:
    return space_count;
}

uint64_t *display_space_list(uint32_t did, int *count)
{
    uint64_t *space_list = NULL;

    CFStringRef uuid = display_uuid(did);
    if (!uuid) goto out;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    if (!display_spaces_ref) goto err;

    int display_spaces_count = CFArrayGetCount(display_spaces_ref);
    for (int i = 0; i < display_spaces_count; ++i) {
        CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
        CFStringRef identifier = CFDictionaryGetValue(display_ref, CFSTR("Display Identifier"));
        if (!CFEqual(uuid, identifier)) continue;

        CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
        int spaces_count = CFArrayGetCount(spaces_ref);

        space_list = ts_alloc_aligned(sizeof(uint64_t), spaces_count);
        *count = spaces_count;

        for (int j = 0; j < spaces_count; ++j) {
            CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
            CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
            CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &space_list[j]);
        }
    }

    CFRelease(display_spaces_ref);
err:
    CFRelease(uuid);
out:
    return space_list;
}

int display_arrangement(uint32_t did)
{
    int result = 0;

    CFStringRef uuid = display_uuid(did);
    if (!uuid) goto out;

    CFArrayRef displays = SLSCopyManagedDisplays(g_connection);
    if (!displays) goto err;

    int displays_count = CFArrayGetCount(displays);
    for (int i = 0; i < displays_count; ++i) {
        if (CFEqual(CFArrayGetValueAtIndex(displays, i), uuid)) {
            result = i + 1;
            break;
        }
    }

    CFRelease(displays);
err:
    CFRelease(uuid);
out:
    return result;
}

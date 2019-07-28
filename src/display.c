#include "display.h"

extern struct event_loop g_event_loop;
extern struct bar g_bar;
extern int g_connection;

static DISPLAY_EVENT_HANDLER(display_handler)
{
    struct event *event;

    if (flags & kCGDisplayAddFlag) {
        event_create(event, DISPLAY_ADDED, (void *)(intptr_t) display_id);
    } else if (flags & kCGDisplayRemoveFlag) {
        event_create(event, DISPLAY_REMOVED, (void *)(intptr_t) display_id);
    } else if (flags & kCGDisplayMovedFlag) {
        event_create(event, DISPLAY_MOVED, (void *)(intptr_t) display_id);
    } else if (flags & kCGDisplayDesktopShapeChangedFlag) {
        event_create(event, DISPLAY_RESIZED, (void *)(intptr_t) display_id);
    } else {
        event = NULL;
    }

    if (event) event_loop_post(&g_event_loop, event);
}

void display_serialize(FILE *rsp, uint32_t did)
{
    CGRect frame = display_bounds(did);

    int buffer_size = MAXLEN;
    size_t bytes_written = 0;
    char buffer[MAXLEN] = {};
    char *cursor = buffer;

    int count;
    uint64_t *space_list = display_space_list(did, &count);
    if (space_list) {
        for (int i = 0; i < count; ++i) {
            if (i < count - 1) {
                bytes_written = snprintf(cursor, buffer_size, "%d, ", space_manager_mission_control_index(space_list[i]));
            } else {
                bytes_written = snprintf(cursor, buffer_size, "%d", space_manager_mission_control_index(space_list[i]));
            }

            cursor += bytes_written;
            buffer_size -= bytes_written;
            if (buffer_size <= 0) break;
        }
        free(space_list);
    }

    fprintf(rsp,
            "{\n"
            "\t\"id\":%d,\n"
            "\t\"index\":%d,\n"
            "\t\"spaces\":[%s],\n"
            "\t\"frame\":{\n\t\t\"x\":%.4f,\n\t\t\"y\":%.4f,\n\t\t\"w\":%.4f,\n\t\t\"h\":%.4f\n\t}\n"
            "}",
            did, display_arrangement(did), buffer,
            frame.origin.x, frame.origin.y,
            frame.size.width, frame.size.height);
}

CFStringRef display_uuid(uint32_t did)
{
    CFUUIDRef uuid_ref = CGDisplayCreateUUIDFromDisplayID(did);
    if (!uuid_ref) return NULL;

    CFStringRef uuid_str = CFUUIDCreateString(NULL, uuid_ref);
    CFRelease(uuid_ref);

    return uuid_str;
}

CGRect display_bounds(uint32_t did)
{
    return CGDisplayBounds(did);
}

CGRect display_bounds_constrained(uint32_t did)
{
    CGRect frame  = display_bounds(did);
    CGRect menu   = display_manager_menu_bar_rect();
    CGRect dock   = display_manager_dock_rect();

    if (g_bar.enabled && did == display_manager_main_display_id()) {
        frame.origin.y    += g_bar.frame.size.height;
        frame.size.height -= g_bar.frame.size.height;
    }

    if (!display_manager_menu_bar_hidden()) {
        frame.origin.y    += menu.size.height;
        frame.size.height -= menu.size.height;
    }

    if (!display_manager_dock_hidden()) {
        if (did == display_manager_dock_display_id()) {
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
    CFStringRef uuid = display_uuid(did);
    if (!uuid) return 0;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    if (!display_spaces_ref) return 0;

    int space_count = 0;
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
    CFRelease(uuid);
    return space_count;
}

uint64_t *display_space_list(uint32_t did, int *count)
{
    CFStringRef uuid = display_uuid(did);
    if (!uuid) return NULL;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    if (!display_spaces_ref) return NULL;

    uint64_t *space_list = NULL;
    int display_spaces_count = CFArrayGetCount(display_spaces_ref);

    for (int i = 0; i < display_spaces_count; ++i) {
        CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
        CFStringRef identifier = CFDictionaryGetValue(display_ref, CFSTR("Display Identifier"));
        if (!CFEqual(uuid, identifier)) continue;

        CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
        int spaces_count = CFArrayGetCount(spaces_ref);

        space_list = malloc(sizeof(uint64_t) * spaces_count);
        *count = spaces_count;

        for (int j = 0; j < spaces_count; ++j) {
            CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
            CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
            CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &space_list[j]);
        }
    }

    CFRelease(display_spaces_ref);
    CFRelease(uuid);

    return space_list;
}

int display_arrangement(uint32_t did)
{
    CFStringRef uuid = display_uuid(did);
    if (!uuid) return 0;

    CFArrayRef displays = SLSCopyManagedDisplays(g_connection);
    if (!displays) return 0;

    int result = 0;
    int displays_count = CFArrayGetCount(displays);

    for (int i = 0; i < displays_count; ++i) {
        if (CFEqual(CFArrayGetValueAtIndex(displays, i), uuid)) {
            result = i + 1;
            break;
        }
    }

    CFRelease(displays);
    CFRelease(uuid);
    return result;
}

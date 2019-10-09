#include "event_tap.h"

extern struct event_loop g_event_loop;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
static EVENT_TAP_CALLBACK(mouse_handler)
{
    switch (type) {
    case kCGEventTapDisabledByTimeout:
    case kCGEventTapDisabledByUserInput: {
        struct event_tap *event_tap = (struct event_tap *) reference;
        CGEventTapEnable(event_tap->handle, 1);
    } break;
    case kCGEventLeftMouseDown:
    case kCGEventRightMouseDown: {
        volatile int status = EVENT_QUEUED;
        volatile int result = EVENT_SUCCESS;

        struct event *event;
        event_create(event, MOUSE_DOWN, (void *) CGEventCreateCopy(cgevent));
        event->status = &status;
        event->result = &result;
        event_loop_post(&g_event_loop, event);
        while (status == EVENT_QUEUED);

        if (result == EVENT_MOUSE_IGNORE) return NULL;
    } break;
    case kCGEventLeftMouseUp:
    case kCGEventRightMouseUp: {
        struct event *event;
        event_create(event, MOUSE_UP, (void *) CGEventCreateCopy(cgevent));
        event_loop_post(&g_event_loop, event);
    } break;
    case kCGEventLeftMouseDragged:
    case kCGEventRightMouseDragged: {
        struct event *event;
        event_create(event, MOUSE_DRAGGED, (void *) CGEventCreateCopy(cgevent));
        event_loop_post(&g_event_loop, event);
    } break;
    case kCGEventMouseMoved: {
        struct event *event;
        event_create(event, MOUSE_MOVED, (void *) CGEventCreateCopy(cgevent));
        event_loop_post(&g_event_loop, event);
    } break;
    }

    return cgevent;
}
#pragma clang diagnostic pop

bool event_tap_enabled(struct event_tap *event_tap)
{
    bool result = (event_tap->handle && CGEventTapIsEnabled(event_tap->handle));
    return result;
}

bool event_tap_begin(struct event_tap *event_tap, uint32_t mask, event_tap_callback *callback)
{
    event_tap->mask = mask;
    event_tap->handle = CGEventTapCreate(kCGSessionEventTap,
                                         kCGHeadInsertEventTap,
                                         kCGEventTapOptionDefault,
                                         event_tap->mask,
                                         callback,
                                         event_tap);

    bool result = event_tap_enabled(event_tap);
    if (result) {
        event_tap->runloop_source = CFMachPortCreateRunLoopSource(NULL, event_tap->handle, 0);
        CFRunLoopAddSource(CFRunLoopGetMain(), event_tap->runloop_source, kCFRunLoopCommonModes);
    }

    return result;
}

void event_tap_end(struct event_tap *event_tap)
{
    if (event_tap_enabled(event_tap)) {
        CGEventTapEnable(event_tap->handle, false);
        CFMachPortInvalidate(event_tap->handle);
        CFRunLoopRemoveSource(CFRunLoopGetMain(), event_tap->runloop_source, kCFRunLoopCommonModes);
        CFRelease(event_tap->runloop_source);
        CFRelease(event_tap->handle);
        event_tap->handle = NULL;
    }
}

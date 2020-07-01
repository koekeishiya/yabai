#include "event_tap.h"

extern struct event_loop g_event_loop;
extern struct mouse_state g_mouse_state;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
static bool consume_mouse_click;
static EVENT_TAP_CALLBACK(mouse_handler)
{
    switch (type) {
    case kCGEventTapDisabledByTimeout:
    case kCGEventTapDisabledByUserInput: {
        struct event_tap *event_tap = (struct event_tap *) reference;
        CGEventTapEnable(event_tap->handle, 1);
    } break;
    case kCGEventLeftMouseDown:
    case kCGEventRightMouseDown:
    case kCGEventOtherMouseDown: {
        uint8_t mod = mouse_mod_from_cgflags(CGEventGetFlags(cgevent));
        enum mouse_button button =
              type == kCGEventLeftMouseDown ? MOUSE_BUTTON_LEFT
            : type == kCGEventRightMouseDown ? MOUSE_BUTTON_RIGHT
            : type == kCGEventOtherMouseDown ? MOUSE_BUTTON_MIDDLE
            : MOUSE_BUTTON_NONE;
        consume_mouse_click = mouse_action_get(&g_mouse_state, mod, button) != MOUSE_ACTION_NONE;
        event_loop_post(&g_event_loop, MOUSE_DOWN, (void *) CFRetain(cgevent), mod, NULL);
        if (consume_mouse_click) return NULL;
    } break;
    case kCGEventLeftMouseUp:
    case kCGEventRightMouseUp:
    case kCGEventOtherMouseUp: {
        event_loop_post(&g_event_loop, MOUSE_UP, (void *) CFRetain(cgevent), 0, NULL);
        if (consume_mouse_click) return NULL;
    } break;
    case kCGEventLeftMouseDragged:
    case kCGEventRightMouseDragged:
    case kCGEventOtherMouseDragged: {
        event_loop_post(&g_event_loop, MOUSE_DRAGGED, (void *) CFRetain(cgevent), 0, NULL);
    } break;
    case kCGEventMouseMoved: {
        event_loop_post(&g_event_loop, MOUSE_MOVED, (void *) CFRetain(cgevent), 0, NULL);
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

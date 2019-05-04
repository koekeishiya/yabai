#ifndef EVENT_TAP_H
#define EVENT_TAP_H

#include <stdbool.h>
#include <Carbon/Carbon.h>

struct event_tap
{
    CFMachPortRef handle;
    CFRunLoopSourceRef runloop_source;
    CGEventMask mask;
};

#define EVENT_TAP_CALLBACK(name) \
    CGEventRef name(CGEventTapProxy proxy, \
                    CGEventType type, \
                    CGEventRef cgevent, \
                    void *reference)
typedef EVENT_TAP_CALLBACK(event_tap_callback);

bool event_tap_enabled(struct event_tap *event_tap);
bool event_tap_begin(struct event_tap *event_tap, uint32_t mask, event_tap_callback *callback);
void event_tap_end(struct event_tap *event_tap);

#endif

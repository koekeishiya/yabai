#ifndef EVENT_TAP_H
#define EVENT_TAP_H

#define EVENT_MASK_MOUSE_FFM (1 << kCGEventMouseMoved) | \
                             (1 << kCGEventLeftMouseDown) | \
                             (1 << kCGEventLeftMouseUp) | \
                             (1 << kCGEventLeftMouseDragged) | \
                             (1 << kCGEventRightMouseDown) | \
                             (1 << kCGEventRightMouseUp) | \
                             (1 << kCGEventRightMouseDragged)

#define EVENT_MASK_MOUSE     (1 << kCGEventLeftMouseDown) | \
                             (1 << kCGEventLeftMouseUp) | \
                             (1 << kCGEventLeftMouseDragged) | \
                             (1 << kCGEventRightMouseDown) | \
                             (1 << kCGEventRightMouseUp) | \
                             (1 << kCGEventRightMouseDragged)

#define EVENT_MASK_ALT   kCGEventFlagMaskAlternate
#define EVENT_MASK_SHIFT kCGEventFlagMaskShift
#define EVENT_MASK_CMD   kCGEventFlagMaskCommand
#define EVENT_MASK_CTRL  kCGEventFlagMaskControl
#define EVENT_MASK_FN    kCGEventFlagMaskSecondaryFn

struct event_tap
{
    CFMachPortRef handle;
    CFRunLoopSourceRef runloop_source;
};

#define EVENT_TAP_CALLBACK(name) \
    CGEventRef name(CGEventTapProxy proxy, \
                    CGEventType type, \
                    CGEventRef cgevent, \
                    void *reference)
typedef EVENT_TAP_CALLBACK(event_tap_callback);

bool event_tap_begin(struct event_tap *event_tap, uint32_t mask, event_tap_callback *callback);
void event_tap_end(struct event_tap *event_tap);

#endif

extern struct event_loop g_event_loop;
extern struct mouse_state g_mouse_state;
extern pid_t g_pid;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
static bool consume_mouse_click;
static EVENT_TAP_CALLBACK(mouse_handler)
{
    switch (type) {
    case kCGEventTapDisabledByTimeout:
    case kCGEventTapDisabledByUserInput: {
        struct event_tap *event_tap = (struct event_tap *) reference;
        if (event_tap->handle) CGEventTapEnable(event_tap->handle, true);
    } break;
    case kCGEventLeftMouseDown:
    case kCGEventRightMouseDown: {

        //
        // :SynthesizedEvent
        //
        // NOTE(koekeishiya): This event-tap is placed at the "Annotated Session" location, which
        // causes yabai to also intercept events that we post (notably, the way in which window
        // focusing between separate applications have been implemented). Skip these events...
        //

        pid_t source_pid = CGEventGetIntegerValueField(cgevent, kCGEventSourceUnixProcessID);
        if (source_pid == g_pid) return cgevent;

        uint8_t mod = mouse_mod_from_cgflags(CGEventGetFlags(cgevent));
        event_loop_post(&g_event_loop, MOUSE_DOWN, (void *) CFRetain(cgevent), mod, NULL);

        consume_mouse_click = mod == g_mouse_state.modifier;
        if (consume_mouse_click) return NULL;
    } break;
    case kCGEventLeftMouseUp:
    case kCGEventRightMouseUp: {

        //
        // :SynthesizedEvent
        //
        // NOTE(koekeishiya): This event-tap is placed at the "Annotated Session" location, which
        // causes yabai to also intercept events that we post (notably, the way in which window
        // focusing between separate applications have been implemented). Skip these events...
        //

        pid_t source_pid = CGEventGetIntegerValueField(cgevent, kCGEventSourceUnixProcessID);
        if (source_pid == g_pid) return cgevent;

        event_loop_post(&g_event_loop, MOUSE_UP, (void *) CFRetain(cgevent), 0, NULL);
        if (consume_mouse_click) return NULL;
    } break;
    case kCGEventLeftMouseDragged:
    case kCGEventRightMouseDragged: {
        event_loop_post(&g_event_loop, MOUSE_DRAGGED, (void *) CFRetain(cgevent), 0, NULL);
    } break;
    case kCGEventMouseMoved: {
        uint8_t mod = mouse_mod_from_cgflags(CGEventGetFlags(cgevent));
        if (mod == g_mouse_state.modifier) return cgevent;

        event_loop_post(&g_event_loop, MOUSE_MOVED, (void *) CFRetain(cgevent), mod, NULL);
    } break;
    }

    return cgevent;
}
#pragma clang diagnostic pop

bool event_tap_begin(struct event_tap *event_tap, uint32_t mask, event_tap_callback *callback)
{
    if (event_tap->handle) return true;

    event_tap->handle = CGEventTapCreate(kCGAnnotatedSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, mask, callback, event_tap);
    if (!event_tap->handle) return false;

    if (!CGEventTapIsEnabled(event_tap->handle)) {
        CFMachPortInvalidate(event_tap->handle);
        CFRelease(event_tap->handle);
        return false;
    }

    event_tap->runloop_source = CFMachPortCreateRunLoopSource(NULL, event_tap->handle, 0);
    CFRunLoopAddSource(CFRunLoopGetMain(), event_tap->runloop_source, kCFRunLoopCommonModes);

    return true;
}

void event_tap_end(struct event_tap *event_tap)
{
    if (!event_tap->handle) return;

    CGEventTapEnable(event_tap->handle, false);
    CFMachPortInvalidate(event_tap->handle);
    CFRunLoopRemoveSource(CFRunLoopGetMain(), event_tap->runloop_source, kCFRunLoopCommonModes);
    CFRelease(event_tap->runloop_source);
    CFRelease(event_tap->handle);
    event_tap->handle = NULL;
}

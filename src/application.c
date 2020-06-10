#include "application.h"

extern struct event_loop g_event_loop;

static OBSERVER_CALLBACK(application_notification_handler)
{
    if (CFEqual(notification, kAXCreatedNotification)) {
        struct event *event = event_create(&g_event_loop, WINDOW_CREATED, (void *) CFRetain(element));
        event_loop_post(&g_event_loop, event);
    } else if (CFEqual(notification, kAXUIElementDestroyedNotification)) {
        uint32_t *window_id_ptr = *(uint32_t **) context;
        if (!window_id_ptr) return;

        uint32_t window_id = *window_id_ptr;
        while (!__sync_bool_compare_and_swap((uint32_t **) context, window_id_ptr, NULL));

        struct event *event = event_create(&g_event_loop, WINDOW_DESTROYED, (void *)(uintptr_t) window_id);
        event_loop_post(&g_event_loop, event);
    } else if (CFEqual(notification, kAXFocusedWindowChangedNotification)) {
        uint32_t window_id = ax_window_id(element);
        if (!window_id) return;

        struct event *event = event_create(&g_event_loop, WINDOW_FOCUSED, (void *)(intptr_t) window_id);
        event_loop_post(&g_event_loop, event);
    } else if (CFEqual(notification, kAXWindowMovedNotification)) {
        uint32_t window_id = ax_window_id(element);
        if (!window_id) return;

        struct event *event = event_create(&g_event_loop, WINDOW_MOVED, (void *)(intptr_t) window_id);
        event_loop_post(&g_event_loop, event);
    } else if (CFEqual(notification, kAXWindowResizedNotification)) {
        uint32_t window_id = ax_window_id(element);
        if (!window_id) return;

        struct event *event = event_create(&g_event_loop, WINDOW_RESIZED, (void *)(intptr_t) window_id);
        event_loop_post(&g_event_loop, event);
    } else if (CFEqual(notification, kAXWindowMiniaturizedNotification)) {
        uint32_t window_id = **((uint32_t **) context);
        struct event *event = event_create(&g_event_loop, WINDOW_MINIMIZED, (void *)(intptr_t) window_id);
        event_loop_post(&g_event_loop, event);
    } else if (CFEqual(notification, kAXWindowDeminiaturizedNotification)) {
        uint32_t window_id = **((uint32_t **) context);
        struct event *event = event_create(&g_event_loop, WINDOW_DEMINIMIZED, (void *)(intptr_t) window_id);
        event_loop_post(&g_event_loop, event);
    } else if (CFEqual(notification, kAXTitleChangedNotification)) {
        uint32_t window_id = ax_window_id(element);
        if (!window_id) return;

        struct event *event = event_create(&g_event_loop, WINDOW_TITLE_CHANGED, (void *)(intptr_t) window_id);
        event_loop_post(&g_event_loop, event);
    } else if (CFEqual(notification, kAXMenuOpenedNotification)) {
        uint32_t window_id = ax_window_id(element);
        if (!window_id) return;

        struct event *event = event_create(&g_event_loop, MENU_OPENED, (void *)(intptr_t) window_id);
        event_loop_post(&g_event_loop, event);
    }
}

static void
application_observe_notification(struct application *application, int notification)
{
    AXError result = _AXObserverAddNotification(application->observer_ref, application->ref, ax_application_notification[notification], application);
    if (result == kAXErrorSuccess || result == kAXErrorNotificationAlreadyRegistered) {
        application->notification |= 1 << notification;
    } else {
        if (result == kAXErrorCannotComplete) application->ax_retry = true;
        debug("%s: %s failed with error %s\n", __FUNCTION__, ax_application_notification_str[notification], ax_error_str[-result]);
    }
}

static void
application_unobserve_notification(struct application *application, int notification)
{
    _AXObserverRemoveNotification(application->observer_ref, application->ref, ax_application_notification[notification]);
    application->notification &= ~(1 << notification);
}

bool application_observe(struct application *application)
{
    if (AXObserverCreate(application->pid, application_notification_handler, &application->observer_ref) == kAXErrorSuccess) {
        for (int i = 0; i < array_count(ax_application_notification); ++i) {
            application_observe_notification(application, i);
        }

        application->is_observing = true;
        CFRunLoopAddSource(CFRunLoopGetMain(), AXObserverGetRunLoopSource(application->observer_ref), kCFRunLoopDefaultMode);
    }

    return (application->notification & AX_APPLICATION_ALL) == AX_APPLICATION_ALL;
}

void application_unobserve(struct application *application)
{
    if (application->is_observing) {
        for (int i = 0; i < array_count(ax_application_notification); ++i) {
            if (!(application->notification & (1 << i))) continue;
            application_unobserve_notification(application, i);
        }

        application->is_observing = false;
        CFRunLoopSourceInvalidate(AXObserverGetRunLoopSource(application->observer_ref));
        CFRelease(application->observer_ref);
    }
}

uint32_t application_main_window(struct application *application)
{
    CFTypeRef window_ref;
    bool result = AXUIElementCopyAttributeValue(application->ref, kAXMainWindowAttribute, &window_ref) == kAXErrorSuccess;
    if (!result) return 0;

    uint32_t window_id = ax_window_id(window_ref);
    CFRelease(window_ref);

    return window_id;
}

uint32_t application_focused_window(struct application *application)
{
    CFTypeRef window_ref = NULL;
    AXUIElementCopyAttributeValue(application->ref, kAXFocusedWindowAttribute, &window_ref);
    if (!window_ref) return 0;

    uint32_t window_id = ax_window_id(window_ref);
    CFRelease(window_ref);

    return window_id;
}

bool application_is_frontmost(struct application *application)
{
    ProcessSerialNumber psn = {};
    _SLPSGetFrontProcess(&psn);
    return psn_equals(&psn, &application->psn);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
bool application_is_hidden(struct application *application)
{
    return IsProcessVisible(&application->psn) == 0;
}
#pragma clang diagnostic pop

CFArrayRef application_window_list(struct application *application)
{
    CFTypeRef window_list_ref = NULL;
    AXUIElementCopyAttributeValue(application->ref, kAXWindowsAttribute, &window_list_ref);
    return window_list_ref;
}

struct application *application_create(struct process *process)
{
    struct application *application = malloc(sizeof(struct application));
    memset(application, 0, sizeof(struct application));
    application->ref = AXUIElementCreateApplication(process->pid);
    application->psn = process->psn;
    application->pid = process->pid;
    application->name = process->name;
    application->is_hidden = application_is_hidden(application);
    return application;
}

void application_destroy(struct application *application)
{
    CFRelease(application->ref);
    free(application);
}

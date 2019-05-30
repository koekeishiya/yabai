#include "application.h"

extern struct eventloop g_eventloop;

static OBSERVER_CALLBACK(application_notification_handler)
{
    if (CFEqual(notification, kAXCreatedNotification)) {
        struct event *event;
        event_create(event, WINDOW_CREATED, (void *) CFRetain(element));
        eventloop_post(&g_eventloop, event);
    } else if (CFEqual(notification, kAXUIElementDestroyedNotification)) {
        /*
         * NOTE(koekeishiya): If this is an actual window, it should be associated
         * with a valid CGWindowID. HOWEVER, because the window in question has been
         * destroyed. We are unable to utilize this window reference with the AX API.
         *
         * The 'CFEqual()' function can still be used to compare this AXUIElementRef
         * with any existing window refs that we may have. There are a couple of ways
         * we can use to track if an actual window is closed.
         *
         *   a) Store all window AXUIElementRefs in a local cache that we update upon
         *      creation and removal. Requires unsorted container with custom comparator
         *      that uses 'CFEqual()' to match AXUIElementRefs.
         *
         *   b) Instead of tracking 'kAXUIElementDestroyedNotification' for an application,
         *      we have to register this notification separately for every window created.
         *      By doing this, we can pass our own data containing the information necessary
         *      to properly identify and report which window was destroyed.
         *
         * At the very least, we need to know the windowid of the destroyed window.
         */

        /* NOTE(koekeishiya): Option 'b' has been implemented. Leave note for future reference. */

        uint32_t *window_id_ptr = *(uint32_t **) context;
        if (!window_id_ptr) return;

        uint32_t window_id = *window_id_ptr;
        while (!__sync_bool_compare_and_swap((uint32_t **) context, window_id_ptr, NULL));

        struct event *event;
        event_create(event, WINDOW_DESTROYED, (void *)(uintptr_t) window_id);
        eventloop_post(&g_eventloop, event);
    } else if (CFEqual(notification, kAXFocusedWindowChangedNotification)) {
        /*
         * NOTE(koekeishiya): We have to make sure that we can actually interact with the window.
         * When a window is created, we receive this notification before kAXWindowCreatedNotification.
         * When a window is deminimized, we receive this notification before the window is visible.
         */

        uint32_t window_id = ax_window_id(element);
        if (!window_id) return;

        struct event *event;
        event_create(event, WINDOW_FOCUSED, (void *)(intptr_t) window_id);
        eventloop_post(&g_eventloop, event);
    } else if (CFEqual(notification, kAXWindowMovedNotification)) {
        uint32_t window_id = ax_window_id(element);
        if (!window_id) return;

        struct event *event;
        event_create(event, WINDOW_MOVED, (void *)(intptr_t) window_id);
        eventloop_post(&g_eventloop, event);
    } else if (CFEqual(notification, kAXWindowResizedNotification)) {
        uint32_t window_id = ax_window_id(element);
        if (!window_id) return;

        struct event *event;
        event_create(event, WINDOW_RESIZED, (void *)(intptr_t) window_id);
        eventloop_post(&g_eventloop, event);
    } else if (CFEqual(notification, kAXWindowMiniaturizedNotification)) {
        /*
         * NOTE(koekeishiya): We cannot register this notification globally for an application.
         * The AXUIElementRef 'Element' we receive cannot be used with 'AXLibGetWindowID', because
         * a window that is minimized often return a CGWindowID of 0. We have to register this
         * notification for every window such that we can pass our own cached window-information.
         */

        struct event *event;
        uint32_t window_id = **((uint32_t **) context);
        event_create(event, WINDOW_MINIMIZED, (void *)(intptr_t) window_id);
        eventloop_post(&g_eventloop, event);
    } else if (CFEqual(notification, kAXWindowDeminiaturizedNotification)) {
        /*
         * NOTE(koekeishiya): when a deminimized window pulls the user to the space of that window,
         * we receive this notification before 'didActiveSpaceChangeNotification'.
         *
         * This does NOT happen if a window is deminimized by cmd-clicking the window. The window
         * will be deminimized on the currently active space, and no space change occur.
         */

        struct event *event;
        uint32_t window_id = **((uint32_t **) context);
        event_create(event, WINDOW_DEMINIMIZED, (void *)(intptr_t) window_id);
        eventloop_post(&g_eventloop, event);
    } else if (CFEqual(notification, kAXTitleChangedNotification)) {
        uint32_t window_id = ax_window_id(element);
        if (!window_id) return;

        struct event *event;
        event_create(event, WINDOW_TITLE_CHANGED, (void *)(intptr_t) window_id);
        eventloop_post(&g_eventloop, event);
    } else if (CFEqual(notification, kAXMenuOpenedNotification)) {
        uint32_t window_id = ax_window_id(element);
        if (!window_id) return;

        struct event *event;
        event_create(event, MENU_OPENED, (void *)(intptr_t) window_id);
        eventloop_post(&g_eventloop, event);
    }
}

static void
application_observe_notification(struct ax_application *application, int notification)
{
    AXError result = AXObserverAddNotification(application->observer_ref, application->ref, ax_application_notification[notification], application);
    if (result == kAXErrorSuccess || result == kAXErrorNotificationAlreadyRegistered) application->notification |= 1 << notification;
}

static void
application_unobserve_notification(struct ax_application *application, int notification)
{
    AXObserverRemoveNotification(application->observer_ref, application->ref, ax_application_notification[notification]);
    application->notification &= ~(1 << notification);
}

bool application_observe(struct ax_application *application)
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

void application_unobserve(struct ax_application *application)
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

uint32_t application_main_window(struct ax_application *application)
{
    CFTypeRef window_ref;
    bool result = AXUIElementCopyAttributeValue(application->ref, kAXMainWindowAttribute, &window_ref) == kAXErrorSuccess;
    if (!result) return 0;

    uint32_t window_id = ax_window_id(window_ref);
    CFRelease(window_ref);

    return window_id;
}

uint32_t application_focused_window(struct ax_application *application)
{
    CFTypeRef window_ref;
    bool result = AXUIElementCopyAttributeValue(application->ref, kAXFocusedWindowAttribute, &window_ref) == kAXErrorSuccess;
    if (!result) return 0;

    uint32_t window_id = ax_window_id(window_ref);
    CFRelease(window_ref);

    return window_id;
}

bool application_is_frontmost(struct ax_application *application)
{
    ProcessSerialNumber psn = {};
    _SLPSGetFrontProcess(&psn);
    return psn.lowLongOfPSN == application->psn.lowLongOfPSN;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
bool application_is_hidden(struct ax_application *application)
{
    return IsProcessVisible(&application->psn) == 0;
}
#pragma clang diagnostic pop

struct ax_window **application_window_list(struct ax_application *application, int *window_count)
{
    CFTypeRef window_list_ref = NULL;
    AXUIElementCopyAttributeValue(application->ref, kAXWindowsAttribute, &window_list_ref);
    if (!window_list_ref) return NULL;

    *window_count = CFArrayGetCount(window_list_ref);
    struct ax_window **window_list = malloc((*window_count) * sizeof(struct ax_window *));

    for (int i = 0; i < *window_count; ++i) {
        window_list[i] = window_create(application, CFRetain(CFArrayGetValueAtIndex(window_list_ref, i)));
    }

    CFRelease(window_list_ref);
    return window_list;
}

struct ax_application *application_create(struct process *process)
{
    struct ax_application *application = malloc(sizeof(struct ax_application));
    memset(application, 0, sizeof(struct ax_application));
    application->ref = AXUIElementCreateApplication(process->pid);
    application->psn = process->psn;
    application->pid = process->pid;
    application->name = process->name;
    application->is_hidden = application_is_hidden(application);
    return application;
}

void application_destroy(struct ax_application *application)
{
    CFRelease(application->ref);
    free(application);
}

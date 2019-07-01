#ifndef APPLICATION_H
#define APPLICATION_H

#define OBSERVER_CALLBACK(name) void name(AXObserverRef observer, AXUIElementRef element, CFStringRef notification, void *context)
typedef OBSERVER_CALLBACK(observer_callback);

#define application_WINDOW_CREATED_INDEX       0
#define application_WINDOW_FOCUSED_INDEX       1
#define application_WINDOW_MOVED_INDEX         2
#define application_WINDOW_RESIZED_INDEX       3
#define application_WINDOW_TITLE_CHANGED_INDEX 4
#define application_WINDOW_MENU_OPENED_INDEX   5

#define application_WINDOW_CREATED       (1 << application_WINDOW_CREATED_INDEX)
#define application_WINDOW_FOCUSED       (1 << application_WINDOW_FOCUSED_INDEX)
#define application_WINDOW_MOVED         (1 << application_WINDOW_MOVED_INDEX)
#define application_WINDOW_RESIZED       (1 << application_WINDOW_RESIZED_INDEX)
#define application_WINDOW_TITLE_CHANGED (1 << application_WINDOW_TITLE_CHANGED_INDEX)
#define application_ALL                  (application_WINDOW_CREATED |\
                                             application_WINDOW_FOCUSED |\
                                             application_WINDOW_MOVED |\
                                             application_WINDOW_RESIZED |\
                                             application_WINDOW_TITLE_CHANGED)

static const CFStringRef kAXFocusedTabChangedNotification = CFSTR("AXFocusedTabChanged");

static CFStringRef application_notification[] =
{
    [application_WINDOW_CREATED_INDEX]       = kAXCreatedNotification,
    [application_WINDOW_FOCUSED_INDEX]       = kAXFocusedWindowChangedNotification,
    [application_WINDOW_MOVED_INDEX]         = kAXWindowMovedNotification,
    [application_WINDOW_RESIZED_INDEX]       = kAXWindowResizedNotification,
    [application_WINDOW_TITLE_CHANGED_INDEX] = kAXTitleChangedNotification,
    [application_WINDOW_MENU_OPENED_INDEX]   = kAXMenuOpenedNotification
};

struct application
{
    AXUIElementRef ref;
    ProcessSerialNumber psn;
    uint32_t pid;
    char *name;
    AXObserverRef observer_ref;
    uint8_t notification;
    bool is_observing;
    bool is_hidden;
    bool retry;
};

bool application_is_frontmost(struct application *application);
bool application_is_hidden(struct application *application);
uint32_t application_main_window(struct application *application);
uint32_t application_focused_window(struct application *application);
struct window **application_window_list(struct application *application, int *window_count);
bool application_observe(struct application *application);
void application_unobserve(struct application *application);
struct application *application_create(struct process *process);
void application_destroy(struct application *application);

#endif

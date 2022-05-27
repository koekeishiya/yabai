#ifndef APPLICATION_H
#define APPLICATION_H

#define OBSERVER_CALLBACK(name) void name(AXObserverRef observer, AXUIElementRef element, CFStringRef notification, void *context)
typedef OBSERVER_CALLBACK(observer_callback);

#define AX_APPLICATION_WINDOW_CREATED_INDEX       0
#define AX_APPLICATION_WINDOW_FOCUSED_INDEX       1
#define AX_APPLICATION_WINDOW_MOVED_INDEX         2
#define AX_APPLICATION_WINDOW_RESIZED_INDEX       3
#define AX_APPLICATION_WINDOW_TITLE_CHANGED_INDEX 4
#define AX_APPLICATION_WINDOW_MENU_OPENED_INDEX   5
#define AX_APPLICATION_WINDOW_MENU_CLOSED_INDEX   6

#define AX_APPLICATION_WINDOW_CREATED       (1 << AX_APPLICATION_WINDOW_CREATED_INDEX)
#define AX_APPLICATION_WINDOW_FOCUSED       (1 << AX_APPLICATION_WINDOW_FOCUSED_INDEX)
#define AX_APPLICATION_WINDOW_MOVED         (1 << AX_APPLICATION_WINDOW_MOVED_INDEX)
#define AX_APPLICATION_WINDOW_RESIZED       (1 << AX_APPLICATION_WINDOW_RESIZED_INDEX)
#define AX_APPLICATION_WINDOW_TITLE_CHANGED (1 << AX_APPLICATION_WINDOW_TITLE_CHANGED_INDEX)
#define AX_APPLICATION_ALL                  (AX_APPLICATION_WINDOW_CREATED |\
                                             AX_APPLICATION_WINDOW_FOCUSED |\
                                             AX_APPLICATION_WINDOW_MOVED |\
                                             AX_APPLICATION_WINDOW_RESIZED |\
                                             AX_APPLICATION_WINDOW_TITLE_CHANGED)

static const char *ax_error_str[] =
{
    [-kAXErrorSuccess]                           = "kAXErrorSuccess",
    [-kAXErrorFailure]                           = "kAXErrorFailure",
    [-kAXErrorIllegalArgument]                   = "kAXErrorIllegalArgument",
    [-kAXErrorInvalidUIElement]                  = "kAXErrorInvalidUIElement",
    [-kAXErrorInvalidUIElementObserver]          = "kAXErrorInvalidUIElementObserver",
    [-kAXErrorCannotComplete]                    = "kAXErrorCannotComplete",
    [-kAXErrorAttributeUnsupported]              = "kAXErrorAttributeUnsupported",
    [-kAXErrorActionUnsupported]                 = "kAXErrorActionUnsupported",
    [-kAXErrorNotificationUnsupported]           = "kAXErrorNotificationUnsupported",
    [-kAXErrorNotImplemented]                    = "kAXErrorNotImplemented",
    [-kAXErrorNotificationAlreadyRegistered]     = "kAXErrorNotificationAlreadyRegistered",
    [-kAXErrorNotificationNotRegistered]         = "kAXErrorNotificationNotRegistered",
    [-kAXErrorAPIDisabled]                       = "kAXErrorAPIDisabled",
    [-kAXErrorNoValue]                           = "kAXErrorNoValue",
    [-kAXErrorParameterizedAttributeUnsupported] = "kAXErrorParameterizedAttributeUnsupported",
    [-kAXErrorNotEnoughPrecision]                = "kAXErrorNotEnoughPrecision"
};

static const char *ax_application_notification_str[] =
{
    [AX_APPLICATION_WINDOW_CREATED_INDEX]       = "kAXCreatedNotification",
    [AX_APPLICATION_WINDOW_FOCUSED_INDEX]       = "kAXFocusedWindowChangedNotification",
    [AX_APPLICATION_WINDOW_MOVED_INDEX]         = "kAXWindowMovedNotification",
    [AX_APPLICATION_WINDOW_RESIZED_INDEX]       = "kAXWindowResizedNotification",
    [AX_APPLICATION_WINDOW_TITLE_CHANGED_INDEX] = "kAXTitleChangedNotification",
    [AX_APPLICATION_WINDOW_MENU_OPENED_INDEX]   = "kAXMenuOpenedNotification",
    [AX_APPLICATION_WINDOW_MENU_CLOSED_INDEX]   = "kAXMenuClosedNotification"
};

static CFStringRef ax_application_notification[] =
{
    [AX_APPLICATION_WINDOW_CREATED_INDEX]       = kAXCreatedNotification,
    [AX_APPLICATION_WINDOW_FOCUSED_INDEX]       = kAXFocusedWindowChangedNotification,
    [AX_APPLICATION_WINDOW_MOVED_INDEX]         = kAXWindowMovedNotification,
    [AX_APPLICATION_WINDOW_RESIZED_INDEX]       = kAXWindowResizedNotification,
    [AX_APPLICATION_WINDOW_TITLE_CHANGED_INDEX] = kAXTitleChangedNotification,
    [AX_APPLICATION_WINDOW_MENU_OPENED_INDEX]   = kAXMenuOpenedNotification,
    [AX_APPLICATION_WINDOW_MENU_CLOSED_INDEX]   = kAXMenuClosedNotification
};

struct application
{
    AXUIElementRef ref;
    int connection;
    ProcessSerialNumber psn;
    uint32_t pid;
    char *name;
    AXObserverRef observer_ref;
    uint8_t notification;
    bool is_observing;
    bool is_hidden;
    bool ax_retry;
};

bool application_is_frontmost(struct application *application);
bool application_is_hidden(struct application *application);
uint32_t application_main_window(struct application *application);
uint32_t application_focused_window(struct application *application);
CFArrayRef application_window_list(struct application *application);
bool application_observe(struct application *application);
void application_unobserve(struct application *application);
struct application *application_create(struct process *process);
void application_destroy(struct application *application);

#endif

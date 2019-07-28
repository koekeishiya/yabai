#ifndef EVENT_LOOP_EVENT_H
#define EVENT_LOOP_EVENT_H

#define EVENT_CALLBACK(name) int name(void *context, int param1, void *param2)
typedef EVENT_CALLBACK(event_callback);

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_LAUNCHED);
static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_TERMINATED);
static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_FRONT_SWITCHED);
static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_ACTIVATED);
static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_DEACTIVATED);
static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_VISIBLE);
static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_HIDDEN);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_CREATED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_DESTROYED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_FOCUSED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MOVED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_RESIZED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MINIMIZED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_DEMINIMIZED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_TITLE_CHANGED);
static EVENT_CALLBACK(EVENT_HANDLER_SPACE_CHANGED);
static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_ADDED);
static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_REMOVED);
static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_MOVED);
static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_RESIZED);
static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_CHANGED);
static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_DOWN);
static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_UP);
static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_DRAGGED);
static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_MOVED);
static EVENT_CALLBACK(EVENT_HANDLER_MISSION_CONTROL_ENTER);
static EVENT_CALLBACK(EVENT_HANDLER_MISSION_CONTROL_CHECK_FOR_EXIT);
static EVENT_CALLBACK(EVENT_HANDLER_MISSION_CONTROL_EXIT);
static EVENT_CALLBACK(EVENT_HANDLER_DOCK_DID_RESTART);
static EVENT_CALLBACK(EVENT_HANDLER_MENU_OPENED);
static EVENT_CALLBACK(EVENT_HANDLER_MENU_BAR_HIDDEN_CHANGED);
static EVENT_CALLBACK(EVENT_HANDLER_DOCK_DID_CHANGE_PREF);
static EVENT_CALLBACK(EVENT_HANDLER_SYSTEM_WOKE);
static EVENT_CALLBACK(EVENT_HANDLER_BAR_REFRESH);
static EVENT_CALLBACK(EVENT_HANDLER_DAEMON_MESSAGE);

#define EVENT_IGNORED   -1
#define EVENT_QUEUED     0
#define EVENT_PROCESSED  1

#define EVENT_FAILURE     -1
#define EVENT_SUCCESS      0
#define EVENT_MOUSE_IGNORE 1

enum event_type
{
    EVENT_TYPE_UNKNOWN,

    APPLICATION_LAUNCHED,
    APPLICATION_TERMINATED,
    APPLICATION_FRONT_SWITCHED,
    APPLICATION_ACTIVATED,
    APPLICATION_DEACTIVATED,
    APPLICATION_VISIBLE,
    APPLICATION_HIDDEN,
    WINDOW_CREATED,
    WINDOW_DESTROYED,
    WINDOW_FOCUSED,
    WINDOW_MOVED,
    WINDOW_RESIZED,
    WINDOW_MINIMIZED,
    WINDOW_DEMINIMIZED,
    WINDOW_TITLE_CHANGED,
    SPACE_CHANGED,
    DISPLAY_ADDED,
    DISPLAY_REMOVED,
    DISPLAY_MOVED,
    DISPLAY_RESIZED,
    DISPLAY_CHANGED,
    MOUSE_DOWN,
    MOUSE_UP,
    MOUSE_DRAGGED,
    MOUSE_MOVED,
    MISSION_CONTROL_ENTER,
    MISSION_CONTROL_CHECK_FOR_EXIT,
    MISSION_CONTROL_EXIT,
    DOCK_DID_RESTART,
    MENU_OPENED,
    MENU_BAR_HIDDEN_CHANGED,
    DOCK_DID_CHANGE_PREF,
    SYSTEM_WOKE,
    BAR_REFRESH,
    DAEMON_MESSAGE,

    EVENT_TYPE_COUNT
};

static const char *event_type_str[] =
{
    [EVENT_TYPE_UNKNOWN]             = "event_type_unknown",

    [APPLICATION_LAUNCHED]           = "application_launched",
    [APPLICATION_TERMINATED]         = "application_terminated",
    [APPLICATION_FRONT_SWITCHED]     = "application_front_switched",
    [APPLICATION_ACTIVATED]          = "application_activated",
    [APPLICATION_DEACTIVATED]        = "application_deactivated",
    [APPLICATION_VISIBLE]            = "application_visible",
    [APPLICATION_HIDDEN]             = "application_hidden",
    [WINDOW_CREATED]                 = "window_created",
    [WINDOW_DESTROYED]               = "window_destroyed",
    [WINDOW_FOCUSED]                 = "window_focused",
    [WINDOW_MOVED]                   = "window_moved",
    [WINDOW_RESIZED]                 = "window_resized",
    [WINDOW_MINIMIZED]               = "window_minimized",
    [WINDOW_DEMINIMIZED]             = "window_deminimized",
    [WINDOW_TITLE_CHANGED]           = "window_title_changed",
    [SPACE_CHANGED]                  = "space_changed",
    [DISPLAY_ADDED]                  = "display_added",
    [DISPLAY_REMOVED]                = "display_removed",
    [DISPLAY_MOVED]                  = "display_moved",
    [DISPLAY_RESIZED]                = "display_resized",
    [DISPLAY_CHANGED]                = "display_changed",
    [MOUSE_DOWN]                     = "mouse_down",
    [MOUSE_UP]                       = "mouse_up",
    [MOUSE_DRAGGED]                  = "mouse_dragged",
    [MOUSE_MOVED]                    = "mouse_moved",
    [MISSION_CONTROL_ENTER]          = "mission_control_enter",
    [MISSION_CONTROL_CHECK_FOR_EXIT] = "mission_control_check_for_exit",
    [MISSION_CONTROL_EXIT]           = "mission_control_exit",
    [DOCK_DID_RESTART]               = "dock_did_restart",
    [MENU_OPENED]                    = "menu_opened",
    [MENU_BAR_HIDDEN_CHANGED]        = "menu_bar_hidden_changed",
    [DOCK_DID_CHANGE_PREF]           = "dock_did_change_pref",
    [SYSTEM_WOKE]                    = "system_woke",
    [BAR_REFRESH]                    = "bar_refresh",
    [DAEMON_MESSAGE]                 = "daemon_message",

    [EVENT_TYPE_COUNT]               = "event_type_count"
};

static event_callback *event_handler[] =
{
    [APPLICATION_LAUNCHED]           = EVENT_HANDLER_APPLICATION_LAUNCHED,
    [APPLICATION_TERMINATED]         = EVENT_HANDLER_APPLICATION_TERMINATED,
    [APPLICATION_FRONT_SWITCHED]     = EVENT_HANDLER_APPLICATION_FRONT_SWITCHED,
    [APPLICATION_ACTIVATED]          = EVENT_HANDLER_APPLICATION_ACTIVATED,
    [APPLICATION_DEACTIVATED]        = EVENT_HANDLER_APPLICATION_DEACTIVATED,
    [APPLICATION_VISIBLE]            = EVENT_HANDLER_APPLICATION_VISIBLE,
    [APPLICATION_HIDDEN]             = EVENT_HANDLER_APPLICATION_HIDDEN,
    [WINDOW_CREATED]                 = EVENT_HANDLER_WINDOW_CREATED,
    [WINDOW_DESTROYED]               = EVENT_HANDLER_WINDOW_DESTROYED,
    [WINDOW_FOCUSED]                 = EVENT_HANDLER_WINDOW_FOCUSED,
    [WINDOW_MOVED]                   = EVENT_HANDLER_WINDOW_MOVED,
    [WINDOW_RESIZED]                 = EVENT_HANDLER_WINDOW_RESIZED,
    [WINDOW_MINIMIZED]               = EVENT_HANDLER_WINDOW_MINIMIZED,
    [WINDOW_DEMINIMIZED]             = EVENT_HANDLER_WINDOW_DEMINIMIZED,
    [WINDOW_TITLE_CHANGED]           = EVENT_HANDLER_WINDOW_TITLE_CHANGED,
    [SPACE_CHANGED]                  = EVENT_HANDLER_SPACE_CHANGED,
    [DISPLAY_ADDED]                  = EVENT_HANDLER_DISPLAY_ADDED,
    [DISPLAY_REMOVED]                = EVENT_HANDLER_DISPLAY_REMOVED,
    [DISPLAY_MOVED]                  = EVENT_HANDLER_DISPLAY_MOVED,
    [DISPLAY_RESIZED]                = EVENT_HANDLER_DISPLAY_RESIZED,
    [DISPLAY_CHANGED]                = EVENT_HANDLER_DISPLAY_CHANGED,
    [MOUSE_DOWN]                     = EVENT_HANDLER_MOUSE_DOWN,
    [MOUSE_UP]                       = EVENT_HANDLER_MOUSE_UP,
    [MOUSE_DRAGGED]                  = EVENT_HANDLER_MOUSE_DRAGGED,
    [MOUSE_MOVED]                    = EVENT_HANDLER_MOUSE_MOVED,
    [MISSION_CONTROL_ENTER]          = EVENT_HANDLER_MISSION_CONTROL_ENTER,
    [MISSION_CONTROL_CHECK_FOR_EXIT] = EVENT_HANDLER_MISSION_CONTROL_CHECK_FOR_EXIT,
    [MISSION_CONTROL_EXIT]           = EVENT_HANDLER_MISSION_CONTROL_EXIT,
    [DOCK_DID_RESTART]               = EVENT_HANDLER_DOCK_DID_RESTART,
    [MENU_OPENED]                    = EVENT_HANDLER_MENU_OPENED,
    [MENU_BAR_HIDDEN_CHANGED]        = EVENT_HANDLER_MENU_BAR_HIDDEN_CHANGED,
    [DOCK_DID_CHANGE_PREF]           = EVENT_HANDLER_DOCK_DID_CHANGE_PREF,
    [SYSTEM_WOKE]                    = EVENT_HANDLER_SYSTEM_WOKE,
    [BAR_REFRESH]                    = EVENT_HANDLER_BAR_REFRESH,
    [DAEMON_MESSAGE]                 = EVENT_HANDLER_DAEMON_MESSAGE,
};

struct event
{
    enum event_type type;
    void *context;
    int param1;
    void *param2;
    volatile int *status;
    volatile int *result;
};

struct signal
{
    char *command;
    char *label;
};

#define event_create(e, t, d)\
    do {\
        e = malloc(sizeof(struct event));\
        e->type    = t;\
        e->context = d;\
        e->param1  = 0;\
        e->param2  = 0;\
        e->status  = 0;\
        e->result  = 0;\
    } while (0)

#define event_create_p2(e, t, d, p1, p2)\
    do {\
        e = malloc(sizeof(struct event));\
        e->type    = t;\
        e->context = d;\
        e->param1  = p1;\
        e->param2  = p2;\
        e->status  = 0;\
        e->result  = 0;\
    } while (0)

void event_signal_transmit(void *context, enum event_type type);
void event_signal_add(enum event_type type, char *action, char *label);
bool event_signal_remove(char *label);
void event_destroy(struct event *event);
enum event_type event_type_from_string(const char *str);

#endif

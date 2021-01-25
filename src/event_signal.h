#ifndef EVENT_SIGNAL_H
#define EVENT_SIGNAL_H

enum signal_type
{
    SIGNAL_TYPE_UNKNOWN,

    SIGNAL_APPLICATION_LAUNCHED,
    SIGNAL_APPLICATION_TERMINATED,
    SIGNAL_APPLICATION_FRONT_SWITCHED,
    SIGNAL_APPLICATION_ACTIVATED,
    SIGNAL_APPLICATION_DEACTIVATED,
    SIGNAL_APPLICATION_VISIBLE,
    SIGNAL_APPLICATION_HIDDEN,

    SIGNAL_WINDOW_CREATED,
    SIGNAL_WINDOW_DESTROYED,
    SIGNAL_WINDOW_FOCUSED,
    SIGNAL_WINDOW_MOVED,
    SIGNAL_WINDOW_RESIZED,
    SIGNAL_WINDOW_MINIMIZED,
    SIGNAL_WINDOW_DEMINIMIZED,
    SIGNAL_WINDOW_TITLE_CHANGED,

    SIGNAL_SPACE_CHANGED,

    SIGNAL_DISPLAY_ADDED,
    SIGNAL_DISPLAY_REMOVED,
    SIGNAL_DISPLAY_MOVED,
    SIGNAL_DISPLAY_RESIZED,
    SIGNAL_DISPLAY_CHANGED,

    SIGNAL_MISSION_CONTROL_ENTER,
    SIGNAL_MISSION_CONTROL_EXIT,

    SIGNAL_DOCK_DID_RESTART,
    SIGNAL_MENU_BAR_HIDDEN_CHANGED,
    SIGNAL_DOCK_DID_CHANGE_PREF,

    SIGNAL_TYPE_COUNT
};

static const char *signal_type_str[] =
{
    [SIGNAL_TYPE_UNKNOWN]                   = "signal_type_unknown",

    [SIGNAL_APPLICATION_LAUNCHED]           = "application_launched",
    [SIGNAL_APPLICATION_TERMINATED]         = "application_terminated",
    [SIGNAL_APPLICATION_FRONT_SWITCHED]     = "application_front_switched",
    [SIGNAL_APPLICATION_ACTIVATED]          = "application_activated",
    [SIGNAL_APPLICATION_DEACTIVATED]        = "application_deactivated",
    [SIGNAL_APPLICATION_VISIBLE]            = "application_visible",
    [SIGNAL_APPLICATION_HIDDEN]             = "application_hidden",

    [SIGNAL_WINDOW_CREATED]                 = "window_created",
    [SIGNAL_WINDOW_DESTROYED]               = "window_destroyed",
    [SIGNAL_WINDOW_FOCUSED]                 = "window_focused",
    [SIGNAL_WINDOW_MOVED]                   = "window_moved",
    [SIGNAL_WINDOW_RESIZED]                 = "window_resized",
    [SIGNAL_WINDOW_MINIMIZED]               = "window_minimized",
    [SIGNAL_WINDOW_DEMINIMIZED]             = "window_deminimized",
    [SIGNAL_WINDOW_TITLE_CHANGED]           = "window_title_changed",

    [SIGNAL_SPACE_CHANGED]                  = "space_changed",

    [SIGNAL_DISPLAY_ADDED]                  = "display_added",
    [SIGNAL_DISPLAY_REMOVED]                = "display_removed",
    [SIGNAL_DISPLAY_MOVED]                  = "display_moved",
    [SIGNAL_DISPLAY_RESIZED]                = "display_resized",
    [SIGNAL_DISPLAY_CHANGED]                = "display_changed",

    [SIGNAL_MISSION_CONTROL_ENTER]          = "mission_control_enter",
    [SIGNAL_MISSION_CONTROL_EXIT]           = "mission_control_exit",

    [SIGNAL_DOCK_DID_RESTART]               = "dock_did_restart",
    [SIGNAL_MENU_BAR_HIDDEN_CHANGED]        = "menu_bar_hidden_changed",
    [SIGNAL_DOCK_DID_CHANGE_PREF]           = "dock_did_change_pref",

    [SIGNAL_TYPE_COUNT]                     = "signal_type_count"
};

#define SIGNAL_PROP_UD  0
#define SIGNAL_PROP_YES 1
#define SIGNAL_PROP_NO  2

struct event_signal
{
    enum signal_type type;
    char *arg_name[2];
    char *arg_value[2];
    char *app;
    char *title;
    int active;
};

struct signal
{
    char *app;
    char *title;
    bool app_regex_valid;
    bool title_regex_valid;
    bool app_regex_exclude;
    bool title_regex_exclude;
    regex_t app_regex;
    regex_t title_regex;
    int active;
    char *command;
    char *label;
};

void event_signal_push(enum signal_type type, void *context);
void event_signal_flush(void);

void event_signal_add(enum signal_type type, struct signal *signal);
void event_signal_destroy(struct signal *signal);
bool event_signal_remove_by_index(int index);
bool event_signal_remove(char *label);
void event_signal_list(FILE *rsp);
enum signal_type signal_type_from_string(const char *str);

#endif

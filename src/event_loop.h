#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#define EVENT_CALLBACK(name) uint32_t name(void *context, int param1)
typedef EVENT_CALLBACK(event_callback);

#define EVENT_QUEUED    0x0
#define EVENT_PROCESSED 0x1

#define EVENT_SUCCESS 0x0
#define EVENT_FAILURE 0x1

#define event_status(e) ((e)  & 0x1)
#define event_result(e) ((e) >> 0x1)

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
    SLS_WINDOW_MOVED,
    SLS_WINDOW_RESIZED,
    SLS_WINDOW_ORDER_CHANGED,
    SLS_WINDOW_IS_VISIBLE,
    SLS_WINDOW_IS_INVISIBLE,
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
    MISSION_CONTROL_SHOW_ALL_WINDOWS,
    MISSION_CONTROL_SHOW_FRONT_WINDOWS,
    MISSION_CONTROL_SHOW_DESKTOP,
    MISSION_CONTROL_ENTER,
    MISSION_CONTROL_CHECK_FOR_EXIT,
    MISSION_CONTROL_EXIT,
    DOCK_DID_RESTART,
    MENU_OPENED,
    MENU_CLOSED,
    MENU_BAR_HIDDEN_CHANGED,
    DOCK_DID_CHANGE_PREF,
    SYSTEM_WOKE,
    DAEMON_MESSAGE,

    EVENT_TYPE_COUNT
};

struct event
{
    void *context;
    volatile uint32_t *info;
    enum event_type type;
    int param1;
};

struct event_loop_item
{
    struct event_loop_item *next;
    struct event event;
};

struct event_loop
{
    bool is_running;
    pthread_t thread;
    sem_t *semaphore;
    struct memory_pool pool;
    struct event_loop_item *head;
    struct event_loop_item *tail;
};

bool event_loop_init(struct event_loop *event_loop);
bool event_loop_begin(struct event_loop *event_loop);
bool event_loop_end(struct event_loop *event_loop);
void event_loop_post(struct event_loop *event_loop, enum event_type type, void *context, int param1, volatile uint32_t *info);

#endif

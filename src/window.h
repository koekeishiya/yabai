#ifndef WINDOW_H
#define WINDOW_H

const CFStringRef kAXFullscreenAttribute = CFSTR("AXFullScreen");

#define AX_WINDOW_MINIMIZED_INDEX      0
#define AX_WINDOW_DEMINIMIZED_INDEX    1
#define AX_WINDOW_DESTROYED_INDEX      2

#define AX_WINDOW_DESTROYED      (1 << AX_WINDOW_DESTROYED_INDEX)
#define AX_WINDOW_MINIMIZED      (1 << AX_WINDOW_MINIMIZED_INDEX)
#define AX_WINDOW_DEMINIMIZED    (1 << AX_WINDOW_DEMINIMIZED_INDEX)
#define AX_WINDOW_ALL            (AX_WINDOW_DESTROYED |\
                                  AX_WINDOW_MINIMIZED |\
                                  AX_WINDOW_DEMINIMIZED)

static const char *ax_window_notification_str[] =
{
    [AX_WINDOW_DESTROYED_INDEX]      = "kAXUIElementDestroyedNotification",
    [AX_WINDOW_MINIMIZED_INDEX]      = "kAXWindowMiniaturizedNotification",
    [AX_WINDOW_DEMINIMIZED_INDEX]    = "kAXWindowDeminiaturizedNotification"
};

static CFStringRef ax_window_notification[] =
{
    [AX_WINDOW_DESTROYED_INDEX]      = kAXUIElementDestroyedNotification,
    [AX_WINDOW_MINIMIZED_INDEX]      = kAXWindowMiniaturizedNotification,
    [AX_WINDOW_DEMINIMIZED_INDEX]    = kAXWindowDeminiaturizedNotification
};

struct window
{
    struct application *application;
    AXUIElementRef ref;
    uint32_t id;
    uint32_t *volatile id_ptr;
    CGRect frame;
    uint8_t notification;
    uint8_t rule_flags;
    uint8_t flags;
    float opacity;
    struct border border;
};

enum window_flag
{
    WINDOW_SHADOW     = 1 << 0,
    WINDOW_FULLSCREEN = 1 << 1,
    WINDOW_MINIMIZE   = 1 << 2,
    WINDOW_FLOAT      = 1 << 3,
    WINDOW_STICKY     = 1 << 4,
};

#define window_check_flag(w, x) ((w)->flags  &  (x))
#define window_clear_flag(w, x) ((w)->flags &= ~(x))
#define window_set_flag(w, x)   ((w)->flags |=  (x))

enum window_rule_flag
{
    WINDOW_RULE_MANAGED    = 1 << 0,
    WINDOW_RULE_FULLSCREEN = 1 << 1,
    WINDOW_RULE_MFF        = 1 << 2,
    WINDOW_RULE_MFF_VALUE  = 1 << 3,
    WINDOW_RULE_BORDER     = 1 << 4,
};

#define window_rule_check_flag(w, x) ((w)->rule_flags  &  (x))
#define window_rule_clear_flag(w, x) ((w)->rule_flags &= ~(x))
#define window_rule_set_flag(w, x)   ((w)->rule_flags |=  (x))

CFStringRef window_display_uuid(struct window *window);
uint32_t window_display_id(struct window *window);
uint64_t window_space(struct window *window);
uint64_t *window_space_list(struct window *window, int *count);
void window_serialize(FILE *rsp, struct window *window);
char *window_title_ts(struct window *window);
CGPoint window_ax_origin(struct window *window);
CGRect window_ax_frame(struct window *window);
float window_opacity(struct window *window);
int window_level(uint32_t wid);
uint64_t window_tags(struct window *window);
CFStringRef window_role(struct window *window);
char *window_role_ts(struct window *window);
CFStringRef window_subrole(struct window *window);
char *window_subrole_ts(struct window *window);
bool window_can_move(struct window *window);
bool window_can_resize(struct window *window);
bool window_can_minimize(struct window *window);
bool window_level_is_standard(struct window *window);
bool window_is_undersized(struct window *window);
bool window_is_minimized(struct window *window);
bool window_is_fullscreen(struct window *window);
bool window_is_sticky(struct window *window);
bool window_is_standard(struct window *window);
bool window_is_dialog(struct window *window);
bool window_is_popover(struct window *window);
bool window_is_unknown(struct window *window);
bool window_observe(struct window *window);
void window_unobserve(struct window *window);
struct window *window_create(struct application *application, AXUIElementRef window_ref, uint32_t window_id);
void window_destroy(struct window *window);

#endif

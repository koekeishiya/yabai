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
    int connection;
    uint32_t id;
    uint32_t *volatile id_ptr;
    CGRect frame;
    uint8_t notification;
    bool has_shadow;
    bool is_fullscreen;
    bool is_minimized;
    bool is_floating;
    bool is_sticky;
    float opacity;
    uint32_t blur_radius;
    bool rule_manage;
    bool rule_fullscreen;
    bool rule_mff;
    bool enable_mff;
    struct border border;
};

CFStringRef window_display_uuid(struct window *window);
int window_display_id(struct window *window);
uint64_t window_space(struct window *window);
uint64_t *window_space_list(struct window *window, int *count);
void window_serialize(FILE *rsp, struct window *window);
char *window_title(struct window *window);
CGPoint window_ax_origin(struct window *window);
CGRect window_ax_frame(struct window *window);
float window_opacity(struct window *window);
int window_level(struct window *window);
uint64_t window_tags(struct window *window);
CFStringRef window_role(struct window *window);
CFStringRef window_subrole(struct window *window);
bool window_can_move(struct window *window);
bool window_can_resize(struct window *window);
bool window_can_minimize(struct window *window);
bool window_level_is_standard(struct window *window);
bool window_is_undersized(struct window *window);
bool window_is_minimized(struct window *window);
bool window_is_fullscreen(struct window *window);
bool window_is_sticky(struct window *window);
bool window_is_topmost(struct window *window);
bool window_is_standard(struct window *window);
bool window_is_dialog(struct window *window);
bool window_is_popover(struct window *window);
bool window_is_unknown(struct window *window);
bool window_observe(struct window *window);
void window_unobserve(struct window *window);
struct window *window_create(struct application *application, AXUIElementRef window_ref, uint32_t window_id);
void window_destroy(struct window *window);

#endif

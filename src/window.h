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

enum cg_window_level_key
{
    CG_WINDOW_BASE_LEVEL_KEY                =  0,
    CG_WINDOW_MINIMUM_LEVEL_KEY             =  1,
    CG_WINDOW_DESKTOP_LEVEL_KEY             =  2,
    CG_WINDOW_BACKSTOP_MENU_LEVEL_KEY       =  3,
    CG_WINDOW_NORMAL_LEVEL_KEY              =  4,
    CG_WINDOW_FLOATING_LEVEL_KEY            =  5,
    CG_WINDOW_TORN_OFF_MENU_LEVEL_KEY       =  6,
    CG_WINDOW_DOCK_LEVEL_KEY                =  7,
    CG_WINDOW_MAIN_MENU_LEVEL_KEY           =  8,
    CG_WINDOW_STATUS_LEVEL_KEY              =  9,
    CG_WINDOW_MODAL_PANEL_LEVEL_KEY         = 10,
    CG_WINDOW_POPUP_MENU_LEVEL_KEY          = 11,
    CG_WINDOW_DRAGGING_LEVEL_KEY            = 12,
    CG_WINDOW_SCREENSAVER_LEVEL_KEY         = 13,
    CG_WINDOW_MAXIMUM_LEVEL_KEY             = 14,
    CG_WINDOW_OVERLAY_LEVEL_KEY             = 15,
    CG_WINDOW_HELP_LEVEL_KEY                = 16,
    CG_WINDOW_UTILITY_LEVEL_KEY             = 17,
    CG_WINDOW_DESKTOP_ICON_LEVEL_KEY        = 18,
    CG_WINDOW_CURSOR_LEVEL_KEY              = 19,
    CG_WINDOW_ASSISTIVE_TECH_HIGH_LEVEL_KEY = 20,
    CG_WINDOW_LEVEL_KEY_COUNT               = 21
};

static const char *cg_window_level_key_str[] =
{
    [CG_WINDOW_BASE_LEVEL_KEY]                = "CG_WINDOW_BASE_LEVEL_KEY",
    [CG_WINDOW_MINIMUM_LEVEL_KEY]             = "CG_WINDOW_MINIMUM_LEVEL_KEY",
    [CG_WINDOW_DESKTOP_LEVEL_KEY]             = "CG_WINDOW_DESKTOP_LEVEL_KEY",
    [CG_WINDOW_BACKSTOP_MENU_LEVEL_KEY]       = "CG_WINDOW_BACKSTOP_MENU_LEVEL_KEY",
    [CG_WINDOW_NORMAL_LEVEL_KEY]              = "CG_WINDOW_NORMAL_LEVEL_KEY",
    [CG_WINDOW_FLOATING_LEVEL_KEY]            = "CG_WINDOW_FLOATING_LEVEL_KEY",
    [CG_WINDOW_TORN_OFF_MENU_LEVEL_KEY]       = "CG_WINDOW_TORN_OFF_MENU_LEVEL_KEY",
    [CG_WINDOW_DOCK_LEVEL_KEY]                = "CG_WINDOW_DOCK_LEVEL_KEY",
    [CG_WINDOW_MAIN_MENU_LEVEL_KEY]           = "CG_WINDOW_MAIN_MENU_LEVEL_KEY",
    [CG_WINDOW_STATUS_LEVEL_KEY]              = "CG_WINDOW_STATUS_LEVEL_KEY",
    [CG_WINDOW_MODAL_PANEL_LEVEL_KEY]         = "CG_WINDOW_MODAL_PANEL_LEVEL_KEY",
    [CG_WINDOW_POPUP_MENU_LEVEL_KEY]          = "CG_WINDOW_POPUP_MENU_LEVEL_KEY",
    [CG_WINDOW_DRAGGING_LEVEL_KEY]            = "CG_WINDOW_DRAGGING_LEVEL_KEY",
    [CG_WINDOW_SCREENSAVER_LEVEL_KEY]         = "CG_WINDOW_SCREENSAVER_LEVEL_KEY",
    [CG_WINDOW_MAXIMUM_LEVEL_KEY]             = "CG_WINDOW_MAXIMUM_LEVEL_KEY",
    [CG_WINDOW_OVERLAY_LEVEL_KEY]             = "CG_WINDOW_OVERLAY_LEVEL_KEY",
    [CG_WINDOW_HELP_LEVEL_KEY]                = "CG_WINDOW_HELP_LEVEL_KEY",
    [CG_WINDOW_UTILITY_LEVEL_KEY]             = "CG_WINDOW_UTILITY_LEVEL_KEY",
    [CG_WINDOW_DESKTOP_ICON_LEVEL_KEY]        = "CG_WINDOW_DESKTOP_ICON_LEVEL_KEY",
    [CG_WINDOW_CURSOR_LEVEL_KEY]              = "CG_WINDOW_CURSOR_LEVEL_KEY",
    [CG_WINDOW_ASSISTIVE_TECH_HIGH_LEVEL_KEY] = "CG_WINDOW_ASSISTIVE_TECH_HIGH_LEVEL_KEY",
    [CG_WINDOW_LEVEL_KEY_COUNT]               = "CG_WINDOW_LEVEL_KEY_COUNT"
};

struct window
{
    struct application *application;
    AXUIElementRef ref;
    uint32_t id;
    uint32_t *volatile id_ptr;
    CGRect frame;
    bool is_root;
    bool is_eligible;
    uint8_t notification;
    uint8_t rule_flags;
    uint8_t flags;
    float opacity;
    int layer;
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
int window_sub_level(uint32_t wid);
uint64_t window_tags(struct window *window);
CFStringRef window_role(struct window *window);
char *window_role_ts(struct window *window);
CFStringRef window_subrole(struct window *window);
char *window_subrole_ts(struct window *window);
bool window_can_move(struct window *window);
bool window_can_resize(struct window *window);
bool window_can_minimize(struct window *window);
bool window_is_undersized(struct window *window);
bool window_is_minimized(struct window *window);
bool window_is_fullscreen(struct window *window);
bool window_is_sticky(struct window *window);
bool window_is_real(struct window *window);
bool window_is_standard(struct window *window);
bool window_level_is_standard(struct window *window);
bool window_is_unknown(struct window *window);
bool window_observe(struct window *window);
void window_unobserve(struct window *window);
struct window *window_create(struct application *application, AXUIElementRef window_ref, uint32_t window_id);
void window_destroy(struct window *window);
void window_dump_cg_window_levels(void);

#endif

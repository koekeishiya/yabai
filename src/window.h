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

enum window_property
{
    WINDOW_PROPERTY_ID                  = 0x00000001,
    WINDOW_PROPERTY_PID                 = 0x00000002,
    WINDOW_PROPERTY_APP                 = 0x00000004,
    WINDOW_PROPERTY_TITLE               = 0x00000008,
    WINDOW_PROPERTY_FRAME               = 0x00000010,
    WINDOW_PROPERTY_ROLE                = 0x00000020,
    WINDOW_PROPERTY_SUBROLE             = 0x00000040,
    WINDOW_PROPERTY_ROOT_WINDOW         = 0x00000080,
    WINDOW_PROPERTY_DISPLAY             = 0x00000100,
    WINDOW_PROPERTY_SPACE               = 0x00000200,
    WINDOW_PROPERTY_LEVEL               = 0x00000400,
    WINDOW_PROPERTY_SUB_LEVEL           = 0x00000800,
    WINDOW_PROPERTY_LAYER               = 0x00001000,
    WINDOW_PROPERTY_SUB_LAYER           = 0x00002000,
    WINDOW_PROPERTY_OPACITY             = 0x00004000,
    WINDOW_PROPERTY_SPLIT_TYPE          = 0x00008000,
    WINDOW_PROPERTY_SPLIT_CHILD         = 0x00010000,
    WINDOW_PROPERTY_STACK_INDEX         = 0x00020000,
    WINDOW_PROPERTY_CAN_MOVE            = 0x00040000,
    WINDOW_PROPERTY_CAN_RESIZE          = 0x00080000,
    WINDOW_PROPERTY_HAS_FOCUS           = 0x00100000,
    WINDOW_PROPERTY_HAS_SHADOW          = 0x00200000,
    WINDOW_PROPERTY_HAS_PARENT_ZOOM     = 0x00400000,
    WINDOW_PROPERTY_HAS_FULLSCREEN_ZOOM = 0x00800000,
    WINDOW_PROPERTY_HAS_AX_REFERENCE    = 0x01000000,
    WINDOW_PROPERTY_IS_FULLSCREEN       = 0x02000000,
    WINDOW_PROPERTY_IS_VISIBLE          = 0x04000000,
    WINDOW_PROPERTY_IS_MINIMIZED        = 0x08000000,
    WINDOW_PROPERTY_IS_HIDDEN           = 0x10000000,
    WINDOW_PROPERTY_IS_FLOATING         = 0x20000000,
    WINDOW_PROPERTY_IS_STICKY           = 0x40000000,
    WINDOW_PROPERTY_IS_GRABBED          = 0x80000000
};

static int window_property_val[] =
{
    [0x00] = WINDOW_PROPERTY_ID,
    [0x01] = WINDOW_PROPERTY_PID,
    [0x02] = WINDOW_PROPERTY_APP,
    [0x03] = WINDOW_PROPERTY_TITLE,
    [0x04] = WINDOW_PROPERTY_FRAME,
    [0x05] = WINDOW_PROPERTY_ROLE,
    [0x06] = WINDOW_PROPERTY_SUBROLE,
    [0x07] = WINDOW_PROPERTY_ROOT_WINDOW,
    [0x08] = WINDOW_PROPERTY_DISPLAY,
    [0x09] = WINDOW_PROPERTY_SPACE,
    [0x0A] = WINDOW_PROPERTY_LEVEL,
    [0x0B] = WINDOW_PROPERTY_SUB_LEVEL,
    [0x0C] = WINDOW_PROPERTY_LAYER,
    [0x0D] = WINDOW_PROPERTY_SUB_LAYER,
    [0x0E] = WINDOW_PROPERTY_OPACITY,
    [0x0F] = WINDOW_PROPERTY_SPLIT_TYPE,
    [0x10] = WINDOW_PROPERTY_SPLIT_CHILD,
    [0x11] = WINDOW_PROPERTY_STACK_INDEX,
    [0x12] = WINDOW_PROPERTY_CAN_MOVE,
    [0x13] = WINDOW_PROPERTY_CAN_RESIZE,
    [0x14] = WINDOW_PROPERTY_HAS_FOCUS,
    [0x15] = WINDOW_PROPERTY_HAS_SHADOW,
    [0x16] = WINDOW_PROPERTY_HAS_PARENT_ZOOM,
    [0x17] = WINDOW_PROPERTY_HAS_FULLSCREEN_ZOOM,
    [0x18] = WINDOW_PROPERTY_HAS_AX_REFERENCE,
    [0x19] = WINDOW_PROPERTY_IS_FULLSCREEN,
    [0x1A] = WINDOW_PROPERTY_IS_VISIBLE,
    [0x1B] = WINDOW_PROPERTY_IS_MINIMIZED,
    [0x1C] = WINDOW_PROPERTY_IS_HIDDEN,
    [0x1D] = WINDOW_PROPERTY_IS_FLOATING,
    [0x1E] = WINDOW_PROPERTY_IS_STICKY,
    [0x1F] = WINDOW_PROPERTY_IS_GRABBED
};

static char *window_property_str[] =
{
    "id",
    "pid",
    "app",
    "title",
    "frame",
    "role",
    "subrole",
    "root-window",
    "display",
    "space",
    "level",
    "sub-level",
    "layer",
    "sub-layer",
    "opacity",
    "split-type",
    "split-child",
    "stack-index",
    "can-move",
    "can-resize",
    "has-focus",
    "has-shadow",
    "has-parent_zoom",
    "has-fullscreen-zoom",
    "has-ax-reference",
    "is-native-fullscreen",
    "is-visible",
    "is-minimized",
    "is-hidden",
    "is-floating",
    "is-sticky",
    "is-grabbed"
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

CFStringRef window_display_uuid(uint32_t wid);
uint32_t window_display_id(uint32_t wid);
uint64_t window_space(uint32_t wid);
uint64_t *window_space_list(uint32_t wid, int *count);
void window_unknown_serialize(FILE *rsp, uint32_t wid, uint64_t flags);
void window_serialize(FILE *rsp, struct window *window, uint64_t flags);
char *window_property_title_ts(uint32_t wid);
char *window_title_ts(struct window *window);
bool window_shadow(uint32_t wid);
float window_opacity(uint32_t wid);
uint32_t window_parent(uint32_t wid);
int window_level(uint32_t wid);
int window_sub_level(uint32_t wid);
uint64_t window_tags(uint32_t wid);
bool window_is_sticky(uint32_t wid);
CGPoint window_ax_origin(struct window *window);
CGRect window_ax_frame(struct window *window);
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

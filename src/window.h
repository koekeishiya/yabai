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

#define WINDOW_PROPERTY_LIST \
    WINDOW_PROPERTY_ENTRY("id",                   WINDOW_PROPERTY_ID,                  0x000000001) \
    WINDOW_PROPERTY_ENTRY("pid",                  WINDOW_PROPERTY_PID,                 0x000000002) \
    WINDOW_PROPERTY_ENTRY("app",                  WINDOW_PROPERTY_APP,                 0x000000004) \
    WINDOW_PROPERTY_ENTRY("title",                WINDOW_PROPERTY_TITLE,               0x000000008) \
    WINDOW_PROPERTY_ENTRY("scratchpad",           WINDOW_PROPERTY_SCRATCHPAD,          0x000000010) \
    WINDOW_PROPERTY_ENTRY("frame",                WINDOW_PROPERTY_FRAME,               0x000000020) \
    WINDOW_PROPERTY_ENTRY("role",                 WINDOW_PROPERTY_ROLE,                0x000000040) \
    WINDOW_PROPERTY_ENTRY("subrole",              WINDOW_PROPERTY_SUBROLE,             0x000000080) \
    WINDOW_PROPERTY_ENTRY("root-window",          WINDOW_PROPERTY_ROOT_WINDOW,         0x000000100) \
    WINDOW_PROPERTY_ENTRY("display",              WINDOW_PROPERTY_DISPLAY,             0x000000200) \
    WINDOW_PROPERTY_ENTRY("space",                WINDOW_PROPERTY_SPACE,               0x000000400) \
    WINDOW_PROPERTY_ENTRY("level",                WINDOW_PROPERTY_LEVEL,               0x000000800) \
    WINDOW_PROPERTY_ENTRY("sub-level",            WINDOW_PROPERTY_SUB_LEVEL,           0x000001000) \
    WINDOW_PROPERTY_ENTRY("layer",                WINDOW_PROPERTY_LAYER,               0x000002000) \
    WINDOW_PROPERTY_ENTRY("sub-layer",            WINDOW_PROPERTY_SUB_LAYER,           0x000004000) \
    WINDOW_PROPERTY_ENTRY("opacity",              WINDOW_PROPERTY_OPACITY,             0x000008000) \
    WINDOW_PROPERTY_ENTRY("split-type",           WINDOW_PROPERTY_SPLIT_TYPE,          0x000010000) \
    WINDOW_PROPERTY_ENTRY("split-child",          WINDOW_PROPERTY_SPLIT_CHILD,         0x000020000) \
    WINDOW_PROPERTY_ENTRY("stack-index",          WINDOW_PROPERTY_STACK_INDEX,         0x000040000) \
    WINDOW_PROPERTY_ENTRY("can-move",             WINDOW_PROPERTY_CAN_MOVE,            0x000080000) \
    WINDOW_PROPERTY_ENTRY("can-resize",           WINDOW_PROPERTY_CAN_RESIZE,          0x000100000) \
    WINDOW_PROPERTY_ENTRY("has-focus",            WINDOW_PROPERTY_HAS_FOCUS,           0x000200000) \
    WINDOW_PROPERTY_ENTRY("has-shadow",           WINDOW_PROPERTY_HAS_SHADOW,          0x000400000) \
    WINDOW_PROPERTY_ENTRY("has-parent-zoom",      WINDOW_PROPERTY_HAS_PARENT_ZOOM,     0x000800000) \
    WINDOW_PROPERTY_ENTRY("has-fullscreen-zoom",  WINDOW_PROPERTY_HAS_FULLSCREEN_ZOOM, 0x001000000) \
    WINDOW_PROPERTY_ENTRY("has-ax-reference",     WINDOW_PROPERTY_HAS_AX_REFERENCE,    0x002000000) \
    WINDOW_PROPERTY_ENTRY("is-native-fullscreen", WINDOW_PROPERTY_IS_FULLSCREEN,       0x004000000) \
    WINDOW_PROPERTY_ENTRY("is-visible",           WINDOW_PROPERTY_IS_VISIBLE,          0x008000000) \
    WINDOW_PROPERTY_ENTRY("is-minimized",         WINDOW_PROPERTY_IS_MINIMIZED,        0x010000000) \
    WINDOW_PROPERTY_ENTRY("is-hidden",            WINDOW_PROPERTY_IS_HIDDEN,           0x020000000) \
    WINDOW_PROPERTY_ENTRY("is-floating",          WINDOW_PROPERTY_IS_FLOATING,         0x040000000) \
    WINDOW_PROPERTY_ENTRY("is-sticky",            WINDOW_PROPERTY_IS_STICKY,           0x080000000) \
    WINDOW_PROPERTY_ENTRY("is-grabbed",           WINDOW_PROPERTY_IS_GRABBED,          0x100000000)

enum window_property
{
#define WINDOW_PROPERTY_ENTRY(n, p, v) p = v,
    WINDOW_PROPERTY_LIST
#undef WINDOW_PROPERTY_ENTRY
};

static uint64_t window_property_val[] =
{
#define WINDOW_PROPERTY_ENTRY(n, p, v) p,
    WINDOW_PROPERTY_LIST
#undef WINDOW_PROPERTY_ENTRY
};

static char *window_property_str[] =
{
#define WINDOW_PROPERTY_ENTRY(n, p, v) n,
    WINDOW_PROPERTY_LIST
#undef WINDOW_PROPERTY_ENTRY
};

struct window
{
    struct application *application;
    AXUIElementRef ref;
    uint32_t id;
    uint32_t *volatile id_ptr;
    CFStringRef role;
    CFStringRef subrole;
    CFStringRef title;
    CGRect frame;
    CGRect windowed_frame;
    bool is_root;
    bool is_eligible;
    uint8_t notification;
    uint8_t rule_flags;
    uint8_t flags;
    float opacity;
    int layer;
    char *scratchpad;
};

enum window_flag
{
    WINDOW_SHADOW     = 0x01,
    WINDOW_FULLSCREEN = 0x02,
    WINDOW_MINIMIZE   = 0x04,
    WINDOW_FLOAT      = 0x08,
    WINDOW_STICKY     = 0x10,
    WINDOW_WINDOWED   = 0x20,
    WINDOW_MOVABLE    = 0x40,
    WINDOW_RESIZABLE  = 0x80
};

enum window_rule_flag
{
    WINDOW_RULE_MANAGED    = 0x01,
    WINDOW_RULE_FULLSCREEN = 0x02,
    WINDOW_RULE_MFF        = 0x04,
    WINDOW_RULE_MFF_VALUE  = 0x08
};

static inline bool window_check_flag(struct window *w, enum window_flag x) { return w->flags & x; }
static inline void window_clear_flag(struct window *w, enum window_flag x) { w->flags &= ~x; }
static inline void window_set_flag(struct window *w, enum window_flag x) { w->flags |= x; }

static inline bool window_check_rule_flag(struct window *w, enum window_rule_flag x) { return w->rule_flags & x; }
static inline void window_clear_rule_flag(struct window *w, enum window_rule_flag x) { w->rule_flags &= ~x; }
static inline void window_set_rule_flag(struct window *w, enum window_rule_flag x) { w->rule_flags |= x; }

CFStringRef window_display_uuid(uint32_t wid);
uint32_t window_display_id(uint32_t wid);
uint64_t window_space(uint32_t wid);
uint64_t *window_space_list(uint32_t wid, int *count);
void window_unknown_serialize(FILE *rsp, uint32_t wid, uint64_t flags);
void window_serialize(FILE *rsp, struct window *window, uint64_t flags);
char *window_property_title_ts(uint32_t wid);
char *window_title_ts(struct window *window);
CFStringRef window_title(struct window *window);
bool window_shadow(uint32_t wid);
float window_opacity(uint32_t wid);
uint32_t window_parent(uint32_t wid);
int window_level(uint32_t wid);
int window_sub_level(uint32_t wid);
uint64_t window_tags(uint32_t wid);
bool window_is_sticky(uint32_t wid);
CGPoint window_ax_origin(struct window *window);
CGRect window_ax_frame(struct window *window);
CFStringRef window_ax_role(struct window *window);
CFStringRef window_role(struct window *window);
char *window_role_ts(struct window *window);
CFStringRef window_ax_subrole(struct window *window);
CFStringRef window_subrole(struct window *window);
char *window_subrole_ts(struct window *window);
bool window_ax_can_move(struct window *window);
bool window_can_move(struct window *window);
bool window_ax_can_resize(struct window *window);
bool window_can_resize(struct window *window);
bool window_can_minimize(struct window *window);
bool window_is_undersized(struct window *window);
bool window_is_fullscreen(struct window *window);
bool window_is_real(struct window *window);
bool window_is_standard(struct window *window);
bool window_level_is_standard(struct window *window);
bool window_is_unknown(struct window *window);
bool window_observe(struct window *window);
void window_unobserve(struct window *window);
struct window *window_create(struct application *application, AXUIElementRef window_ref, uint32_t window_id);
void window_destroy(struct window *window);

#endif

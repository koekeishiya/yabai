#ifndef MOUSE_H
#define MOUSE_H

#define MOUSE_EVENT_MASK_FFM (1 << kCGEventMouseMoved) | \
                             (1 << kCGEventLeftMouseDown) | \
                             (1 << kCGEventLeftMouseUp) | \
                             (1 << kCGEventLeftMouseDragged) | \
                             (1 << kCGEventRightMouseDown) | \
                             (1 << kCGEventRightMouseUp) | \
                             (1 << kCGEventRightMouseDragged)

#define MOUSE_EVENT_MASK     (1 << kCGEventLeftMouseDown) | \
                             (1 << kCGEventLeftMouseUp) | \
                             (1 << kCGEventLeftMouseDragged) | \
                             (1 << kCGEventRightMouseDown) | \
                             (1 << kCGEventRightMouseUp) | \
                             (1 << kCGEventRightMouseDragged)

#define MOUSE_HANDLER(name) CGEventRef name(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *context)

enum mouse_drop_action
{
    MOUSE_DROP_ACTION_NONE,
    MOUSE_DROP_ACTION_STACK,
    MOUSE_DROP_ACTION_SWAP,
    MOUSE_DROP_ACTION_WARP_TOP,
    MOUSE_DROP_ACTION_WARP_RIGHT,
    MOUSE_DROP_ACTION_WARP_BOTTOM,
    MOUSE_DROP_ACTION_WARP_LEFT
};

enum mouse_mod
{
    MOUSE_MOD_NONE  = 0x01,
    MOUSE_MOD_ALT   = 0x02,
    MOUSE_MOD_SHIFT = 0x04,
    MOUSE_MOD_CMD   = 0x08,
    MOUSE_MOD_CTRL  = 0x10,
    MOUSE_MOD_FN    = 0x20
};

enum mouse_mode
{
    MOUSE_MODE_NONE,
    MOUSE_MODE_MOVE,
    MOUSE_MODE_RESIZE,
    MOUSE_MODE_SWAP,
    MOUSE_MODE_STACK
};

struct mouse_window_info
{
    float dx, dy, dw, dh;
    bool changed_x, changed_y;
    bool changed_w, changed_h;
    bool changed_position;
    bool changed_size;
};

struct mouse_state
{
    CFMachPortRef handle;
    CFRunLoopSourceRef runloop_source;
    bool consume_mouse_click;
    enum mouse_mode action1;
    enum mouse_mode action2;
    volatile uint8_t modifier;
    enum mouse_mode drop_action;
    enum mouse_mode current_action;
    CGPoint down_location;
    uint64_t last_moved_time;
    struct window *window;
    CGRect window_frame;
    uint32_t ffm_window_id;
};

static char *mouse_mod_str[] =
{
    [MOUSE_MOD_NONE]  = "none",
    [MOUSE_MOD_ALT]   = "alt",
    [MOUSE_MOD_SHIFT] = "shift",
    [MOUSE_MOD_CMD]   = "cmd",
    [MOUSE_MOD_CTRL]  = "ctrl",
    [MOUSE_MOD_FN]    = "fn",
};

static char *mouse_mode_str[] =
{
    [MOUSE_MODE_NONE]   = "none",
    [MOUSE_MODE_MOVE]   = "move",
    [MOUSE_MODE_RESIZE] = "resize",
    [MOUSE_MODE_SWAP]   = "swap",
    [MOUSE_MODE_STACK]  = "stack"
};

void mouse_window_info_populate(struct mouse_state *ms, struct mouse_window_info *info);
enum mouse_drop_action mouse_determine_drop_action(struct mouse_state *ms, struct window_node *src_node, struct window *dst_window, CGPoint point);
void mouse_drop_action_stack(struct space_manager *sm, struct window_manager *wm, struct view *src_view, struct window *src_window, struct view *dst_view, struct window *dst_window);
void mouse_drop_action_swap(struct window_manager *wm, struct view *src_view, struct window_node *src_node, struct window *src_window, struct view *dst_view, struct window_node *dst_node, struct window *dst_window);
void mouse_drop_action_warp(struct space_manager *sm, struct window_manager *wm, struct view *src_view, struct window_node *src_node, struct window *src_window, struct view *dst_view, struct window_node *dst_node, struct window *dst_window, enum window_node_split split, enum window_node_child child);
void mouse_drop_no_target(struct space_manager *sm, struct window_manager *wm, struct view *src_view, struct view *dst_view, struct window *window, struct window_node *node);
void mouse_drop_try_adjust_bsp_grid(struct window_manager *wm, struct view *view, struct window *window, struct mouse_window_info *info);

void mouse_state_init(struct mouse_state *mouse_state);
bool mouse_handler_begin(struct mouse_state *mouse_state, uint32_t mask);
void mouse_handler_end(struct mouse_state *mouse_state);

#endif

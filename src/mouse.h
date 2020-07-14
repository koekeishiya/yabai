#ifndef MOUSE_H
#define MOUSE_H

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
    MOUSE_MOD_FN    = 0x20,
    MOUSE_MOD_END   = 0x40,

    MOUSE_MOD_SUPER = MOUSE_MOD_ALT | MOUSE_MOD_SHIFT
                    | MOUSE_MOD_CMD | MOUSE_MOD_CTRL,
    MOUSE_MOD_INVALID = 0xff
};

enum mouse_mode
{
    MOUSE_MODE_NONE,
    MOUSE_MODE_MOVE,
    MOUSE_MODE_MOVE_LEGACY,
    MOUSE_MODE_RESIZE,
    MOUSE_MODE_SWAP,
    MOUSE_MODE_STACK
};

enum mouse_button
{
    MOUSE_BUTTON_LEFT   = kCGMouseButtonLeft,
    MOUSE_BUTTON_RIGHT  = kCGMouseButtonRight,
    MOUSE_BUTTON_MIDDLE = kCGMouseButtonCenter,
    MOUSE_BUTTON_NONE
};

struct mouse_action
{
    volatile uint8_t modifier;
    volatile int64_t button;
    volatile enum mouse_mode drag_action;
    volatile enum mouse_mode drop_action;
};

#define MOUSE_ACTION_NONE (-1)
#define MAX_MOUSE_ACTIONS 16

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
    volatile struct mouse_action actions[MAX_MOUSE_ACTIONS];
    enum mouse_mode current_action;
    enum mouse_mode drop_action;
    uint8_t modifier;
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
    [MOUSE_MODE_MOVE_LEGACY] = "move2",
    [MOUSE_MODE_RESIZE] = "resize",
    [MOUSE_MODE_SWAP]   = "swap",
    [MOUSE_MODE_STACK]  = "stack"
};

static char *mouse_button_str[] =
{
    [MOUSE_BUTTON_NONE]   = "none",
    [MOUSE_BUTTON_LEFT]   = "left",
    [MOUSE_BUTTON_RIGHT]  = "right",
    [MOUSE_BUTTON_MIDDLE] = "middle"
};

static inline uint8_t mouse_mod_from_cgflags(uint32_t cgflags)
{
    uint8_t flags = 0;

    if ((cgflags & EVENT_MASK_ALT)   == EVENT_MASK_ALT)   flags |= MOUSE_MOD_ALT;
    if ((cgflags & EVENT_MASK_SHIFT) == EVENT_MASK_SHIFT) flags |= MOUSE_MOD_SHIFT;
    if ((cgflags & EVENT_MASK_CMD)   == EVENT_MASK_CMD)   flags |= MOUSE_MOD_CMD;
    if ((cgflags & EVENT_MASK_CTRL)  == EVENT_MASK_CTRL)  flags |= MOUSE_MOD_CTRL;
    if ((cgflags & EVENT_MASK_FN)    == EVENT_MASK_FN)    flags |= MOUSE_MOD_FN;

    return flags;
}

static inline void mouse_action_set(
    struct mouse_state *state, int i,
    enum mouse_mod mod, enum mouse_button button,
    enum mouse_mode drag_action, enum mouse_mode drop_action)
{
    if (0 <= i && i < MAX_MOUSE_ACTIONS) {
        state->actions[i].modifier    = mod;
        state->actions[i].button      = button;
        state->actions[i].drag_action = drag_action;
        state->actions[i].drop_action = drop_action;
    }
}

static inline void mouse_action_clear(struct mouse_state *state, int i)
{
    if (0 <= i && i < MAX_MOUSE_ACTIONS)
        mouse_action_set(state, i, MOUSE_MOD_NONE, MOUSE_BUTTON_NONE,
                                   MOUSE_MODE_NONE, MOUSE_MODE_NONE);
}

static inline void mouse_action_clear_all(struct mouse_state *state)
{
    for (int i = 0; i < MAX_MOUSE_ACTIONS; i++)
        mouse_action_clear(state, i);
}

static inline void mouse_state_init(struct mouse_state *state)
{
    mouse_action_clear_all(state);
    mouse_action_set(state, 0, MOUSE_MOD_FN, MOUSE_BUTTON_LEFT,
                               MOUSE_MODE_MOVE, MOUSE_MODE_SWAP);
    mouse_action_set(state, 1, MOUSE_MOD_FN, MOUSE_BUTTON_RIGHT,
                               MOUSE_MODE_RESIZE, MOUSE_MODE_NONE);
    state->current_action = MOUSE_MODE_NONE;
    state->drop_action    = MOUSE_MODE_NONE;
    state->modifier       = MOUSE_MOD_INVALID;
}

static inline int mouse_action_get(struct mouse_state *state, enum mouse_mod mod, enum mouse_button button) {
    for (int i = 0; i < MAX_MOUSE_ACTIONS; i++) {
        if (button == state->actions[i].button &&
               mod == state->actions[i].modifier) 
            return i;
    }
    return MOUSE_ACTION_NONE;
}

void mouse_window_info_populate(struct mouse_state *ms, struct mouse_window_info *info);
enum mouse_drop_action mouse_determine_drop_action(struct mouse_state *ms, struct window_node *src_node, struct window *dst_window, CGPoint point);
void mouse_drop_action_stack(struct space_manager *sm, struct window_manager *wm, struct view *src_view, struct window *src_window, struct view *dst_view, struct window *dst_window);
void mouse_drop_action_swap(struct window_manager *wm, struct view *src_view, struct window_node *src_node, struct window *src_window, struct view *dst_view, struct window_node *dst_node, struct window *dst_window);
void mouse_drop_action_warp(struct space_manager *sm, struct window_manager *wm, struct view *src_view, struct window_node *src_node, struct window *src_window, struct view *dst_view, struct window_node *dst_node, struct window *dst_window, enum window_node_split split, enum window_node_child child);
void mouse_drop_no_target(struct space_manager *sm, struct window_manager *wm, struct view *src_view, struct view *dst_view, struct window *window, struct window_node *node);
void mouse_drop_try_adjust_bsp_grid(struct window_manager *wm, struct view *view, struct window *window, struct mouse_window_info *info);

#endif

#ifndef MOUSE_H
#define MOUSE_H

#define MOUSE_MOD_FLAG_MIN 0x02
#define MOUSE_MOD_FLAG_MAX 0x20

enum mouse_mod
{
    MOUSE_MOD_NONE  = 0x01,
    MOUSE_MOD_ALT   = 0x02,
    MOUSE_MOD_SHIFT = 0x04,
    MOUSE_MOD_CMD   = 0x08,
    MOUSE_MOD_CTRL  = 0x10,
    MOUSE_MOD_FN    = 0x20,
    MOUSE_MOD_SUPER = MOUSE_MOD_ALT | MOUSE_MOD_SHIFT | MOUSE_MOD_CMD | MOUSE_MOD_CTRL,
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

struct mouse_state
{
    enum mouse_mode current_action;
    enum mouse_mode action1;
    enum mouse_mode action2;
    enum mouse_mode drop_action;
    volatile uint8_t modifier;
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
    [MOUSE_MOD_SUPER] = "super",
};

static char *mouse_mode_str[] =
{
    [MOUSE_MODE_NONE]        = "none",
    [MOUSE_MODE_MOVE]        = "move",
    [MOUSE_MODE_MOVE_LEGACY] = "move2",
    [MOUSE_MODE_RESIZE]      = "resize",
    [MOUSE_MODE_SWAP]        = "swap",
    [MOUSE_MODE_STACK]       = "stack"
};

static uint8_t mouse_mod_from_cgflags(uint32_t cgflags)
{
    uint8_t flags = 0;
    if ((cgflags & EVENT_MASK_ALT)   == EVENT_MASK_ALT)   flags |= MOUSE_MOD_ALT;
    if ((cgflags & EVENT_MASK_SHIFT) == EVENT_MASK_SHIFT) flags |= MOUSE_MOD_SHIFT;
    if ((cgflags & EVENT_MASK_CMD)   == EVENT_MASK_CMD)   flags |= MOUSE_MOD_CMD;
    if ((cgflags & EVENT_MASK_CTRL)  == EVENT_MASK_CTRL)  flags |= MOUSE_MOD_CTRL;
    if ((cgflags & EVENT_MASK_FN)    == EVENT_MASK_FN)    flags |= MOUSE_MOD_FN;
    return flags;
}

static inline void mouse_state_init(struct mouse_state *state)
{
    state->modifier     = MOUSE_MOD_FN;
    state->action1      = MOUSE_MODE_MOVE;
    state->action2      = MOUSE_MODE_RESIZE;
    state->drop_action  = MOUSE_MODE_SWAP;
}

#endif

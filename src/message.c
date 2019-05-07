#include "message.h"

extern struct eventloop g_eventloop;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;

#define DOMAIN_CONFIG  "config"
#define DOMAIN_DISPLAY "display"
#define DOMAIN_SPACE   "space"
#define DOMAIN_WINDOW  "window"

/* --------------------------------DOMAIN CONFIG-------------------------------- */
#define COMMAND_CONFIG_MFF                   "mouse_follows_focus"
#define COMMAND_CONFIG_FFM                   "focus_follows_mouse"
#define COMMAND_CONFIG_SHADOW                "window_shadow"
#define COMMAND_CONFIG_BORDER                "window_border"
#define COMMAND_CONFIG_BORDER_WIDTH          "window_border_width"
#define COMMAND_CONFIG_ACTIVE_BORDER_COLOR   "active_window_border_color"
#define COMMAND_CONFIG_NORMAL_BORDER_COLOR   "normal_window_border_color"
#define COMMAND_CONFIG_INSERT_BORDER_COLOR   "insert_window_border_color"
#define COMMAND_CONFIG_TOP_PADDING           "top_padding"
#define COMMAND_CONFIG_BOTTOM_PADDING        "bottom_padding"
#define COMMAND_CONFIG_LEFT_PADDING          "left_padding"
#define COMMAND_CONFIG_RIGHT_PADDING         "right_padding"
#define COMMAND_CONFIG_WINDOW_GAP            "window_gap"
#define COMMAND_CONFIG_SPLIT_RATIO           "split_ratio"
#define COMMAND_CONFIG_AUTO_BALANCE          "auto_balance"

#define SELECTOR_CONFIG_SPACE                "--space"

#define ARGUMENT_CONFIG_MFF_ON               "on"
#define ARGUMENT_CONFIG_MFF_OFF              "off"
#define ARGUMENT_CONFIG_FFM_DISABLED         "off"
#define ARGUMENT_CONFIG_FFM_AUTOFOCUS        "autofocus"
#define ARGUMENT_CONFIG_FFM_AUTORAISE        "autoraise"
#define ARGUMENT_CONFIG_SHADOW_OFF           "off"
#define ARGUMENT_CONFIG_SHADOW_ALL           "on"
// #define ARGUMENT_CONFIG_SHADOW_FLT           "float"
// #define ARGUMENT_CONFIG_SHADOW_ALL           "always"
#define ARGUMENT_CONFIG_BORDER_ON            "on"
#define ARGUMENT_CONFIG_BORDER_OFF           "off"
#define ARGUMENT_CONFIG_AUTO_BALANCE_ON      "on"
#define ARGUMENT_CONFIG_AUTO_BALANCE_OFF     "off"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN DISPLAY------------------------------- */
#define COMMAND_DISPLAY_FOCUS "--focus"

#define ARGUMENT_DISPLAY_FOCUS_PREV "prev"
#define ARGUMENT_DISPLAY_FOCUS_NEXT "next"
#define ARGUMENT_DISPLAY_FOCUS_LAST "last"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN SPACE--------------------------------- */
#define COMMAND_SPACE_FOCUS   "--focus"
#define COMMAND_SPACE_MOVE    "--move"
#define COMMAND_SPACE_CREATE  "--create"
#define COMMAND_SPACE_DESTROY "--destroy"
#define COMMAND_SPACE_BALANCE "--balance"
#define COMMAND_SPACE_MIRROR  "--mirror"
#define COMMAND_SPACE_ROTATE  "--rotate"
#define COMMAND_SPACE_PADDING "--padding"
#define COMMAND_SPACE_TOGGLE  "--toggle"

#define ARGUMENT_SPACE_FOCUS_PREV  "prev"
#define ARGUMENT_SPACE_FOCUS_NEXT  "next"
#define ARGUMENT_SPACE_FOCUS_LAST  "last"
#define ARGUMENT_SPACE_MOVE_PREV   "prev"
#define ARGUMENT_SPACE_MOVE_NEXT   "next"
#define ARGUMENT_SPACE_MOVE_LAST   "last"
#define ARGUMENT_SPACE_MIRROR_X    "x-axis"
#define ARGUMENT_SPACE_MIRROR_Y    "y-axis"
#define ARGUMENT_SPACE_ROTATE_90   "90"
#define ARGUMENT_SPACE_ROTATE_180  "180"
#define ARGUMENT_SPACE_ROTATE_270  "270"
#define ARGUMENT_SPACE_PADDING     "%d:%d:%d:%d"
#define ARGUMENT_SPACE_TGL_PADDING "padding"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN WINDOW-------------------------------- */
#define COMMAND_WINDOW_FOCUS   "--focus"
#define COMMAND_WINDOW_SWAP    "--swap"
#define COMMAND_WINDOW_WARP    "--warp"
#define COMMAND_WINDOW_INSERT  "--insert"
#define COMMAND_WINDOW_GRID    "--grid"
#define COMMAND_WINDOW_TOGGLE  "--toggle"
#define COMMAND_WINDOW_DISPLAY "--display"
#define COMMAND_WINDOW_SPACE   "--space"

#define ARGUMENT_WINDOW_DIR_NORTH     "north"
#define ARGUMENT_WINDOW_DIR_EAST      "east"
#define ARGUMENT_WINDOW_DIR_SOUTH     "south"
#define ARGUMENT_WINDOW_DIR_WEST      "west"
#define ARGUMENT_WINDOW_GRID          "%d:%d:%d:%d:%d:%d"
#define ARGUMENT_WINDOW_TOGGLE_FLOAT  "float"
#define ARGUMENT_WINDOW_TOGGLE_STICKY "sticky"
#define ARGUMENT_WINDOW_TOGGLE_SPLIT  "split"
#define ARGUMENT_WINDOW_TOGGLE_FULLSC "fullscreen"
#define ARGUMENT_WINDOW_TOGGLE_NATIVE "native-fullscreen"
#define ARGUMENT_WINDOW_TOGGLE_BORDER "border"
#define ARGUMENT_WINDOW_SPACE_PREV    "prev"
#define ARGUMENT_WINDOW_SPACE_NEXT    "next"
#define ARGUMENT_WINDOW_SPACE_LAST    "last"
#define ARGUMENT_WINDOW_DISPLAY_PREV  "prev"
#define ARGUMENT_WINDOW_DISPLAY_NEXT  "next"
#define ARGUMENT_WINDOW_DISPLAY_LAST  "last"
/* ----------------------------------------------------------------------------- */

struct token
{
    char *text;
    unsigned int length;
};

static bool token_equals(struct token token, char *match)
{
    char *at = match;
    for (int i = 0; i < token.length; ++i, ++at) {
        if ((*at == 0) || (token.text[i] != *at)) {
            return false;
        }
    }
    return *at == 0;
}

static bool token_is_valid(struct token token)
{
    return token.text && token.length > 0;
}

static uint64_t token_to_uint64t(struct token token)
{
    uint64_t result = 0;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%lld", &result);
    return result;
}

static uint32_t token_to_uint32t(struct token token)
{
    uint32_t result = 0;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%x", &result);
    return result;
}

static int token_to_int(struct token token)
{
    int result = 0;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%d", &result);
    return result;
}

static float token_to_float(struct token token)
{
    float result = 0.0f;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%f", &result);
    return result;
}

static struct token get_token(char **message)
{
    struct token token;

    token.text = *message;
    while (**message && !isspace(**message)) {
        ++(*message);
    }
    token.length = *message - token.text;

    if (isspace(**message)) {
        ++(*message);
    } else {
        // NOTE(koekeishiya): don't go past the null-terminator
    }

    return token;
}

static void daemon_fail(FILE *rsp, char *fmt, ...)
{
    fprintf(rsp, FAILURE_MESSAGE);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(rsp, fmt, ap);
    va_end(ap);
}

static void handle_domain_config(FILE *rsp, struct token domain, char *message)
{
    int sel_mci = 0;

    struct token selector = get_token(&message);
    struct token command  = selector;

    if (token_equals(selector, SELECTOR_CONFIG_SPACE)) {
        struct token value = get_token(&message);
        if ((sel_mci = token_to_int(value)) == 0) {
            daemon_fail(rsp, "unknown value '%.*s' given to selector '%.*s' for domain '%.*s'\n", value.length, value.text, selector.length, selector.text, domain.length, domain.text);
            return;
        }

        command = get_token(&message);
    }

    if (token_equals(command, COMMAND_CONFIG_MFF)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_CONFIG_MFF_OFF)) {
            g_window_manager.enable_mff = false;
        } else if (token_equals(value, ARGUMENT_CONFIG_MFF_ON)) {
            g_window_manager.enable_mff = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_FFM)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_CONFIG_FFM_DISABLED)) {
            g_window_manager.ffm_mode = FFM_DISABLED;
        } else if (token_equals(value, ARGUMENT_CONFIG_FFM_AUTOFOCUS)) {
            g_window_manager.ffm_mode = FFM_AUTOFOCUS;
        } else if (token_equals(value, ARGUMENT_CONFIG_FFM_AUTORAISE)) {
            g_window_manager.ffm_mode = FFM_AUTORAISE;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_SHADOW)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_CONFIG_SHADOW_OFF)) {
            g_window_manager.purify_mode = PURIFY_ALWAYS;
#if 0
        } else if (token_equals(value, ARGUMENT_CONFIG_SHADOW_FLT)) {
            g_window_manager.purify_mode = PURIFY_MANAGED;
#endif
        } else if (token_equals(value, ARGUMENT_CONFIG_SHADOW_ALL)) {
            g_window_manager.purify_mode = PURIFY_DISABLED;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_CONFIG_BORDER_OFF)) {
            g_window_manager.enable_window_border = false;
        } else if (token_equals(value, ARGUMENT_CONFIG_BORDER_ON)) {
            g_window_manager.enable_window_border = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_WIDTH)) {
        struct token value = get_token(&message);
        int width = token_to_int(value);
        if (width) {
            g_window_manager.window_border_width = width;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_ACTIVE_BORDER_COLOR)) {
        struct token value = get_token(&message);
        uint32_t color = token_to_uint32t(value);
        if (color) {
            g_window_manager.active_window_border_color = color;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_NORMAL_BORDER_COLOR)) {
        struct token value = get_token(&message);
        uint32_t color = token_to_uint32t(value);
        if (color) {
            g_window_manager.normal_window_border_color = color;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_INSERT_BORDER_COLOR)) {
        struct token value = get_token(&message);
        uint32_t color = token_to_uint32t(value);
        if (color) {
            g_window_manager.insert_window_border_color = color;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_TOP_PADDING)) {
        struct token value = get_token(&message);
        g_space_manager.top_padding[sel_mci] = token_to_int(value);
    } else if (token_equals(command, COMMAND_CONFIG_BOTTOM_PADDING)) {
        struct token value = get_token(&message);
        g_space_manager.bottom_padding[sel_mci] = token_to_int(value);
    } else if (token_equals(command, COMMAND_CONFIG_LEFT_PADDING)) {
        struct token value = get_token(&message);
        g_space_manager.left_padding[sel_mci] = token_to_int(value);
    } else if (token_equals(command, COMMAND_CONFIG_RIGHT_PADDING)) {
        struct token value = get_token(&message);
        g_space_manager.right_padding[sel_mci] = token_to_int(value);
    } else if (token_equals(command, COMMAND_CONFIG_WINDOW_GAP)) {
        struct token value = get_token(&message);
        g_space_manager.window_gap = token_to_int(value);
    } else if (token_equals(command, COMMAND_CONFIG_SPLIT_RATIO)) {
        struct token value = get_token(&message);
        g_space_manager.split_ratio = token_to_float(value);
    } else if (token_equals(command, COMMAND_CONFIG_AUTO_BALANCE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_CONFIG_AUTO_BALANCE_OFF)) {
            g_space_manager.auto_balance = false;
        } else if (token_equals(value, ARGUMENT_CONFIG_AUTO_BALANCE_ON)) {
            g_space_manager.auto_balance = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static void handle_domain_display(FILE *rsp, struct token domain, char *message)
{
    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_DISPLAY_FOCUS)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_DISPLAY_FOCUS_PREV)) {
            uint32_t did = display_manager_prev_display_id(display_manager_active_display_id());
            if (did) {
                display_manager_focus_display(did);
            } else {
                daemon_fail(rsp, "could not locate the previous display.\n");
            }
        } else if (token_equals(value, ARGUMENT_DISPLAY_FOCUS_NEXT)) {
            uint32_t did = display_manager_next_display_id(display_manager_active_display_id());
            if (did) {
                display_manager_focus_display(did);
            } else {
                daemon_fail(rsp, "could not locate the next display.\n");
            }
        } else if (token_equals(value, ARGUMENT_DISPLAY_FOCUS_LAST)) {
            display_manager_focus_display(g_display_manager.last_display_id);
        } else if (token_is_valid(value)) {
            int arrangement_index = token_to_int(value);
            uint32_t did = display_manager_arrangement_display_id(arrangement_index);
            if (did) {
                display_manager_focus_display(did);
            } else {
                daemon_fail(rsp, "could not locate display with arrangement index '%d'.\n", arrangement_index);
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static void handle_domain_space(FILE *rsp, struct token domain, char *message)
{
    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_SPACE_FOCUS)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_FOCUS_PREV)) {
            uint64_t sid = space_manager_prev_space(space_manager_active_space());
            if (sid) {
                space_manager_focus_space(sid);
            } else {
                daemon_fail(rsp, "could not locate the previous space.\n");
            }
        } else if (token_equals(value, ARGUMENT_SPACE_FOCUS_NEXT)) {
            uint64_t sid = space_manager_next_space(space_manager_active_space());
            if (sid) {
                space_manager_focus_space(sid);
            } else {
                daemon_fail(rsp, "could not locate the next space.\n");
            }
        } else if (token_equals(value, ARGUMENT_SPACE_FOCUS_LAST)) {
            space_manager_focus_space(g_space_manager.last_space_id);
        } else if (token_is_valid(value)) {
            int mci = token_to_int(value);
            uint64_t sid = space_manager_mission_control_space(mci);
            if (sid) {
                space_manager_focus_space(sid);
            } else {
                daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", mci);
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_MOVE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_MOVE_PREV)) {
            uint32_t did = display_manager_prev_display_id(display_manager_active_display_id());
            if (did) {
                space_manager_move_space_to_display(&g_space_manager, did);
            } else {
                daemon_fail(rsp, "could not locate the previous display.\n");
            }
        } else if (token_equals(value, ARGUMENT_SPACE_MOVE_NEXT)) {
            uint32_t did = display_manager_next_display_id(display_manager_active_display_id());
            if (did) {
                space_manager_move_space_to_display(&g_space_manager, did);
            } else {
                daemon_fail(rsp, "could not locate the next display.\n");
            }
        } else if (token_equals(value, ARGUMENT_SPACE_MOVE_LAST)) {
            space_manager_move_space_to_display(&g_space_manager, g_display_manager.last_display_id);
        } else if (token_is_valid(value)) {
            int arrangement_index = token_to_int(value);
            uint32_t did = display_manager_arrangement_display_id(arrangement_index);
            if (did) {
                space_manager_move_space_to_display(&g_space_manager, did);
            } else {
                daemon_fail(rsp, "could not locate display with arrangement index '%d'.\n", arrangement_index);
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_CREATE)) {
        space_manager_add_space();
    } else if (token_equals(command, COMMAND_SPACE_DESTROY)) {
        space_manager_destroy_space();
    } else if (token_equals(command, COMMAND_SPACE_BALANCE)) {
        space_manager_balance_space(&g_space_manager, space_manager_active_space());
    } else if (token_equals(command, COMMAND_SPACE_MIRROR)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_MIRROR_X)) {
            space_manager_mirror_space(&g_space_manager, space_manager_active_space(), SPLIT_X);
        } else if (token_equals(value, ARGUMENT_SPACE_MIRROR_Y)) {
            space_manager_mirror_space(&g_space_manager, space_manager_active_space(), SPLIT_Y);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_ROTATE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_ROTATE_90)) {
            space_manager_rotate_space(&g_space_manager, space_manager_active_space(), 90);
        } else if (token_equals(value, ARGUMENT_SPACE_ROTATE_180)) {
            space_manager_rotate_space(&g_space_manager, space_manager_active_space(), 180);
        } else if (token_equals(value, ARGUMENT_SPACE_ROTATE_270)) {
            space_manager_rotate_space(&g_space_manager, space_manager_active_space(), 270);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_PADDING)) {
        struct token value = get_token(&message);
        unsigned t, b, l, r;
        if ((sscanf(value.text, ARGUMENT_SPACE_PADDING, &t, &b, &l, &r) == 4)) {
            space_manager_set_padding_for_space(&g_space_manager, space_manager_active_space(), t, b, l, r);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_TOGGLE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_TGL_PADDING)) {
            space_manager_toggle_padding_for_space(&g_space_manager, space_manager_active_space());
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static void handle_domain_window(FILE *rsp, struct token domain, char *message)
{
    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_WINDOW_FOCUS)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_DIR_NORTH)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, window, DIR_NORTH);
            if (closest_window) window_manager_focus_window_with_raise(closest_window->id);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_EAST)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, window, DIR_EAST);
            if (closest_window) window_manager_focus_window_with_raise(closest_window->id);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_SOUTH)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, window, DIR_SOUTH);
            if (closest_window) window_manager_focus_window_with_raise(closest_window->id);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_WEST)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, window, DIR_WEST);
            if (closest_window) window_manager_focus_window_with_raise(closest_window->id);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_SWAP)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_DIR_NORTH)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_NORTH);
            if (closest_window) window_manager_swap_window(&g_space_manager, &g_window_manager, window, closest_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_EAST)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_EAST);
            if (closest_window) window_manager_swap_window(&g_space_manager, &g_window_manager, window, closest_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_SOUTH)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_SOUTH);
            if (closest_window) window_manager_swap_window(&g_space_manager, &g_window_manager, window, closest_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_WEST)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_WEST);
            if (closest_window) window_manager_swap_window(&g_space_manager, &g_window_manager, window, closest_window);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_WARP)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_DIR_NORTH)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_NORTH);
            if (closest_window) window_manager_warp_window(&g_space_manager, window, closest_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_EAST)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_EAST);
            if (closest_window) window_manager_warp_window(&g_space_manager, window, closest_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_SOUTH)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_SOUTH);
            if (closest_window) window_manager_warp_window(&g_space_manager, window, closest_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_WEST)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            struct ax_window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_WEST);
            if (closest_window) window_manager_warp_window(&g_space_manager, window, closest_window);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_INSERT)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_DIR_NORTH)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            window_manager_set_window_insertion(&g_space_manager, window, DIR_NORTH);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_EAST)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            window_manager_set_window_insertion(&g_space_manager, window, DIR_EAST);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_SOUTH)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            window_manager_set_window_insertion(&g_space_manager, window, DIR_SOUTH);
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_WEST)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (!window) return;
            window_manager_set_window_insertion(&g_space_manager, window, DIR_WEST);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_GRID)) {
        struct token value = get_token(&message);
        unsigned r, c, x, y, w, h;
        if ((sscanf(value.text, ARGUMENT_WINDOW_GRID, &r, &c, &x, &y, &w, &h) == 6)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (window) window_manager_apply_grid(&g_space_manager, &g_window_manager, window, r, c, x, y, w, h);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_TOGGLE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_FLOAT)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (window) window_manager_toggle_window_float(&g_space_manager, &g_window_manager, window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_STICKY)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (window) window_manager_toggle_window_sticky(&g_space_manager, &g_window_manager, window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_SPLIT)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (window) space_manager_toggle_window_split(&g_space_manager, window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_FULLSC)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (window) window_manager_toggle_window_fullscreen(&g_space_manager, &g_window_manager, window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_NATIVE)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (window) window_manager_toggle_window_native_fullscreen(&g_space_manager, &g_window_manager, window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_BORDER)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (window) window_manager_toggle_window_border(window);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_DISPLAY)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_DISPLAY_PREV)) {
            uint32_t did = display_manager_prev_display_id(display_manager_active_display_id());
            if (did) {
                struct ax_window *window = window_manager_focused_window(&g_window_manager);
                if (window) window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, display_space_id(did));
            } else {
                daemon_fail(rsp, "could not locate the previous display.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DISPLAY_NEXT)) {
            uint32_t did = display_manager_next_display_id(display_manager_active_display_id());
            if (did) {
                struct ax_window *window = window_manager_focused_window(&g_window_manager);
                if (window) window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, display_space_id(did));
            } else {
                daemon_fail(rsp, "could not locate the next display.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DISPLAY_LAST)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (window) window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, display_space_id(g_display_manager.last_display_id));
        } else if (token_is_valid(value)) {
            int arrangement_index = token_to_int(value);
            uint32_t did = display_manager_arrangement_display_id(arrangement_index);
            if (did) {
                struct ax_window *window = window_manager_focused_window(&g_window_manager);
                if (window) window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, display_space_id(did));
            } else {
                daemon_fail(rsp, "could not locate display with arrangement index '%d'.\n", arrangement_index);
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_SPACE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_SPACE_PREV)) {
            uint64_t sid = space_manager_prev_space(space_manager_active_space());
            if (sid) {
                struct ax_window *window = window_manager_focused_window(&g_window_manager);
                if (window) window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, sid);
            } else {
                daemon_fail(rsp, "could not locate the previous space.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_SPACE_NEXT)) {
            uint64_t sid = space_manager_next_space(space_manager_active_space());
            if (sid) {
                struct ax_window *window = window_manager_focused_window(&g_window_manager);
                if (window) window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, sid);
            } else {
                daemon_fail(rsp, "could not locate the next space.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_SPACE_LAST)) {
            struct ax_window *window = window_manager_focused_window(&g_window_manager);
            if (window) window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, g_space_manager.last_space_id);
        } else if (token_is_valid(value)) {
            int mci = token_to_int(value);
            uint64_t sid = space_manager_mission_control_space(mci);
            if (sid) {
                struct ax_window *window = window_manager_focused_window(&g_window_manager);
                if (window) window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, sid);
            } else {
                daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", mci);
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

void handle_message(FILE *rsp, char *message)
{
    struct token domain = get_token(&message);
    if (token_equals(domain, DOMAIN_CONFIG)) {
        handle_domain_config(rsp, domain, message);
    } else if (token_equals(domain, DOMAIN_DISPLAY)) {
        handle_domain_display(rsp, domain, message);
    } else if (token_equals(domain, DOMAIN_SPACE)) {
        handle_domain_space(rsp, domain, message);
    } else if (token_equals(domain, DOMAIN_WINDOW)) {
        handle_domain_window(rsp, domain, message);
    } else {
        daemon_fail(rsp, "unknown domain '%.*s'\n", domain.length, domain.text);
    }
}

static SOCKET_DAEMON_HANDLER(message_handler)
{
    struct event *event;
    event_create_p1(event, DAEMON_MESSAGE, message, sockfd);
    eventloop_post(&g_eventloop, event);
}

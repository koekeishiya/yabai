#include "message.h"

extern struct signal *g_signal_event[EVENT_TYPE_COUNT];
extern struct event_loop g_event_loop;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;
extern struct mouse_state g_mouse_state;
extern struct bar g_bar;

static const char *bool_str[] = { "off", "on" };

#define DOMAIN_CONFIG  "config"
#define DOMAIN_DISPLAY "display"
#define DOMAIN_SPACE   "space"
#define DOMAIN_WINDOW  "window"
#define DOMAIN_QUERY   "query"
#define DOMAIN_RULE    "rule"
#define DOMAIN_SIGNAL  "signal"

/* --------------------------------DOMAIN CONFIG-------------------------------- */
#define COMMAND_CONFIG_MFF                   "mouse_follows_focus"
#define COMMAND_CONFIG_FFM                   "focus_follows_mouse"
#define COMMAND_CONFIG_WINDOW_PLACEMENT      "window_placement"
#define COMMAND_CONFIG_TOPMOST               "window_topmost"
#define COMMAND_CONFIG_OPACITY               "window_opacity"
#define COMMAND_CONFIG_SHADOW                "window_shadow"
#define COMMAND_CONFIG_BORDER                "window_border"
#define COMMAND_CONFIG_BORDER_WIDTH          "window_border_width"
#define COMMAND_CONFIG_ACTIVE_WINDOW_OPACITY "active_window_opacity"
#define COMMAND_CONFIG_NORMAL_WINDOW_OPACITY "normal_window_opacity"
#define COMMAND_CONFIG_ACTIVE_BORDER_COLOR   "active_window_border_color"
#define COMMAND_CONFIG_NORMAL_BORDER_COLOR   "normal_window_border_color"
#define COMMAND_CONFIG_INSERT_BORDER_COLOR   "insert_window_border_color"
#define COMMAND_CONFIG_TOP_PADDING           "top_padding"
#define COMMAND_CONFIG_BOTTOM_PADDING        "bottom_padding"
#define COMMAND_CONFIG_LEFT_PADDING          "left_padding"
#define COMMAND_CONFIG_RIGHT_PADDING         "right_padding"
#define COMMAND_CONFIG_LAYOUT                "layout"
#define COMMAND_CONFIG_WINDOW_GAP            "window_gap"
#define COMMAND_CONFIG_SPLIT_RATIO           "split_ratio"
#define COMMAND_CONFIG_AUTO_BALANCE          "auto_balance"
#define COMMAND_CONFIG_MOUSE_MOD             "mouse_modifier"
#define COMMAND_CONFIG_MOUSE_ACTION1         "mouse_action1"
#define COMMAND_CONFIG_MOUSE_ACTION2         "mouse_action2"
#define COMMAND_CONFIG_BAR                   "status_bar"
#define COMMAND_CONFIG_BAR_TEXT_FONT         "status_bar_text_font"
#define COMMAND_CONFIG_BAR_ICON_FONT         "status_bar_icon_font"
#define COMMAND_CONFIG_BAR_BACKGROUND        "status_bar_background_color"
#define COMMAND_CONFIG_BAR_FOREGROUND        "status_bar_foreground_color"
#define COMMAND_CONFIG_BAR_SPACE_STRIP       "status_bar_space_icon_strip"
#define COMMAND_CONFIG_BAR_POWER_STRIP       "status_bar_power_icon_strip"
#define COMMAND_CONFIG_BAR_SPACE_ICON        "status_bar_space_icon"
#define COMMAND_CONFIG_BAR_CLOCK_ICON        "status_bar_clock_icon"

#define SELECTOR_CONFIG_SPACE                "--space"

#define ARGUMENT_CONFIG_FFM_AUTOFOCUS        "autofocus"
#define ARGUMENT_CONFIG_FFM_AUTORAISE        "autoraise"
#define ARGUMENT_CONFIG_WINDOW_PLACEMENT_FST "first_child"
#define ARGUMENT_CONFIG_WINDOW_PLACEMENT_SND "second_child"
#define ARGUMENT_CONFIG_SHADOW_FLT           "float"
#define ARGUMENT_CONFIG_LAYOUT_BSP           "bsp"
#define ARGUMENT_CONFIG_LAYOUT_FLOAT         "float"
#define ARGUMENT_CONFIG_MOUSE_MOD_ALT        "alt"
#define ARGUMENT_CONFIG_MOUSE_MOD_SHIFT      "shift"
#define ARGUMENT_CONFIG_MOUSE_MOD_CMD        "cmd"
#define ARGUMENT_CONFIG_MOUSE_MOD_CTRL       "ctrl"
#define ARGUMENT_CONFIG_MOUSE_MOD_FN         "fn"
#define ARGUMENT_CONFIG_MOUSE_ACTION_MOVE    "move"
#define ARGUMENT_CONFIG_MOUSE_ACTION_RESIZE  "resize"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN DISPLAY------------------------------- */
#define COMMAND_DISPLAY_FOCUS "--focus"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN SPACE--------------------------------- */
#define COMMAND_SPACE_FOCUS   "--focus"
#define COMMAND_SPACE_CREATE  "--create"
#define COMMAND_SPACE_DESTROY "--destroy"
#define COMMAND_SPACE_MOVE    "--move"
#define COMMAND_SPACE_DISPLAY "--display"
#define COMMAND_SPACE_BALANCE "--balance"
#define COMMAND_SPACE_MIRROR  "--mirror"
#define COMMAND_SPACE_ROTATE  "--rotate"
#define COMMAND_SPACE_PADDING "--padding"
#define COMMAND_SPACE_GAP     "--gap"
#define COMMAND_SPACE_TOGGLE  "--toggle"
#define COMMAND_SPACE_LAYOUT  "--layout"

#define ARGUMENT_SPACE_MIRROR_X     "x-axis"
#define ARGUMENT_SPACE_MIRROR_Y     "y-axis"
#define ARGUMENT_SPACE_ROTATE_90    "90"
#define ARGUMENT_SPACE_ROTATE_180   "180"
#define ARGUMENT_SPACE_ROTATE_270   "270"
#define ARGUMENT_SPACE_PADDING      "%255[^:]:%d:%d:%d:%d"
#define ARGUMENT_SPACE_GAP          "%255[^:]:%d"
#define ARGUMENT_SPACE_TGL_PADDING  "padding"
#define ARGUMENT_SPACE_TGL_GAP      "gap"
#define ARGUMENT_SPACE_LAYOUT_BSP   "bsp"
#define ARGUMENT_SPACE_LAYOUT_FLT   "float"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN WINDOW-------------------------------- */
#define COMMAND_WINDOW_FOCUS   "--focus"
#define COMMAND_WINDOW_SWAP    "--swap"
#define COMMAND_WINDOW_WARP    "--warp"
#define COMMAND_WINDOW_INSERT  "--insert"
#define COMMAND_WINDOW_GRID    "--grid"
#define COMMAND_WINDOW_MOVE    "--move"
#define COMMAND_WINDOW_RESIZE  "--resize"
#define COMMAND_WINDOW_TOGGLE  "--toggle"
#define COMMAND_WINDOW_DISPLAY "--display"
#define COMMAND_WINDOW_SPACE   "--space"

#define ARGUMENT_WINDOW_DIR_NORTH     "north"
#define ARGUMENT_WINDOW_DIR_EAST      "east"
#define ARGUMENT_WINDOW_DIR_SOUTH     "south"
#define ARGUMENT_WINDOW_DIR_WEST      "west"
#define ARGUMENT_WINDOW_SEL_MOUSE     "mouse"
#define ARGUMENT_WINDOW_GRID          "%d:%d:%d:%d:%d:%d"
#define ARGUMENT_WINDOW_MOVE          "%255[^:]:%f:%f"
#define ARGUMENT_WINDOW_RESIZE        "%255[^:]:%f:%f"
#define ARGUMENT_WINDOW_TOGGLE_FLOAT  "float"
#define ARGUMENT_WINDOW_TOGGLE_STICKY "sticky"
#define ARGUMENT_WINDOW_TOGGLE_SPLIT  "split"
#define ARGUMENT_WINDOW_TOGGLE_PARENT "zoom-parent"
#define ARGUMENT_WINDOW_TOGGLE_FULLSC "zoom-fullscreen"
#define ARGUMENT_WINDOW_TOGGLE_NATIVE "native-fullscreen"
#define ARGUMENT_WINDOW_TOGGLE_BORDER "border"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN QUERY--------------------------------- */
#define COMMAND_QUERY_DISPLAYS "--displays"
#define COMMAND_QUERY_SPACES   "--spaces"
#define COMMAND_QUERY_WINDOWS  "--windows"

#define ARGUMENT_QUERY_DISPLAY "--display"
#define ARGUMENT_QUERY_SPACE   "--space"
#define ARGUMENT_QUERY_WINDOW  "--window"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN RULE---------------------------------- */
#define COMMAND_RULE_ADD "--add"
#define COMMAND_RULE_REM "--remove"

#define ARGUMENT_RULE_KEY_APP     "app"
#define ARGUMENT_RULE_KEY_TITLE   "title"
#define ARGUMENT_RULE_KEY_DISPLAY "display"
#define ARGUMENT_RULE_KEY_SPACE   "space"
#define ARGUMENT_RULE_KEY_ALPHA   "opacity"
#define ARGUMENT_RULE_KEY_MANAGE  "manage"
#define ARGUMENT_RULE_KEY_STICKY  "sticky"
#define ARGUMENT_RULE_KEY_BORDER  "border"
#define ARGUMENT_RULE_KEY_FULLSCR "native-fullscreen"
#define ARGUMENT_RULE_KEY_GRID    "grid"
#define ARGUMENT_RULE_KEY_LABEL   "label"

#define ARGUMENT_RULE_VALUE_SPACE '^'
#define ARGUMENT_RULE_VALUE_GRID  "%d:%d:%d:%d:%d:%d"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN SIGNAL-------------------------------- */
#define COMMAND_SIGNAL_ADD "--add"
#define COMMAND_SIGNAL_REM "--remove"

#define ARGUMENT_SIGNAL_KEY_EVENT    "event"
#define ARGUMENT_SIGNAL_KEY_ACTION   "action"
#define ARGUMENT_SIGNAL_KEY_LABEL    "label"
/* ----------------------------------------------------------------------------- */

/* --------------------------------COMMON ARGUMENTS----------------------------- */
#define ARGUMENT_COMMON_VAL_ON     "on"
#define ARGUMENT_COMMON_VAL_OFF    "off"
#define ARGUMENT_COMMON_SEL_PREV   "prev"
#define ARGUMENT_COMMON_SEL_NEXT   "next"
#define ARGUMENT_COMMON_SEL_FIRST  "first"
#define ARGUMENT_COMMON_SEL_LAST   "last"
#define ARGUMENT_COMMON_SEL_RECENT "recent"
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

static char *token_to_string(struct token token)
{
    char *result = malloc(token.length + 1);
    if (!result) return NULL;

    memcpy(result, token.text, token.length);
    result[token.length] = '\0';
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
    while (**message) {
        ++(*message);
    }
    token.length = *message - token.text;

    if ((*message)[0] == '\0' && (*message)[1] != '\0') {
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

#define VIEW_SET_PROPERTY(p) \
                    view->custom_##p = true; \
                    view->p = token_to_int(value); \
                    view_update(view); \
                    view_flush(view);

static void handle_domain_config(FILE *rsp, struct token domain, char *message)
{
    int sel_mci = 0;
    uint64_t sel_sid = 0;
    bool found_selector = true;

    struct token selector = get_token(&message);
    struct token command  = selector;

    if (token_equals(selector, SELECTOR_CONFIG_SPACE)) {
        struct token value = get_token(&message);
        sel_mci = token_to_int(value);
        if (sel_mci) sel_sid = space_manager_mission_control_space(sel_mci);
    } else {
        found_selector = false;
    }

    if (found_selector) command = get_token(&message);

    if (token_equals(command, COMMAND_CONFIG_MFF)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", bool_str[g_window_manager.enable_mff]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            g_window_manager.enable_mff = false;
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            g_window_manager.enable_mff = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_FFM)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", ffm_mode_str[g_window_manager.ffm_mode]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            g_window_manager.ffm_mode = FFM_DISABLED;
        } else if (token_equals(value, ARGUMENT_CONFIG_FFM_AUTOFOCUS)) {
            g_window_manager.ffm_mode = FFM_AUTOFOCUS;
        } else if (token_equals(value, ARGUMENT_CONFIG_FFM_AUTORAISE)) {
            g_window_manager.ffm_mode = FFM_AUTORAISE;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_WINDOW_PLACEMENT)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", window_node_child_str[g_space_manager.window_placement]);
        } else if (token_equals(value, ARGUMENT_CONFIG_WINDOW_PLACEMENT_FST)) {
            g_space_manager.window_placement = CHILD_FIRST;
        } else if (token_equals(value, ARGUMENT_CONFIG_WINDOW_PLACEMENT_SND)) {
            g_space_manager.window_placement = CHILD_SECOND;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_TOPMOST)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", bool_str[g_window_manager.enable_window_topmost]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            g_window_manager.enable_window_topmost = false;
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            g_window_manager.enable_window_topmost = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_OPACITY)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", bool_str[g_window_manager.enable_window_opacity]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            g_window_manager.enable_window_opacity = false;
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            g_window_manager.enable_window_opacity = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_SHADOW)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", purify_mode_str[g_window_manager.purify_mode]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            window_manager_set_purify_mode(&g_window_manager, PURIFY_ALWAYS);
        } else if (token_equals(value, ARGUMENT_CONFIG_SHADOW_FLT)) {
            window_manager_set_purify_mode(&g_window_manager, PURIFY_MANAGED);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            window_manager_set_purify_mode(&g_window_manager, PURIFY_DISABLED);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", bool_str[g_window_manager.enable_window_border]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            g_window_manager.enable_window_border = false;
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            g_window_manager.enable_window_border = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_WIDTH)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%d\n", g_window_manager.window_border_width);
        } else {
            int width = token_to_int(value);
            if (width) {
                window_manager_set_border_window_width(&g_window_manager, width);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_ACTIVE_WINDOW_OPACITY)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%.4f\n", g_window_manager.active_window_opacity);
        } else {
            float opacity = token_to_float(value);
            if (opacity > 0.0f && opacity <= 1.0f) {
                window_manager_set_active_window_opacity(&g_window_manager, opacity);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_NORMAL_WINDOW_OPACITY)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%.4f\n", g_window_manager.normal_window_opacity);
        } else {
            float opacity = token_to_float(value);
            if (opacity > 0.0f && opacity <= 1.0f) {
                window_manager_set_normal_window_opacity(&g_window_manager, opacity);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_ACTIVE_BORDER_COLOR)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "0x%x\n", g_window_manager.active_window_border_color);
        } else {
            uint32_t color = token_to_uint32t(value);
            if (color) {
                window_manager_set_active_border_window_color(&g_window_manager, color);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_NORMAL_BORDER_COLOR)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "0x%x\n", g_window_manager.normal_window_border_color);
        } else {
            uint32_t color = token_to_uint32t(value);
            if (color) {
                window_manager_set_normal_border_window_color(&g_window_manager, color);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_INSERT_BORDER_COLOR)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "0x%x\n", g_window_manager.insert_window_border_color);
        } else {
            uint32_t color = token_to_uint32t(value);
            if (color) {
                g_window_manager.insert_window_border_color = color;
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_TOP_PADDING)) {
        struct token value = get_token(&message);
        if (sel_mci) {
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (!token_is_valid(value)) {
                    fprintf(rsp, "%d\n", view->top_padding);
                } else {
                    VIEW_SET_PROPERTY(top_padding);
                }
            } else {
                daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", sel_mci);
            }
        } else {
            if (!token_is_valid(value)) {
                fprintf(rsp, "%d\n", g_space_manager.top_padding);
            } else {
                space_manager_set_top_padding_for_all_spaces(&g_space_manager, token_to_int(value));
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_BOTTOM_PADDING)) {
        struct token value = get_token(&message);
        if (sel_mci) {
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (!token_is_valid(value)) {
                    fprintf(rsp, "%d\n", view->bottom_padding);
                } else {
                    VIEW_SET_PROPERTY(bottom_padding);
                }
            } else {
                daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", sel_mci);
            }
        } else {
            if (!token_is_valid(value)) {
                fprintf(rsp, "%d\n", g_space_manager.bottom_padding);
            } else {
                space_manager_set_bottom_padding_for_all_spaces(&g_space_manager, token_to_int(value));
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_LEFT_PADDING)) {
        struct token value = get_token(&message);
        if (sel_mci) {
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (!token_is_valid(value)) {
                    fprintf(rsp, "%d\n", view->left_padding);
                } else {
                    VIEW_SET_PROPERTY(left_padding);
                }
            } else {
                daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", sel_mci);
            }
        } else {
            if (!token_is_valid(value)) {
                fprintf(rsp, "%d\n", g_space_manager.left_padding);
            } else {
                space_manager_set_left_padding_for_all_spaces(&g_space_manager, token_to_int(value));
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_RIGHT_PADDING)) {
        struct token value = get_token(&message);
        if (sel_mci) {
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (!token_is_valid(value)) {
                    fprintf(rsp, "%d\n", view->right_padding);
                } else {
                    VIEW_SET_PROPERTY(right_padding);
                }
            } else {
                daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", sel_mci);
            }
        } else {
            if (!token_is_valid(value)) {
                fprintf(rsp, "%d\n", g_space_manager.right_padding);
            } else {
                space_manager_set_right_padding_for_all_spaces(&g_space_manager, token_to_int(value));
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_WINDOW_GAP)) {
        struct token value = get_token(&message);
        if (sel_mci) {
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (!token_is_valid(value)) {
                    fprintf(rsp, "%d\n", view->window_gap);
                } else {
                    VIEW_SET_PROPERTY(window_gap);
                }
            } else {
                daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", sel_mci);
            }
        } else {
            if (!token_is_valid(value)) {
                fprintf(rsp, "%d\n", g_space_manager.window_gap);
            } else {
                space_manager_set_window_gap_for_all_spaces(&g_space_manager, token_to_int(value));
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_LAYOUT)) {
        struct token value = get_token(&message);
        if (sel_mci) {
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (!token_is_valid(value)) {
                    fprintf(rsp, "%s\n", view_type_str[view->layout]);
                } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_BSP)) {
                    view->layout = VIEW_BSP;
                    view->custom_layout = true;
                    window_manager_check_for_windows_on_space(&g_space_manager, &g_window_manager, sel_sid);
                } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_FLOAT)) {
                    view->layout = VIEW_FLOAT;
                    view->custom_layout = true;
                } else {
                    daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
                }
            } else {
                daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", sel_mci);
            }
        } else {
            if (!token_is_valid(value)) {
                fprintf(rsp, "%s\n", view_type_str[g_space_manager.layout]);
            } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_BSP)) {
                space_manager_set_layout_for_all_spaces(&g_space_manager, VIEW_BSP);
            } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_FLOAT)) {
                space_manager_set_layout_for_all_spaces(&g_space_manager, VIEW_FLOAT);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_SPLIT_RATIO)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%.4f\n", g_space_manager.split_ratio);
        } else {
            g_space_manager.split_ratio = token_to_float(value);
        }
    } else if (token_equals(command, COMMAND_CONFIG_AUTO_BALANCE)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", bool_str[g_space_manager.auto_balance]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            g_space_manager.auto_balance = false;
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            g_space_manager.auto_balance = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_MOUSE_MOD)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", mouse_mod_str[g_mouse_state.modifier]);
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_MOD_ALT)) {
            g_mouse_state.modifier = MOUSE_MOD_ALT;
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_MOD_SHIFT)) {
            g_mouse_state.modifier = MOUSE_MOD_SHIFT;
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_MOD_CMD)) {
            g_mouse_state.modifier = MOUSE_MOD_CMD;
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_MOD_CTRL)) {
            g_mouse_state.modifier = MOUSE_MOD_CTRL;
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_MOD_FN)) {
            g_mouse_state.modifier = MOUSE_MOD_FN;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_MOUSE_ACTION1)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", mouse_mode_str[g_mouse_state.action1]);
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_ACTION_MOVE)) {
            g_mouse_state.action1 = MOUSE_MODE_MOVE;
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_ACTION_RESIZE)) {
            g_mouse_state.action1 = MOUSE_MODE_RESIZE;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_MOUSE_ACTION2)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", mouse_mode_str[g_mouse_state.action2]);
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_ACTION_MOVE)) {
            g_mouse_state.action2 = MOUSE_MODE_MOVE;
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_ACTION_RESIZE)) {
            g_mouse_state.action2 = MOUSE_MODE_RESIZE;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", bool_str[g_bar.enabled]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            g_bar.enabled = false;
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            g_bar.enabled = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_TEXT_FONT)) {
        int length = strlen(message);
        if (length <= 0) {
            fprintf(rsp, "%s\n", g_bar.t_font_prop);
        } else {
            g_bar.t_font_prop = string_copy(message);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_ICON_FONT)) {
        int length = strlen(message);
        if (length <= 0) {
            fprintf(rsp, "%s\n", g_bar.i_font_prop);
        } else {
            g_bar.i_font_prop = string_copy(message);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_BACKGROUND)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "0x%x\n", g_bar.background_color.p);
        } else {
            uint32_t color = token_to_uint32t(value);
            if (color) {
                g_bar.background_color = rgba_color_from_hex(color);
                g_bar.background_color_dim = rgba_color_dim(g_bar.background_color);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_FOREGROUND)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "0x%x\n", g_bar.foreground_color.p);
        } else {
            uint32_t color = token_to_uint32t(value);
            if (color) {
                g_bar.foreground_color = rgba_color_from_hex(color);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACE_STRIP)) {
        struct token token = get_token(&message);
        while (token.text && token.length > 0) {
            buf_push(g_bar._space_icon_strip, token_to_string(token));
            token = get_token(&message);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_POWER_STRIP)) {
        struct token token = get_token(&message);
        while (token.text && token.length > 0) {
            buf_push(g_bar._power_icon_strip, token_to_string(token));
            token = get_token(&message);
        }
        if (buf_len(g_bar._power_icon_strip) != 2) {
            daemon_fail(rsp, "value for '%.*s' must contain exactly two symbols separated by whitespace.\n", command.length, command.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACE_ICON)) {
        struct token token = get_token(&message);
        if (!token_is_valid(token)) {
            fprintf(rsp, "%s\n", g_bar._space_icon ? g_bar._space_icon : "");
        } else {
            g_bar._space_icon = token_to_string(token);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_CLOCK_ICON)) {
        struct token token = get_token(&message);
        if (!token_is_valid(token)) {
            fprintf(rsp, "%s\n", g_bar._clock_icon ? g_bar._clock_icon : "");
        } else {
            g_bar._clock_icon = token_to_string(token);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

#undef VIEW_SET_PROPERTY

static void handle_domain_display(FILE *rsp, struct token domain, char *message)
{
    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_DISPLAY_FOCUS)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_COMMON_SEL_PREV)) {
            uint32_t did = display_manager_prev_display_id(display_manager_active_display_id());
            if (did) {
                display_manager_focus_display(did);
            } else {
                daemon_fail(rsp, "could not locate the previous display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_NEXT)) {
            uint32_t did = display_manager_next_display_id(display_manager_active_display_id());
            if (did) {
                display_manager_focus_display(did);
            } else {
                daemon_fail(rsp, "could not locate the next display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_FIRST)) {
            uint32_t did = display_manager_first_display_id();
            if (did) {
                display_manager_focus_display(did);
            } else {
                daemon_fail(rsp, "could not locate the first display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_LAST)) {
            uint32_t did = display_manager_last_display_id();
            if (did) {
                display_manager_focus_display(did);
            } else {
                daemon_fail(rsp, "could not locate the last display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_RECENT)) {
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

static uint8_t parse_value_type(char *type)
{
    if (string_equals(type, "abs")) {
        return TYPE_ABS;
    } else if (string_equals(type, "rel")) {
        return TYPE_REL;
    } else {
        return 0;
    }
}

static void handle_domain_space(FILE *rsp, struct token domain, char *message)
{
    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_SPACE_FOCUS)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_COMMON_SEL_PREV)) {
            uint64_t sid = space_manager_prev_space(space_manager_active_space());
            if (sid) {
                space_manager_focus_space(sid);
            } else {
                daemon_fail(rsp, "could not locate the previous space.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_NEXT)) {
            uint64_t sid = space_manager_next_space(space_manager_active_space());
            if (sid) {
                space_manager_focus_space(sid);
            } else {
                daemon_fail(rsp, "could not locate the next space.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_FIRST)) {
            uint64_t sid = space_manager_first_space();
            if (sid) {
                space_manager_focus_space(sid);
            } else {
                daemon_fail(rsp, "could not locate the first space.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_LAST)) {
            uint64_t sid = space_manager_last_space();
            if (sid) {
                space_manager_focus_space(sid);
            } else {
                daemon_fail(rsp, "could not locate the last space.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_RECENT)) {
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
        if (token_equals(value, ARGUMENT_COMMON_SEL_PREV)) {
            uint64_t cur_sid = space_manager_active_space();
            uint64_t pre_sid = space_manager_prev_space(cur_sid);
            if (cur_sid && pre_sid) {
                space_manager_move_space_after_space(pre_sid, cur_sid, false);
            } else {
                daemon_fail(rsp, "could not swap places with the previous space.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_NEXT)) {
            uint64_t src_sid = space_manager_active_space();
            uint64_t dst_sid = space_manager_next_space(src_sid);
            if (src_sid && dst_sid) {
                space_manager_move_space_after_space(src_sid, dst_sid, true);
            } else {
                daemon_fail(rsp, "could not swap places with the next space.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_DISPLAY)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_COMMON_SEL_PREV)) {
            uint32_t did = display_manager_prev_display_id(display_manager_active_display_id());
            if (did) {
                space_manager_move_space_to_display(&g_space_manager, did);
            } else {
                daemon_fail(rsp, "could not locate the previous display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_NEXT)) {
            uint32_t did = display_manager_next_display_id(display_manager_active_display_id());
            if (did) {
                space_manager_move_space_to_display(&g_space_manager, did);
            } else {
                daemon_fail(rsp, "could not locate the next display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_FIRST)) {
            uint32_t did = display_manager_first_display_id();
            if (did) {
                space_manager_move_space_to_display(&g_space_manager, did);
            } else {
                daemon_fail(rsp, "could not locate the first display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_LAST)) {
            uint32_t did = display_manager_last_display_id();
            if (did) {
                space_manager_move_space_to_display(&g_space_manager, did);
            } else {
                daemon_fail(rsp, "could not locate the last display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_RECENT)) {
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
        char type[MAXLEN];
        int t, b, l, r;
        if ((sscanf(value.text, ARGUMENT_SPACE_PADDING, type, &t, &b, &l, &r) == 5)) {
            space_manager_set_padding_for_space(&g_space_manager, space_manager_active_space(), parse_value_type(type), t, b, l, r);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_GAP)) {
        struct token value = get_token(&message);
        char type[MAXLEN];
        int gap;
        if ((sscanf(value.text, ARGUMENT_SPACE_GAP, type, &gap) == 2)) {
            space_manager_set_gap_for_space(&g_space_manager, space_manager_active_space(), parse_value_type(type), gap);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_TOGGLE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_TGL_PADDING)) {
            space_manager_toggle_padding_for_space(&g_space_manager, space_manager_active_space());
        } else if (token_equals(value, ARGUMENT_SPACE_TGL_GAP)) {
            space_manager_toggle_gap_for_space(&g_space_manager, space_manager_active_space());
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_LAYOUT)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_LAYOUT_BSP)) {
            space_manager_set_layout_for_space(&g_space_manager, space_manager_active_space(), VIEW_BSP);
        } else if (token_equals(value, ARGUMENT_SPACE_LAYOUT_FLT)) {
            space_manager_set_layout_for_space(&g_space_manager, space_manager_active_space(), VIEW_FLOAT);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static uint8_t parse_resize_handle(char *handle)
{
    if (string_equals(handle, "top")) {
        return HANDLE_TOP;
    } else if (string_equals(handle, "bottom")) {
        return HANDLE_BOTTOM;
    } else if (string_equals(handle, "left")) {
        return HANDLE_LEFT;
    } else if (string_equals(handle, "right")) {
        return HANDLE_RIGHT;
    } else if (string_equals(handle, "top_left")) {
        return HANDLE_TOP | HANDLE_LEFT;
    } else if (string_equals(handle, "top_right")) {
        return HANDLE_TOP | HANDLE_RIGHT;
    } else if (string_equals(handle, "bottom_left")) {
        return HANDLE_BOTTOM | HANDLE_LEFT;
    } else if (string_equals(handle, "bottom_right")) {
        return HANDLE_BOTTOM | HANDLE_RIGHT;
    } else if (string_equals(handle, "abs")) {
        return HANDLE_ABS;
    } else {
        return 0;
    }
}

static void handle_domain_window(FILE *rsp, struct token domain, char *message)
{
    struct token command;
    struct token selector = get_token(&message);

    struct window *window;
    uint32_t sel_wid = token_to_int(selector);

    if (sel_wid != 0) {
        window = window_manager_find_window(&g_window_manager, sel_wid);
        command = get_token(&message);
    } else {
        window = window_manager_focused_window(&g_window_manager);;
        command = selector;
    }

    if (token_equals(command, COMMAND_WINDOW_FOCUS)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_DIR_NORTH)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, window, DIR_NORTH);
                if (closest_window) {
                    window_manager_focus_window_with_raise(closest_window->id);
                } else {
                    daemon_fail(rsp, "could not locate a northward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_EAST)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, window, DIR_EAST);
                if (closest_window) {
                    window_manager_focus_window_with_raise(closest_window->id);
                } else {
                    daemon_fail(rsp, "could not locate a eastward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_SOUTH)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, window, DIR_SOUTH);
                if (closest_window) {
                    window_manager_focus_window_with_raise(closest_window->id);
                } else {
                    daemon_fail(rsp, "could not locate a southward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_WEST)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, window, DIR_WEST);
                if (closest_window) {
                    window_manager_focus_window_with_raise(closest_window->id);
                } else {
                    daemon_fail(rsp, "could not locate a westward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_SEL_MOUSE)) {
            struct window *mouse_window = window_manager_find_window_below_cursor(&g_window_manager);
            if (mouse_window) {
                window_manager_focus_window_with_raise(mouse_window->id);
            } else {
                daemon_fail(rsp, "could not locate a window below the cursor.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_PREV)) {
            if (window) {
                struct window *prev_window = window_manager_find_prev_managed_window(&g_space_manager, &g_window_manager, window);
                if (prev_window) {
                    window_manager_focus_window_with_raise(prev_window->id);
                } else {
                    daemon_fail(rsp, "could not locate the prev managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_NEXT)) {
            if (window) {
                struct window *next_window = window_manager_find_next_managed_window(&g_space_manager, &g_window_manager, window);
                if (next_window) {
                    window_manager_focus_window_with_raise(next_window->id);
                } else {
                    daemon_fail(rsp, "could not locate the next managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_FIRST)) {
            if (window) {
                struct window *first_window = window_manager_find_first_managed_window(&g_space_manager, &g_window_manager);
                if (first_window) {
                    window_manager_focus_window_with_raise(first_window->id);
                } else {
                    daemon_fail(rsp, "could not locate the first managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_LAST)) {
            if (window) {
                struct window *last_window = window_manager_find_last_managed_window(&g_space_manager, &g_window_manager);
                if (last_window) {
                    window_manager_focus_window_with_raise(last_window->id);
                } else {
                    daemon_fail(rsp, "could not locate the last managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_RECENT)) {
            struct window *recent_window = window_manager_find_recent_managed_window(&g_space_manager, &g_window_manager);
            if (recent_window) {
                window_manager_focus_window_with_raise(recent_window->id);
            } else {
                daemon_fail(rsp, "could not locate the most recently focused window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_SWAP)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_DIR_NORTH)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_NORTH);
                if (closest_window) {
                    window_manager_swap_window(&g_space_manager, &g_window_manager, window, closest_window);
                } else {
                    daemon_fail(rsp, "could not locate a northward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_EAST)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_EAST);
                if (closest_window) {
                    window_manager_swap_window(&g_space_manager, &g_window_manager, window, closest_window);
                } else {
                    daemon_fail(rsp, "could not locate a eastward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_SOUTH)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_SOUTH);
                if (closest_window) {
                    window_manager_swap_window(&g_space_manager, &g_window_manager, window, closest_window);
                } else {
                    daemon_fail(rsp, "could not locate a southward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_WEST)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_WEST);
                if (closest_window) {
                    window_manager_swap_window(&g_space_manager, &g_window_manager, window, closest_window);
                } else {
                    daemon_fail(rsp, "could not locate a westward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_SEL_MOUSE)) {
            if (window) {
                struct window *mouse_window = window_manager_find_window_below_cursor(&g_window_manager);
                if (mouse_window) {
                    window_manager_swap_window(&g_space_manager, &g_window_manager, window, mouse_window);
                } else {
                    daemon_fail(rsp, "could not locate a window below the cursor.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_PREV)) {
            if (window) {
                struct window *prev_window = window_manager_find_prev_managed_window(&g_space_manager, &g_window_manager, window);
                if (prev_window) {
                    window_manager_swap_window(&g_space_manager, &g_window_manager, window, prev_window);
                } else {
                    daemon_fail(rsp, "could not locate the prev managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_NEXT)) {
            if (window) {
                struct window *next_window = window_manager_find_next_managed_window(&g_space_manager, &g_window_manager, window);
                if (next_window) {
                    window_manager_swap_window(&g_space_manager, &g_window_manager, window, next_window);
                } else {
                    daemon_fail(rsp, "could not locate the next managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_FIRST)) {
            if (window) {
                struct window *first_window = window_manager_find_first_managed_window(&g_space_manager, &g_window_manager);
                if (first_window) {
                    window_manager_swap_window(&g_space_manager, &g_window_manager, window, first_window);
                } else {
                    daemon_fail(rsp, "could not locate the first managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_LAST)) {
            if (window) {
                struct window *last_window = window_manager_find_last_managed_window(&g_space_manager, &g_window_manager);
                if (last_window) {
                    window_manager_swap_window(&g_space_manager, &g_window_manager, window, last_window);
                } else {
                    daemon_fail(rsp, "could not locate the last managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_RECENT)) {
            if (window) {
                struct window *recent_window = window_manager_find_recent_managed_window(&g_space_manager, &g_window_manager);
                if (recent_window) {
                    window_manager_swap_window(&g_space_manager, &g_window_manager, window, recent_window);
                } else {
                    daemon_fail(rsp, "could not locate the most recently focused window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_WARP)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_DIR_NORTH)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_NORTH);
                if (closest_window) {
                    window_manager_warp_window(&g_space_manager, window, closest_window);
                } else {
                    daemon_fail(rsp, "could not locate a northward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_EAST)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_EAST);
                if (closest_window) {
                    window_manager_warp_window(&g_space_manager, window, closest_window);
                } else {
                    daemon_fail(rsp, "could not locate a eastward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_SOUTH)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_SOUTH);
                if (closest_window) {
                    window_manager_warp_window(&g_space_manager, window, closest_window);
                } else {
                    daemon_fail(rsp, "could not locate a southward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_WEST)) {
            if (window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, window, DIR_WEST);
                if (closest_window) {
                    window_manager_warp_window(&g_space_manager, window, closest_window);
                } else {
                    daemon_fail(rsp, "could not locate a westward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_SEL_MOUSE)) {
            if (window) {
                struct window *mouse_window = window_manager_find_window_below_cursor(&g_window_manager);
                if (mouse_window) {
                    window_manager_warp_window(&g_space_manager, window, mouse_window);
                } else {
                    daemon_fail(rsp, "could not locate a window below the cursor.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_PREV)) {
            if (window) {
                struct window *prev_window = window_manager_find_prev_managed_window(&g_space_manager, &g_window_manager, window);
                if (prev_window) {
                    window_manager_warp_window(&g_space_manager, window, prev_window);
                } else {
                    daemon_fail(rsp, "could not locate the prev managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_NEXT)) {
            if (window) {
                struct window *next_window = window_manager_find_next_managed_window(&g_space_manager, &g_window_manager, window);
                if (next_window) {
                    window_manager_warp_window(&g_space_manager, window, next_window);
                } else {
                    daemon_fail(rsp, "could not locate the next managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_FIRST)) {
            if (window) {
                struct window *first_window = window_manager_find_first_managed_window(&g_space_manager, &g_window_manager);
                if (first_window) {
                    window_manager_warp_window(&g_space_manager, window, first_window);
                } else {
                    daemon_fail(rsp, "could not locate the first managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_LAST)) {
            if (window) {
                struct window *last_window = window_manager_find_last_managed_window(&g_space_manager, &g_window_manager);
                if (last_window) {
                    window_manager_warp_window(&g_space_manager, window, last_window);
                } else {
                    daemon_fail(rsp, "could not locate the last managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_RECENT)) {
            if (window) {
                struct window *recent_window = window_manager_find_recent_managed_window(&g_space_manager, &g_window_manager);
                if (recent_window) {
                    window_manager_warp_window(&g_space_manager, window, recent_window);
                } else {
                    daemon_fail(rsp, "could not locate the most recently focused window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_INSERT)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_DIR_NORTH)) {
            if (window) {
                window_manager_set_window_insertion(&g_space_manager, &g_window_manager, window, DIR_NORTH);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_EAST)) {
            if (window) {
                window_manager_set_window_insertion(&g_space_manager, &g_window_manager, window, DIR_EAST);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_SOUTH)) {
            if (window) {
                window_manager_set_window_insertion(&g_space_manager, &g_window_manager, window, DIR_SOUTH);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_DIR_WEST)) {
            if (window) {
                window_manager_set_window_insertion(&g_space_manager, &g_window_manager, window, DIR_WEST);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_GRID)) {
        struct token value = get_token(&message);
        unsigned r, c, x, y, w, h;
        if ((sscanf(value.text, ARGUMENT_WINDOW_GRID, &r, &c, &x, &y, &w, &h) == 6)) {
            if (window) {
                window_manager_apply_grid(&g_space_manager, &g_window_manager, window, r, c, x, y, w, h);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_MOVE)) {
        struct token value = get_token(&message);
        char type[MAXLEN];
        float x, y;
        if ((sscanf(value.text, ARGUMENT_WINDOW_MOVE, type, &x, &y) == 3)) {
            if (window) {
                window_manager_move_window_relative(&g_window_manager, window, parse_value_type(type), x, y);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_RESIZE)) {
        struct token value = get_token(&message);
        char handle[MAXLEN];
        float w, h;
        if ((sscanf(value.text, ARGUMENT_WINDOW_RESIZE, handle, &w, &h) == 3)) {
            if (window) {
                window_manager_resize_window_relative(&g_window_manager, window, parse_resize_handle(handle), w, h);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_TOGGLE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_FLOAT)) {
            if (window) {
                window_manager_toggle_window_float(&g_space_manager, &g_window_manager, window);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_STICKY)) {
            if (window) {
                window_manager_toggle_window_sticky(&g_space_manager, &g_window_manager, window);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_SPLIT)) {
            if (window) {
                space_manager_toggle_window_split(&g_space_manager, window);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_PARENT)) {
            if (window) {
                window_manager_toggle_window_parent(&g_space_manager, &g_window_manager, window);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_FULLSC)) {
            if (window) {
                window_manager_toggle_window_fullscreen(&g_space_manager, &g_window_manager, window);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_NATIVE)) {
            if (window) {
                window_manager_toggle_window_native_fullscreen(&g_space_manager, &g_window_manager, window);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_BORDER)) {
            if (window) {
                window_manager_toggle_window_border(&g_window_manager, window);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_DISPLAY)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_COMMON_SEL_PREV)) {
            uint32_t did = display_manager_prev_display_id(display_manager_active_display_id());
            if (did) {
                if (window) {
                    window_manager_send_window_to_display(&g_space_manager, &g_window_manager, window, did);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the previous display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_NEXT)) {
            uint32_t did = display_manager_next_display_id(display_manager_active_display_id());
            if (did) {
                if (window) {
                    window_manager_send_window_to_display(&g_space_manager, &g_window_manager, window, did);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the next display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_FIRST)) {
            uint32_t did = display_manager_first_display_id();
            if (did) {
                if (window) {
                    window_manager_send_window_to_display(&g_space_manager, &g_window_manager, window, did);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the first display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_LAST)) {
            uint32_t did = display_manager_last_display_id();
            if (did) {
                if (window) {
                    window_manager_send_window_to_display(&g_space_manager, &g_window_manager, window, did);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the last display.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_RECENT)) {
            if (window) {
                window_manager_send_window_to_display(&g_space_manager, &g_window_manager, window, g_display_manager.last_display_id);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_is_valid(value)) {
            int arrangement_index = token_to_int(value);
            uint32_t did = display_manager_arrangement_display_id(arrangement_index);
            if (did) {
                if (window) {
                    window_manager_send_window_to_display(&g_space_manager, &g_window_manager, window, did);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate display with arrangement index '%d'.\n", arrangement_index);
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_SPACE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_COMMON_SEL_PREV)) {
            uint64_t sid = space_manager_prev_space(space_manager_active_space());
            if (sid) {
                if (window) {
                    window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, sid);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the previous space.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_NEXT)) {
            uint64_t sid = space_manager_next_space(space_manager_active_space());
            if (sid) {
                if (window) {
                    window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, sid);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the next space.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_FIRST)) {
            uint64_t sid = space_manager_first_space();
            if (sid) {
                if (window) {
                    window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, sid);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the first space.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_LAST)) {
            uint64_t sid = space_manager_last_space();
            if (sid) {
                if (window) {
                    window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, sid);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the last space.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_RECENT)) {
            if (window) {
                window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, g_space_manager.last_space_id);
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_is_valid(value)) {
            int mci = token_to_int(value);
            uint64_t sid = space_manager_mission_control_space(mci);
            if (sid) {
                if (window) {
                    window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, sid);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
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

static void handle_domain_query(FILE *rsp, struct token domain, char *message)
{
    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_QUERY_DISPLAYS)) {
        struct token option = get_token(&message);
        if (token_equals(option, ARGUMENT_QUERY_DISPLAY)) {
            struct token value = get_token(&message);
            if (token_is_valid(value)) {
                int arrangement_index = token_to_int(value);
                uint32_t did = display_manager_arrangement_display_id(arrangement_index);
                if (did) {
                    display_serialize(rsp, did);
                    fprintf(rsp, "\n");
                } else {
                    daemon_fail(rsp, "could not locate display with arrangement index '%d'.\n", arrangement_index);
                }
            } else {
                display_serialize(rsp, display_manager_active_display_id());
                fprintf(rsp, "\n");
            }
        } else if (token_equals(option, ARGUMENT_QUERY_SPACE)) {
            struct token value = get_token(&message);
            if (token_is_valid(value)) {
                int mci = token_to_int(value);
                uint64_t sid = space_manager_mission_control_space(mci);
                if (sid) {
                    display_serialize(rsp, space_display_id(sid));
                    fprintf(rsp, "\n");
                } else {
                    daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", mci);
                }
            } else {
                display_serialize(rsp, space_display_id(space_manager_active_space()));
                fprintf(rsp, "\n");
            }
        } else if (token_equals(option, ARGUMENT_QUERY_WINDOW)) {
            struct token value = get_token(&message);
            if (token_is_valid(value)) {
                struct window *window = window_manager_find_window(&g_window_manager, token_to_int(value));
                if (window) {
                    display_serialize(rsp, window_display_id(window));
                    fprintf(rsp, "\n");
                } else {
                    daemon_fail(rsp, "could not find window to retrieve display details\n");
                }
            } else {
                struct window *window = window_manager_focused_window(&g_window_manager);
                if (window) {
                    display_serialize(rsp, window_display_id(window));
                    fprintf(rsp, "\n");
                } else {
                    daemon_fail(rsp, "could not find window to retrieve display details\n");
                }
            }
        } else if (token_is_valid(option)) {
            daemon_fail(rsp, "unknown option '%.*s' given to command '%.*s' for domain '%.*s'\n", option.length, option.text, command.length, command.text, domain.length, domain.text);
        } else {
            display_manager_query_displays(rsp);
        }
    } else if (token_equals(command, COMMAND_QUERY_SPACES)) {
        struct token option = get_token(&message);
        if (token_equals(option, ARGUMENT_QUERY_DISPLAY)) {
            struct token value = get_token(&message);
            if (token_is_valid(value)) {
                int arrangement_index = token_to_int(value);
                uint32_t did = display_manager_arrangement_display_id(arrangement_index);
                if (did) {
                    if (!space_manager_query_spaces_for_display(rsp, did)) {
                        daemon_fail(rsp, "could not retrieve spaces for display\n");
                    }
                } else {
                    daemon_fail(rsp, "could not locate display with arrangement index '%d'.\n", arrangement_index);
                }
            } else {
                if (!space_manager_query_spaces_for_display(rsp, display_manager_active_display_id())) {
                    daemon_fail(rsp, "could not retrieve spaces for display\n");
                }
            }
        } else if (token_equals(option, ARGUMENT_QUERY_SPACE)) {
            struct token value = get_token(&message);
            if (token_is_valid(value)) {
                int mci = token_to_int(value);
                uint64_t sid = space_manager_mission_control_space(mci);
                if (sid) {
                    struct view *view = space_manager_query_view(&g_space_manager, sid);
                    if (view) {
                        view_serialize(rsp, view);
                        fprintf(rsp, "\n");
                    } else {
                        daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", mci);
                    }
                } else {
                    daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", mci);
                }
            } else {
                if (!space_manager_query_active_space(rsp)) {
                    daemon_fail(rsp, "could not retrieve active space\n");
                }
            }
        } else if (token_equals(option, ARGUMENT_QUERY_WINDOW)) {
            struct token value = get_token(&message);
            if (token_is_valid(value)) {
                struct window *window = window_manager_find_window(&g_window_manager, token_to_int(value));
                if (window) {
                    space_manager_query_spaces_for_window(rsp, window);
                } else {
                    daemon_fail(rsp, "could not find window to retrieve space details\n");
                }
            } else {
                struct window *window = window_manager_focused_window(&g_window_manager);
                if (window) {
                    space_manager_query_spaces_for_window(rsp, window);
                } else {
                    daemon_fail(rsp, "could not find window to retrieve space details\n");
                }
            }
        } else if (token_is_valid(option)) {
            daemon_fail(rsp, "unknown option '%.*s' given to command '%.*s' for domain '%.*s'\n", option.length, option.text, command.length, command.text, domain.length, domain.text);
        } else {
            if (!space_manager_query_spaces_for_displays(rsp)) {
                daemon_fail(rsp, "could not retrieve spaces for displays\n");
            }
        }
    } else if (token_equals(command, COMMAND_QUERY_WINDOWS)) {
        struct token option = get_token(&message);
        if (token_equals(option, ARGUMENT_QUERY_DISPLAY)) {
            struct token value = get_token(&message);
            if (token_is_valid(value)) {
                int arrangement_index = token_to_int(value);
                uint32_t did = display_manager_arrangement_display_id(arrangement_index);
                if (did) {
                    window_manager_query_windows_for_display(rsp, did);
                } else {
                    daemon_fail(rsp, "could not locate display with arrangement index '%d'.\n", arrangement_index);
                }
            } else {
                window_manager_query_windows_for_display(rsp, display_manager_active_display_id());
            }
        } else if (token_equals(option, ARGUMENT_QUERY_SPACE)) {
            struct token value = get_token(&message);
            if (token_is_valid(value)) {
                int mci = token_to_int(value);
                uint64_t sid = space_manager_mission_control_space(mci);
                if (sid) {
                    window_manager_query_windows_for_space(rsp, sid);
                } else {
                    daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", mci);
                }
            } else {
                window_manager_query_windows_for_space(rsp, space_manager_active_space());
            }
        } else if (token_equals(option, ARGUMENT_QUERY_WINDOW)) {
            struct window *window = window_manager_focused_window(&g_window_manager);
            if (window) {
                window_serialize(window, rsp);
                fprintf(rsp, "\n");
            } else {
                daemon_fail(rsp, "could not retrieve window details\n");
            }
        } else if (token_is_valid(option)) {
            daemon_fail(rsp, "unknown option '%.*s' given to command '%.*s' for domain '%.*s'\n", option.length, option.text, command.length, command.text, domain.length, domain.text);
        } else {
            window_manager_query_windows_for_displays(rsp);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static void handle_domain_rule(FILE *rsp, struct token domain, char *message)
{
    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_RULE_ADD)) {
        struct rule *rule = rule_create();
        struct token token = get_token(&message);
        while (token.text && token.length > 0) {
            char *key = strtok(token.text, "=");
            char *value = strtok(NULL, "=");
            if (!key || !value) {
                daemon_fail(rsp, "invalid key-value pair '%s'\n", token.text);
                return;
            }

            if (string_equals(key, ARGUMENT_RULE_KEY_APP)) {
                rule->app_regex_valid = regcomp(&rule->app_regex, value, REG_EXTENDED) == 0;
                if (!rule->app_regex_valid) daemon_fail(rsp, "could not compile regex for pattern '%s'\n", value);
            } else if (string_equals(key, ARGUMENT_RULE_KEY_TITLE)) {
                rule->title_regex_valid = regcomp(&rule->title_regex, value, REG_EXTENDED) == 0;
                if (!rule->title_regex_valid) daemon_fail(rsp, "could not compile regex for pattern '%s'\n", value);
            } else if (string_equals(key, ARGUMENT_RULE_KEY_DISPLAY)) {
                if (value[0] == ARGUMENT_RULE_VALUE_SPACE) {
                    ++value;
                    rule->follow_space = true;
                }
                sscanf(value, "%d", &rule->display);
            } else if (string_equals(key, ARGUMENT_RULE_KEY_SPACE)) {
                if (value[0] == ARGUMENT_RULE_VALUE_SPACE) {
                    ++value;
                    rule->follow_space = true;
                }
                sscanf(value, "%d", &rule->space);
            } else if (string_equals(key, ARGUMENT_RULE_KEY_ALPHA)) {
                sscanf(value, "%f", &rule->alpha);
            } else if (string_equals(key, ARGUMENT_RULE_KEY_MANAGE)) {
                if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    rule->manage = RULE_PROP_ON;
                } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    rule->manage = RULE_PROP_OFF;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_STICKY)) {
                if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    rule->sticky = RULE_PROP_ON;
                } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    rule->sticky = RULE_PROP_OFF;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_BORDER)) {
                if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    rule->border = RULE_PROP_ON;
                } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    rule->border = RULE_PROP_OFF;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_FULLSCR)) {
                if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    rule->fullscreen = RULE_PROP_ON;
                } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    rule->fullscreen = RULE_PROP_OFF;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_GRID)) {
                if ((sscanf(value, ARGUMENT_RULE_VALUE_GRID,
                            &rule->grid[0], &rule->grid[1],
                            &rule->grid[2], &rule->grid[3],
                            &rule->grid[4], &rule->grid[5]) != 6)) {
                    memset(rule->grid, 0, sizeof(rule->grid));
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_LABEL)) {
                rule->label = string_copy(value);
            }

            token = get_token(&message);
        }

        if (rule_is_valid(rule)) {
            rule_add(rule);
        } else {
            rule_destroy(rule);
            daemon_fail(rsp, "a rule must contain at least one of the following key-value pairs: 'app', 'title'\n");
        }
    } else if (token_equals(command, COMMAND_RULE_REM)) {
        struct token token = get_token(&message);
        if (token_is_valid(token)) {
            char *label = token_to_string(token);
            bool did_remove_rule = rule_remove(label);
            if (!did_remove_rule) daemon_fail(rsp, "rule with label '%s' not found.\n", label);
            free(label);
        } else {
            daemon_fail(rsp, "invalid label specified.\n");
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static void handle_domain_signal(FILE *rsp, struct token domain, char *message)
{
    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_SIGNAL_ADD)) {
        enum event_type signal_type = EVENT_TYPE_UNKNOWN;
        char *signal_action = NULL;
        char *signal_label  = NULL;

        struct token token = get_token(&message);
        while (token.text && token.length > 0) {
            char *key = strtok(token.text, "=");
            char *value = strtok(NULL, "=");
            if (!key || !value) {
                daemon_fail(rsp, "invalid key-value pair '%s'\n", token.text);
                return;
            }

            if (string_equals(key, ARGUMENT_SIGNAL_KEY_EVENT)) {
                signal_type = event_type_from_string(value);
                if (signal_type == EVENT_TYPE_UNKNOWN) {
                    daemon_fail(rsp, "unknown event-type '%s'\n", value);
                }
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_ACTION)) {
                signal_action = string_copy(value);
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_LABEL)) {
                signal_label = string_copy(value);
            }

            token = get_token(&message);
        }

        if (signal_type > EVENT_TYPE_UNKNOWN && signal_type < EVENT_TYPE_COUNT && signal_action) {
            event_signal_add(signal_type, signal_action, signal_label);
        } else {
            daemon_fail(rsp, "signal was not added.\n");
        }
    } else if (token_equals(command, COMMAND_SIGNAL_REM)) {
        struct token token = get_token(&message);
        if (token_is_valid(token)) {
            char *label = token_to_string(token);
            bool did_remove_signal = event_signal_remove(label);
            if (!did_remove_signal) daemon_fail(rsp, "signal with label '%s' not found.\n", label);
            free(label);
        } else {
            daemon_fail(rsp, "invalid label specified.\n");
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
    } else if (token_equals(domain, DOMAIN_QUERY)) {
        handle_domain_query(rsp, domain, message);
    } else if (token_equals(domain, DOMAIN_RULE)) {
        handle_domain_rule(rsp, domain, message);
    } else if (token_equals(domain, DOMAIN_SIGNAL)) {
        handle_domain_signal(rsp, domain, message);
    } else {
        daemon_fail(rsp, "unknown domain '%.*s'\n", domain.length, domain.text);
    }
}

static SOCKET_DAEMON_HANDLER(message_handler)
{
    struct event *event;
    volatile int status = EVENT_QUEUED;

    FILE *rsp = fdopen(sockfd, "w");
    if (!rsp) goto fderr;

    event_create_p2(event, DAEMON_MESSAGE, message, length, rsp);
    event->status = &status;
    event_loop_post(&g_event_loop, event);
    while (status == EVENT_QUEUED);

    if (status == EVENT_IGNORED) {
        debug("yabai: event_loop is not running! ignoring event..\n");
        daemon_fail(rsp, "event_loop is not running! ignoring event..\n");
    }

    fflush(rsp);
    fclose(rsp);

fderr:
    socket_close(sockfd);
    free(message);
}

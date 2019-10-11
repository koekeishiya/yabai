#include "message.h"

extern struct signal *g_signal_event[EVENT_TYPE_COUNT];
extern struct event_loop g_event_loop;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;
extern struct mouse_state g_mouse_state;
extern struct bar g_bar;

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
#define COMMAND_CONFIG_OPACITY_DURATION      "window_opacity_duration"
#define COMMAND_CONFIG_SHADOW                "window_shadow"
#define COMMAND_CONFIG_BORDER                "window_border"
#define COMMAND_CONFIG_BORDER_WIDTH          "window_border_width"
#define COMMAND_CONFIG_BORDER_RADIUS         "window_border_radius"
#define COMMAND_CONFIG_BORDER_PLACEMENT      "window_border_placement"
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
#define ARGUMENT_CONFIG_BORDER_PLACEMENT_EXT "exterior"
#define ARGUMENT_CONFIG_BORDER_PLACEMENT_INT "interior"
#define ARGUMENT_CONFIG_BORDER_PLACEMENT_IS  "inset"
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
#define ARGUMENT_SPACE_TGL_MC       "mission-control"
#define ARGUMENT_SPACE_TGL_SD       "show-desktop"
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
#define COMMAND_WINDOW_CLOSE   "--close"
#define COMMAND_WINDOW_TOGGLE  "--toggle"
#define COMMAND_WINDOW_DISPLAY "--display"
#define COMMAND_WINDOW_SPACE   "--space"

#define ARGUMENT_WINDOW_DIR_NORTH     "north"
#define ARGUMENT_WINDOW_DIR_EAST      "east"
#define ARGUMENT_WINDOW_DIR_SOUTH     "south"
#define ARGUMENT_WINDOW_DIR_WEST      "west"
#define ARGUMENT_WINDOW_SEL_MOUSE     "mouse"
#define ARGUMENT_WINDOW_SEL_LARGEST   "largest"
#define ARGUMENT_WINDOW_SEL_SMALLEST  "smallest"
#define ARGUMENT_WINDOW_GRID          "%d:%d:%d:%d:%d:%d"
#define ARGUMENT_WINDOW_MOVE          "%255[^:]:%f:%f"
#define ARGUMENT_WINDOW_RESIZE        "%255[^:]:%f:%f"
#define ARGUMENT_WINDOW_TOGGLE_ON_TOP "topmost"
#define ARGUMENT_WINDOW_TOGGLE_FLOAT  "float"
#define ARGUMENT_WINDOW_TOGGLE_STICKY "sticky"
#define ARGUMENT_WINDOW_TOGGLE_SHADOW "shadow"
#define ARGUMENT_WINDOW_TOGGLE_SPLIT  "split"
#define ARGUMENT_WINDOW_TOGGLE_PARENT "zoom-parent"
#define ARGUMENT_WINDOW_TOGGLE_FULLSC "zoom-fullscreen"
#define ARGUMENT_WINDOW_TOGGLE_NATIVE "native-fullscreen"
#define ARGUMENT_WINDOW_TOGGLE_BORDER "border"
#define ARGUMENT_WINDOW_TOGGLE_EXPOSE "expose"
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
#define ARGUMENT_RULE_KEY_ON_TOP  "topmost"
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

#define ARGUMENT_SIGNAL_KEY_APP      "app"
#define ARGUMENT_SIGNAL_KEY_TITLE    "title"
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

static bool token_to_int(struct token token, int *value)
{
    int result = 0;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    bool success = sscanf(buffer, "%d", &result) == 1;
    *value = result;
    return success;
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
    if (!rsp) return;

    fprintf(rsp, FAILURE_MESSAGE);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(rsp, fmt, ap);
    va_end(ap);
}

#define VIEW_SET_PROPERTY(p) \
                    int p_val = 0; \
                    if (token_to_int(value, &p_val)) { \
                    view->custom_##p = true; \
                    view->p = p_val; \
                    view_update(view); \
                    view_flush(view); \
                    }

static void handle_domain_config(FILE *rsp, struct token domain, char *message)
{
    int sel_mci = 0;
    uint64_t sel_sid = 0;
    bool found_selector = true;

    struct token selector = get_token(&message);
    struct token command  = selector;

    if (token_equals(selector, SELECTOR_CONFIG_SPACE)) {
        struct token value = get_token(&message);
        if (token_to_int(value, &sel_mci) && sel_mci) {
            sel_sid = space_manager_mission_control_space(sel_mci);
        }
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
    } else if (token_equals(command, COMMAND_CONFIG_OPACITY_DURATION)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%f\n", g_window_manager.window_opacity_duration);
        } else {
            g_window_manager.window_opacity_duration = token_to_float(value);
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
            window_manager_set_border_window_enabled(&g_window_manager, false);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            window_manager_set_border_window_enabled(&g_window_manager, true);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_PLACEMENT)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", border_placement_str[g_window_manager.window_border_placement]);
        } else if (token_equals(value, ARGUMENT_CONFIG_BORDER_PLACEMENT_EXT)) {
            g_window_manager.window_border_placement = BORDER_PLACEMENT_EXTERIOR;
        } else if (token_equals(value, ARGUMENT_CONFIG_BORDER_PLACEMENT_INT)) {
            g_window_manager.window_border_placement = BORDER_PLACEMENT_INTERIOR;
        } else if (token_equals(value, ARGUMENT_CONFIG_BORDER_PLACEMENT_IS)) {
            g_window_manager.window_border_placement = BORDER_PLACEMENT_INSET;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_WIDTH)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%d\n", g_window_manager.window_border_width);
        } else {
            int width = 0;
            if (token_to_int(value, &width) && width) {
                window_manager_set_border_window_width(&g_window_manager, width);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_RADIUS)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%.4f\n", g_window_manager.window_border_radius);
        } else {
            float radius = token_to_float(value);
            if (radius == -1.f || (radius >= 0.0f && radius <= 20.0f)) {
                window_manager_set_border_window_radius(&g_window_manager, radius);
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
                int padding = 0;
                if (token_to_int(value, &padding)) {
                    space_manager_set_top_padding_for_all_spaces(&g_space_manager, padding);
                }
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
                int padding = 0;
                if (token_to_int(value, &padding)) {
                    space_manager_set_bottom_padding_for_all_spaces(&g_space_manager, padding);
                }
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
                int padding = 0;
                if (token_to_int(value, &padding)) {
                    space_manager_set_left_padding_for_all_spaces(&g_space_manager, padding);
                }
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
                int padding = 0;
                if (token_to_int(value, &padding)) {
                    space_manager_set_right_padding_for_all_spaces(&g_space_manager, padding);
                }
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
                int gap = 0;
                if (token_to_int(value, &gap)) {
                    space_manager_set_window_gap_for_all_spaces(&g_space_manager, gap);
                }
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
                    if (space_is_user(sel_sid)) {
                        view->layout = VIEW_BSP;
                        view->custom_layout = true;
                        window_manager_check_for_windows_on_space(&g_space_manager, &g_window_manager, sel_sid);
                    } else {
                        daemon_fail(rsp, "cannot set layout for a macOS fullscreen space!\n");
                    }
                } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_FLOAT)) {
                    if (space_is_user(sel_sid)) {
                        view->layout = VIEW_FLOAT;
                        view->custom_layout = true;
                        view_clear(view);
                    } else {
                        daemon_fail(rsp, "cannot set layout for a macOS fullscreen space!\n");
                    }
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
            bar_destroy(&g_bar);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            bar_create(&g_bar);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_TEXT_FONT)) {
        int length = strlen(message);
        if (length <= 0) {
            fprintf(rsp, "%s\n", g_bar.t_font_prop);
        } else {
            bar_set_text_font(&g_bar, string_copy(message));
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_ICON_FONT)) {
        int length = strlen(message);
        if (length <= 0) {
            fprintf(rsp, "%s\n", g_bar.i_font_prop);
        } else {
            bar_set_icon_font(&g_bar, string_copy(message));
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_BACKGROUND)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "0x%x\n", g_bar.background_color.p);
        } else {
            uint32_t color = token_to_uint32t(value);
            if (color) {
                bar_set_background_color(&g_bar, color);
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
                bar_set_foreground_color(&g_bar, color);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACE_STRIP)) {
        char **icon_strip = NULL;
        struct token token = get_token(&message);
        while (token.text && token.length > 0) {
            buf_push(icon_strip, token_to_string(token));
            token = get_token(&message);
        }
        bar_set_space_strip(&g_bar, icon_strip);
    } else if (token_equals(command, COMMAND_CONFIG_BAR_POWER_STRIP)) {
        char **icon_strip = NULL;
        struct token token = get_token(&message);
        while (token.text && token.length > 0) {
            buf_push(icon_strip, token_to_string(token));
            token = get_token(&message);
        }
        bar_set_power_strip(&g_bar, icon_strip);
        if (buf_len(g_bar._power_icon_strip) != 2) {
            daemon_fail(rsp, "value for '%.*s' must contain exactly two symbols separated by whitespace.\n", command.length, command.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACE_ICON)) {
        struct token token = get_token(&message);
        if (!token_is_valid(token)) {
            fprintf(rsp, "%s\n", g_bar._space_icon ? g_bar._space_icon : "");
        } else {
            bar_set_space_icon(&g_bar, token_to_string(token));
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_CLOCK_ICON)) {
        struct token token = get_token(&message);
        if (!token_is_valid(token)) {
            fprintf(rsp, "%s\n", g_bar._clock_icon ? g_bar._clock_icon : "");
        } else {
            bar_set_clock_icon(&g_bar, token_to_string(token));
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

#undef VIEW_SET_PROPERTY

struct selector
{
    struct token token;
    bool did_parse;

    union {
        int dir;
        uint32_t did;
        uint64_t sid;
        struct window *window;
    };
};

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

static struct selector parse_display_selector(FILE *rsp, char **message, uint32_t acting_did)
{
    struct selector result = {
        .token = get_token(message),
        .did_parse = true
    };

    if (token_equals(result.token, ARGUMENT_COMMON_SEL_PREV)) {
        if (acting_did) {
            uint32_t did = display_manager_prev_display_id(acting_did);
            if (did) {
                result.did = did;
            } else {
                daemon_fail(rsp, "could not locate the previous display.\n");
            }
        } else {
            daemon_fail(rsp, "could not locate the selected display.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_NEXT)) {
        if (acting_did) {
            uint32_t did = display_manager_next_display_id(acting_did);
            if (did) {
                result.did = did;
            } else {
                daemon_fail(rsp, "could not locate the next display.\n");
            }
        } else {
            daemon_fail(rsp, "could not locate the selected display.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_FIRST)) {
        uint32_t did = display_manager_first_display_id();
        if (did) {
            result.did = did;
        } else {
            daemon_fail(rsp, "could not locate the first display.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_LAST)) {
        uint32_t did = display_manager_last_display_id();
        if (did) {
            result.did = did;
        } else {
            daemon_fail(rsp, "could not locate the last display.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_RECENT)) {
        result.did = g_display_manager.last_display_id;
    } else if (token_is_valid(result.token)) {
        int arrangement_index = 0;
        if (token_to_int(result.token, &arrangement_index)) {
            if (arrangement_index) {
                uint32_t did = display_manager_arrangement_display_id(arrangement_index);
                if (did) {
                    result.did = did;
                } else {
                    daemon_fail(rsp, "could not locate display with arrangement index '%d'.\n", arrangement_index);
                }
            } else {
                daemon_fail(rsp, "invalid arrangement-index specified '%d'.\n", arrangement_index);
            }
        } else {
            result.did_parse = false;
            daemon_fail(rsp, "value '%.*s' is not a valid option for DISPLAY_SEL\n", result.token.length, result.token.text);
        }
    } else {
        result.did_parse = false;
        daemon_fail(rsp, "value '%.*s' is not a valid option for DISPLAY_SEL\n", result.token.length, result.token.text);
    }

    return result;
}

static struct selector parse_space_selector(FILE *rsp, char **message, uint64_t acting_sid)
{
    struct selector result = {
        .token = get_token(message),
        .did_parse = true
    };

    if (token_equals(result.token, ARGUMENT_COMMON_SEL_PREV)) {
        if (acting_sid) {
            uint64_t sid = space_manager_prev_space(acting_sid);
            if (sid) {
                result.sid = sid;
            } else {
                daemon_fail(rsp, "could not locate the previous space.\n");
            }
        } else {
            daemon_fail(rsp, "could not locate the selected space.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_NEXT)) {
        if (acting_sid) {
            uint64_t sid = space_manager_next_space(acting_sid);
            if (sid) {
                result.sid = sid;
            } else {
                daemon_fail(rsp, "could not locate the next space.\n");
            }
        } else {
            daemon_fail(rsp, "could not locate the selected space.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_FIRST)) {
        uint64_t sid = space_manager_first_space();
        if (sid) {
            result.sid = sid;
        } else {
            daemon_fail(rsp, "could not locate the first space.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_LAST)) {
        uint64_t sid = space_manager_last_space();
        if (sid) {
            result.sid = sid;
        } else {
            daemon_fail(rsp, "could not locate the last space.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_RECENT)) {
        result.sid = g_space_manager.last_space_id;
    } else if (token_is_valid(result.token)) {
        int mci = 0;
        if (token_to_int(result.token, &mci)) {
            if (mci) {
                uint64_t sid = space_manager_mission_control_space(mci);
                if (sid) {
                    result.sid = sid;
                } else {
                    daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", mci);
                }
            } else {
                daemon_fail(rsp, "invalid mission-control index specified '%d'.\n", mci);
            }
        } else {
            result.did_parse = false;
            daemon_fail(rsp, "value '%.*s' is not a valid option for SPACE_SEL\n", result.token.length, result.token.text);
        }
    } else {
        result.did_parse = false;
        daemon_fail(rsp, "value '%.*s' is not a valid option for SPACE_SEL\n", result.token.length, result.token.text);
    }

    return result;
}

static struct selector parse_window_selector(FILE *rsp, char **message, struct window *acting_window)
{
    struct selector result = {
        .token = get_token(message),
        .did_parse = true
    };

    if (token_equals(result.token, ARGUMENT_WINDOW_DIR_NORTH)) {
        if (acting_window) {
            struct window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, acting_window, DIR_NORTH);
            if (closest_window) {
                result.window = closest_window;
            } else {
                daemon_fail(rsp, "could not locate a northward managed window.\n");
            }
        } else {
            daemon_fail(rsp, "could not locate the selected window.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_WINDOW_DIR_EAST)) {
        if (acting_window) {
            struct window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, acting_window, DIR_EAST);
            if (closest_window) {
                result.window = closest_window;
            } else {
                daemon_fail(rsp, "could not locate a eastward managed window.\n");
            }
        } else {
            daemon_fail(rsp, "could not locate the selected window.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_WINDOW_DIR_SOUTH)) {
        if (acting_window) {
            struct window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, acting_window, DIR_SOUTH);
            if (closest_window) {
                result.window = closest_window;
            } else {
                daemon_fail(rsp, "could not locate a southward managed window.\n");
            }
        } else {
            daemon_fail(rsp, "could not locate the selected window.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_WINDOW_DIR_WEST)) {
        if (acting_window) {
            struct window *closest_window = window_manager_find_closest_window_in_direction(&g_window_manager, acting_window, DIR_WEST);
            if (closest_window) {
                result.window = closest_window;
            } else {
                daemon_fail(rsp, "could not locate a westward managed window.\n");
            }
        } else {
            daemon_fail(rsp, "could not locate the selected window.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_WINDOW_SEL_MOUSE)) {
        struct window *mouse_window = window_manager_find_window_below_cursor(&g_window_manager);
        if (mouse_window) {
            result.window = mouse_window;
        } else {
            daemon_fail(rsp, "could not locate a window below the cursor.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_WINDOW_SEL_LARGEST)) {
        struct window *area_window = window_manager_find_largest_managed_window(&g_space_manager, &g_window_manager);
        if (area_window) {
            result.window = area_window;
        } else {
            daemon_fail(rsp, "could not locate window with the largest area.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_WINDOW_SEL_SMALLEST)) {
        struct window *area_window = window_manager_find_smallest_managed_window(&g_space_manager, &g_window_manager);
        if (area_window) {
            result.window = area_window;
        } else {
            daemon_fail(rsp, "could not locate window with the smallest area.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_PREV)) {
        if (acting_window) {
            struct window *prev_window = window_manager_find_prev_managed_window(&g_space_manager, &g_window_manager, acting_window);
            if (prev_window) {
                result.window = prev_window;
            } else {
                daemon_fail(rsp, "could not locate the prev managed window.\n");
            }
        } else {
            daemon_fail(rsp, "could not locate the selected window.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_NEXT)) {
        if (acting_window) {
            struct window *next_window = window_manager_find_next_managed_window(&g_space_manager, &g_window_manager, acting_window);
            if (next_window) {
                result.window = next_window;
            } else {
                daemon_fail(rsp, "could not locate the next managed window.\n");
            }
        } else {
            daemon_fail(rsp, "could not locate the selected window.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_FIRST)) {
        struct window *first_window = window_manager_find_first_managed_window(&g_space_manager, &g_window_manager);
        if (first_window) {
            result.window = first_window;
        } else {
            daemon_fail(rsp, "could not locate the first managed window.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_LAST)) {
        struct window *last_window = window_manager_find_last_managed_window(&g_space_manager, &g_window_manager);
        if (last_window) {
            result.window = last_window;
        } else {
            daemon_fail(rsp, "could not locate the last managed window.\n");
        }
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_RECENT)) {
        struct window *recent_window = window_manager_find_recent_managed_window(&g_space_manager, &g_window_manager);
        if (recent_window) {
            result.window = recent_window;
        } else {
            daemon_fail(rsp, "could not locate the most recently focused window.\n");
        }
    } else if (token_is_valid(result.token)) {
        int wid = 0;
        if (token_to_int(result.token, &wid)) {
            if (wid) {
                struct window *window = window_manager_find_window(&g_window_manager, wid);
                if (window) {
                    result.window = window;
                } else {
                    daemon_fail(rsp, "could not locate window with the specified id '%d'.\n", wid);
                }
            } else {
                daemon_fail(rsp, "invalid window id specified '%d'.\n", wid);
            }
        } else {
            result.did_parse = false;
            daemon_fail(rsp, "value '%.*s' is not a valid option for WINDOW_SEL\n", result.token.length, result.token.text);
        }
    } else {
        result.did_parse = false;
        daemon_fail(rsp, "value '%.*s' is not a valid option for WINDOW_SEL\n", result.token.length, result.token.text);
    }

    return result;
}

static struct selector parse_dir_selector(FILE *rsp, char **message)
{
    struct selector result = {
        .token = get_token(message),
        .did_parse = true
    };

    if (token_equals(result.token, ARGUMENT_WINDOW_DIR_NORTH)) {
        result.dir = DIR_NORTH;
    } else if (token_equals(result.token, ARGUMENT_WINDOW_DIR_EAST)) {
        result.dir = DIR_EAST;
    } else if (token_equals(result.token, ARGUMENT_WINDOW_DIR_SOUTH)) {
        result.dir = DIR_SOUTH;
    } else if (token_equals(result.token, ARGUMENT_WINDOW_DIR_WEST)) {
        result.dir = DIR_WEST;
    } else {
        result.did_parse = false;
        daemon_fail(rsp, "value '%.*s' is not a valid option for DIR_SEL\n", result.token.length, result.token.text);
    }

    return result;
}

static void handle_domain_display(FILE *rsp, struct token domain, char *message)
{
    struct token command;
    uint32_t acting_did;
    struct selector selector = parse_display_selector(NULL, &message, display_manager_active_display_id());

    if (selector.did_parse) {
        acting_did = selector.did;
        command = get_token(&message);
    } else {
        acting_did = display_manager_active_display_id();
        command = selector.token;
    }

    if (!acting_did) {
        daemon_fail(rsp, "could not locate the display to act on!\n");
        return;
    }

    if (token_equals(command, COMMAND_DISPLAY_FOCUS)) {
        struct selector selector = parse_display_selector(rsp, &message, acting_did);
        if (selector.did_parse && selector.did) {
            display_manager_focus_display(selector.did);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static void handle_domain_space(FILE *rsp, struct token domain, char *message)
{
    struct token command;
    uint64_t acting_sid;
    struct selector selector = parse_space_selector(NULL, &message, space_manager_active_space());

    if (selector.did_parse) {
        acting_sid = selector.sid;
        command = get_token(&message);
    } else {
        acting_sid = space_manager_active_space();
        command = selector.token;
    }

    if (!acting_sid) {
        daemon_fail(rsp, "could not locate the space to act on!\n");
        return;
    }

    if (token_equals(command, COMMAND_SPACE_FOCUS)) {
        struct selector selector = parse_space_selector(rsp, &message, acting_sid);
        if (selector.did_parse && selector.sid) {
            space_manager_focus_space(selector.sid);
        }
    } else if (token_equals(command, COMMAND_SPACE_MOVE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_COMMON_SEL_PREV)) {
            uint64_t pre_sid = space_manager_prev_space(acting_sid);
            if (acting_sid && pre_sid) {
                space_manager_move_space_after_space(pre_sid, acting_sid, false);
            } else {
                daemon_fail(rsp, "could not swap places with the previous space.\n");
            }
        } else if (token_equals(value, ARGUMENT_COMMON_SEL_NEXT)) {
            uint64_t dst_sid = space_manager_next_space(acting_sid);
            if (acting_sid && dst_sid) {
                space_manager_move_space_after_space(acting_sid, dst_sid, acting_sid == space_manager_active_space());
            } else {
                daemon_fail(rsp, "could not swap places with the next space.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_DISPLAY)) {
        struct selector selector = parse_display_selector(rsp, &message, display_manager_active_display_id());
        if (selector.did_parse && selector.did) {
            enum space_op_error result = space_manager_move_space_to_display(&g_space_manager, acting_sid, selector.did);
            if (result == SPACE_OP_ERROR_MISSING_SRC) {
                daemon_fail(rsp, "could not locate the space to act on.\n");
            } else if (result == SPACE_OP_ERROR_MISSING_DST) {
                daemon_fail(rsp, "could not locate the active space of the given display.\n");
            } else if (result == SPACE_OP_ERROR_INVALID_SRC) {
                daemon_fail(rsp, "acting space is the last user-space on the source display and cannot be moved.\n");
            } else if (result == SPACE_OP_ERROR_INVALID_DST) {
                daemon_fail(rsp, "acting space is already located on the given display.\n");
            }
        }
    } else if (token_equals(command, COMMAND_SPACE_CREATE)) {
        space_manager_add_space(acting_sid);
    } else if (token_equals(command, COMMAND_SPACE_DESTROY)) {
        enum space_op_error result = space_manager_destroy_space(acting_sid);
        if (result == SPACE_OP_ERROR_MISSING_SRC) {
            daemon_fail(rsp, "could not locate the space to act on.\n");
        } else if (result == SPACE_OP_ERROR_INVALID_SRC) {
            daemon_fail(rsp, "acting space is the last user-space on the source display and cannot be destroyed.\n");
        } else if (result == SPACE_OP_ERROR_INVALID_TYPE) {
            daemon_fail(rsp, "cannot destroy a macOS fullscreen space.\n");
        }
    } else if (token_equals(command, COMMAND_SPACE_BALANCE)) {
        space_manager_balance_space(&g_space_manager, acting_sid);
    } else if (token_equals(command, COMMAND_SPACE_MIRROR)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_MIRROR_X)) {
            space_manager_mirror_space(&g_space_manager, acting_sid, SPLIT_X);
        } else if (token_equals(value, ARGUMENT_SPACE_MIRROR_Y)) {
            space_manager_mirror_space(&g_space_manager, acting_sid, SPLIT_Y);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_ROTATE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_ROTATE_90)) {
            space_manager_rotate_space(&g_space_manager, acting_sid, 90);
        } else if (token_equals(value, ARGUMENT_SPACE_ROTATE_180)) {
            space_manager_rotate_space(&g_space_manager, acting_sid, 180);
        } else if (token_equals(value, ARGUMENT_SPACE_ROTATE_270)) {
            space_manager_rotate_space(&g_space_manager, acting_sid, 270);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_PADDING)) {
        int t, b, l, r;
        char type[MAXLEN];
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_SPACE_PADDING, type, &t, &b, &l, &r) == 5)) {
            space_manager_set_padding_for_space(&g_space_manager, acting_sid, parse_value_type(type), t, b, l, r);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_GAP)) {
        int gap;
        char type[MAXLEN];
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_SPACE_GAP, type, &gap) == 2)) {
            space_manager_set_gap_for_space(&g_space_manager, acting_sid, parse_value_type(type), gap);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_TOGGLE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_TGL_PADDING)) {
            space_manager_toggle_padding_for_space(&g_space_manager, acting_sid);
        } else if (token_equals(value, ARGUMENT_SPACE_TGL_GAP)) {
            space_manager_toggle_gap_for_space(&g_space_manager, acting_sid);
        } else if (token_equals(value, ARGUMENT_SPACE_TGL_MC)) {
            space_manager_toggle_mission_control(acting_sid);
        } else if (token_equals(value, ARGUMENT_SPACE_TGL_SD)) {
            space_manager_toggle_show_desktop(acting_sid);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_LAYOUT)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_LAYOUT_BSP)) {
            if (space_is_user(acting_sid)) {
                space_manager_set_layout_for_space(&g_space_manager, acting_sid, VIEW_BSP);
            } else {
                daemon_fail(rsp, "cannot set layout for a macOS fullscreen space!\n");
            }
        } else if (token_equals(value, ARGUMENT_SPACE_LAYOUT_FLT)) {
            if (space_is_user(acting_sid)) {
                space_manager_set_layout_for_space(&g_space_manager, acting_sid, VIEW_FLOAT);
            } else {
                daemon_fail(rsp, "cannot set layout for a macOS fullscreen space!\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static void handle_domain_window(FILE *rsp, struct token domain, char *message)
{
    struct token command;
    struct window *acting_window;
    struct selector selector = parse_window_selector(NULL, &message, window_manager_focused_window(&g_window_manager));

    if (selector.did_parse) {
        acting_window = selector.window;
        command = get_token(&message);
    } else {
        acting_window = window_manager_focused_window(&g_window_manager);
        command = selector.token;
    }

    if (!acting_window && !token_equals(command, COMMAND_WINDOW_FOCUS)) {
        daemon_fail(rsp, "could not locate the window to act on!\n");
        return;
    }

    if (token_equals(command, COMMAND_WINDOW_FOCUS)) {
        struct selector selector = parse_window_selector(rsp, &message, acting_window);
        if (selector.did_parse && selector.window) {
            window_manager_focus_window_with_raise(&selector.window->application->psn, selector.window->id, selector.window->ref);
        }
    } else if (token_equals(command, COMMAND_WINDOW_SWAP)) {
        struct selector selector = parse_window_selector(rsp, &message, acting_window);
        if (selector.did_parse && selector.window) {
            window_manager_swap_window(&g_space_manager, &g_window_manager, acting_window, selector.window);
        }
    } else if (token_equals(command, COMMAND_WINDOW_WARP)) {
        struct selector selector = parse_window_selector(rsp, &message, acting_window);
        if (selector.did_parse && selector.window) {
            window_manager_warp_window(&g_space_manager, &g_window_manager, acting_window, selector.window);
        }
    } else if (token_equals(command, COMMAND_WINDOW_INSERT)) {
        struct selector selector = parse_dir_selector(rsp, &message);
        if (selector.did_parse && selector.dir) {
            window_manager_set_window_insertion(&g_space_manager, &g_window_manager, acting_window, selector.dir);
        }
    } else if (token_equals(command, COMMAND_WINDOW_GRID)) {
        unsigned r, c, x, y, w, h;
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_WINDOW_GRID, &r, &c, &x, &y, &w, &h) == 6)) {
            window_manager_apply_grid(&g_space_manager, &g_window_manager, acting_window, r, c, x, y, w, h);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_MOVE)) {
        float x, y;
        char type[MAXLEN];
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_WINDOW_MOVE, type, &x, &y) == 3)) {
            window_manager_move_window_relative(&g_window_manager, acting_window, parse_value_type(type), x, y);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_RESIZE)) {
        float w, h;
        char handle[MAXLEN];
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_WINDOW_RESIZE, handle, &w, &h) == 3)) {
            window_manager_resize_window_relative(&g_window_manager, acting_window, parse_resize_handle(handle), w, h);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_CLOSE)) {
        if (!window_manager_close_window(acting_window)) {
            daemon_fail(rsp, "could not close window with id '%d'\n", acting_window->id);
        }
    } else if (token_equals(command, COMMAND_WINDOW_TOGGLE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_FLOAT)) {
            window_manager_toggle_window_float(&g_space_manager, &g_window_manager, acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_ON_TOP)) {
            window_manager_toggle_window_topmost(acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_STICKY)) {
            window_manager_toggle_window_sticky(&g_space_manager, &g_window_manager, acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_SHADOW)) {
            window_manager_toggle_window_shadow(&g_space_manager, &g_window_manager, acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_SPLIT)) {
            space_manager_toggle_window_split(&g_space_manager, acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_PARENT)) {
            window_manager_toggle_window_parent(&g_space_manager, &g_window_manager, acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_FULLSC)) {
            window_manager_toggle_window_fullscreen(&g_space_manager, &g_window_manager, acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_NATIVE)) {
            window_manager_toggle_window_native_fullscreen(&g_space_manager, &g_window_manager, acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_BORDER)) {
            window_manager_toggle_window_border(&g_window_manager, acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_EXPOSE)) {
            window_manager_toggle_window_expose(&g_window_manager, acting_window);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_DISPLAY)) {
        struct selector selector = parse_display_selector(rsp, &message, display_manager_active_display_id());
        if (selector.did_parse && selector.did) {
            uint64_t sid = display_space_id(selector.did);
            if (space_is_fullscreen(sid)) {
                daemon_fail(rsp, "can not move window to a macOS fullscreen space!\n");
            } else {
                window_manager_send_window_to_space(&g_space_manager, &g_window_manager, acting_window, sid);
            }
        }
    } else if (token_equals(command, COMMAND_WINDOW_SPACE)) {
        struct selector selector = parse_space_selector(rsp, &message, space_manager_active_space());
        if (selector.did_parse && selector.sid) {
            if (space_is_fullscreen(selector.sid)) {
                daemon_fail(rsp, "can not move window to a macOS fullscreen space!\n");
            } else {
                window_manager_send_window_to_space(&g_space_manager, &g_window_manager, acting_window, selector.sid);
            }
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
            uint32_t acting_did = display_manager_active_display_id();
            struct selector selector = parse_display_selector(NULL, &message, acting_did);
            if (selector.did_parse || token_is_valid(selector.token)) {
                if (selector.did) {
                    display_serialize(rsp, selector.did);
                    fprintf(rsp, "\n");
                } else {
                    daemon_fail(rsp, "could not locate the selected display.\n");
                }
            } else {
                display_serialize(rsp, acting_did);
                fprintf(rsp, "\n");
            }
        } else if (token_equals(option, ARGUMENT_QUERY_SPACE)) {
            uint64_t acting_sid = space_manager_active_space();
            struct selector selector = parse_space_selector(NULL, &message, acting_sid);
            if (selector.did_parse || token_is_valid(selector.token)) {
                if (selector.sid) {
                    display_serialize(rsp, space_display_id(selector.sid));
                    fprintf(rsp, "\n");
                } else {
                    daemon_fail(rsp, "could not locate the selected space.\n");
                }
            } else {
                display_serialize(rsp, space_display_id(acting_sid));
                fprintf(rsp, "\n");
            }
        } else if (token_equals(option, ARGUMENT_QUERY_WINDOW)) {
            struct window *acting_window = window_manager_focused_window(&g_window_manager);
            struct selector selector = parse_window_selector(NULL, &message, acting_window);
            if (selector.did_parse || token_is_valid(selector.token)) {
                if (selector.window) {
                    display_serialize(rsp, window_display_id(selector.window));
                    fprintf(rsp, "\n");
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                if (acting_window) {
                    display_serialize(rsp, window_display_id(acting_window));
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
            uint32_t acting_did = display_manager_active_display_id();
            struct selector selector = parse_display_selector(NULL, &message, acting_did);
            if (selector.did_parse || token_is_valid(selector.token)) {
                if (selector.did) {
                    if (!space_manager_query_spaces_for_display(rsp, selector.did)) {
                        daemon_fail(rsp, "could not retrieve spaces for display\n");
                    }
                } else {
                    daemon_fail(rsp, "could not locate the selected display.\n");
                }
            } else {
                if (!space_manager_query_spaces_for_display(rsp, acting_did)) {
                    daemon_fail(rsp, "could not retrieve spaces for display\n");
                }
            }
        } else if (token_equals(option, ARGUMENT_QUERY_SPACE)) {
            uint64_t acting_sid = space_manager_active_space();
            struct selector selector = parse_space_selector(NULL, &message, acting_sid);
            if (selector.did_parse || token_is_valid(selector.token)) {
                if (selector.sid) {
                    struct view *view = space_manager_query_view(&g_space_manager, selector.sid);
                    if (view) {
                        view_serialize(rsp, view);
                        fprintf(rsp, "\n");
                    } else {
                        daemon_fail(rsp, "could not locate space with id '%lld'.\n", selector.sid);
                    }
                } else {
                    daemon_fail(rsp, "could not locate the selected space.\n");
                }
            } else {
                if (!space_manager_query_active_space(rsp)) {
                    daemon_fail(rsp, "could not retrieve active space\n");
                }
            }
        } else if (token_equals(option, ARGUMENT_QUERY_WINDOW)) {
            struct window *acting_window = window_manager_focused_window(&g_window_manager);
            struct selector selector = parse_window_selector(NULL, &message, acting_window);
            if (selector.did_parse || token_is_valid(selector.token)) {
                if (selector.window) {
                    space_manager_query_spaces_for_window(rsp, selector.window);
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                if (acting_window) {
                    space_manager_query_spaces_for_window(rsp, acting_window);
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
            uint32_t acting_did = display_manager_active_display_id();
            struct selector selector = parse_display_selector(NULL, &message, acting_did);
            if (selector.did_parse || token_is_valid(selector.token)) {
                if (selector.did) {
                    window_manager_query_windows_for_display(rsp, selector.did);
                } else {
                    daemon_fail(rsp, "could not locate the selected display.\n");
                }
            } else {
                window_manager_query_windows_for_display(rsp, acting_did);
            }
        } else if (token_equals(option, ARGUMENT_QUERY_SPACE)) {
            uint64_t acting_sid = space_manager_active_space();
            struct selector selector = parse_space_selector(NULL, &message, acting_sid);
            if (selector.did_parse || token_is_valid(selector.token)) {
                if (selector.sid) {
                    window_manager_query_windows_for_space(rsp, selector.sid);
                } else {
                    daemon_fail(rsp, "could not locate the selected space.\n");
                }
            } else {
                window_manager_query_windows_for_space(rsp, acting_sid);
            }
        } else if (token_equals(option, ARGUMENT_QUERY_WINDOW)) {
            struct window *acting_window = window_manager_focused_window(&g_window_manager);
            struct selector selector = parse_window_selector(NULL, &message, acting_window);
            if (selector.did_parse || token_is_valid(selector.token)) {
                if (selector.window) {
                    window_serialize(rsp, selector.window);
                    fprintf(rsp, "\n");
                } else {
                    daemon_fail(rsp, "could not locate the selected window.\n");
                }
            } else {
                if (acting_window) {
                    window_serialize(rsp, acting_window);
                    fprintf(rsp, "\n");
                } else {
                    daemon_fail(rsp, "could not retrieve window details\n");
                }
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
                if (!rule->app_regex_valid) {
                    daemon_fail(rsp, "could not compile regex for pattern '%s'\n", value);
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_TITLE)) {
                rule->title_regex_valid = regcomp(&rule->title_regex, value, REG_EXTENDED) == 0;
                if (!rule->title_regex_valid) {
                    daemon_fail(rsp, "could not compile regex for pattern '%s'\n", value);
                }
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
            } else if (string_equals(key, ARGUMENT_RULE_KEY_ON_TOP)) {
                if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    rule->topmost = RULE_PROP_ON;
                } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    rule->topmost = RULE_PROP_OFF;
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
        struct signal signal = {};

        struct token token = get_token(&message);
        while (token.text && token.length > 0) {
            char *key = strtok(token.text, "=");
            char *value = strtok(NULL, "=");
            if (!key || !value) {
                daemon_fail(rsp, "invalid key-value pair '%s'\n", token.text);
                return;
            }

            if (string_equals(key, ARGUMENT_SIGNAL_KEY_APP)) {
                signal.app_regex_valid = regcomp(&signal.app_regex, value, REG_EXTENDED) == 0;
                if (!signal.app_regex_valid) {
                    daemon_fail(rsp, "could not compile regex for pattern '%s'\n", value);
                }
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_TITLE)) {
                signal.title_regex_valid = regcomp(&signal.title_regex, value, REG_EXTENDED) == 0;
                if (!signal.title_regex_valid) {
                    daemon_fail(rsp, "could not compile regex for pattern '%s'\n", value);
                }
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_EVENT)) {
                signal_type = event_type_from_string(value);
                if (signal_type == EVENT_TYPE_UNKNOWN) {
                    daemon_fail(rsp, "unknown event-type '%s'\n", value);
                }
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_ACTION)) {
                signal.command = string_copy(value);
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_LABEL)) {
                signal.label = string_copy(value);
            }

            token = get_token(&message);
        }

        if (signal_type > EVENT_TYPE_UNKNOWN && signal_type < EVENT_TYPE_COUNT && signal.command) {
            event_signal_add(signal_type, signal);
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

#include "message.h"

extern struct signal *g_signal_event[EVENT_TYPE_COUNT];
extern struct event_loop g_event_loop;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;
extern struct mouse_state g_mouse_state;
extern bool g_verbose;

#define DOMAIN_CONFIG  "config"
#define DOMAIN_DISPLAY "display"
#define DOMAIN_SPACE   "space"
#define DOMAIN_WINDOW  "window"
#define DOMAIN_QUERY   "query"
#define DOMAIN_RULE    "rule"
#define DOMAIN_SIGNAL  "signal"

/* --------------------------------DOMAIN CONFIG-------------------------------- */
#define COMMAND_CONFIG_DEBUG_OUTPUT          "debug_output"
#define COMMAND_CONFIG_MFF                   "mouse_follows_focus"
#define COMMAND_CONFIG_FFM                   "focus_follows_mouse"
#define COMMAND_CONFIG_WINDOW_PLACEMENT      "window_placement"
#define COMMAND_CONFIG_TOPMOST               "window_topmost"
#define COMMAND_CONFIG_OPACITY               "window_opacity"
#define COMMAND_CONFIG_OPACITY_DURATION      "window_opacity_duration"
#define COMMAND_CONFIG_BORDER                "window_border"
#define COMMAND_CONFIG_BORDER_WIDTH          "window_border_width"
#define COMMAND_CONFIG_BORDER_ACTIVE_COLOR   "active_window_border_color"
#define COMMAND_CONFIG_BORDER_NORMAL_COLOR   "normal_window_border_color"
#define COMMAND_CONFIG_SHADOW                "window_shadow"
#define COMMAND_CONFIG_ACTIVE_WINDOW_OPACITY "active_window_opacity"
#define COMMAND_CONFIG_NORMAL_WINDOW_OPACITY "normal_window_opacity"
#define COMMAND_CONFIG_INSERT_FEEDBACK_COLOR "insert_feedback_color"
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
#define COMMAND_CONFIG_MOUSE_DROP_ACTION     "mouse_drop_action"
#define COMMAND_CONFIG_EXTERNAL_BAR          "external_bar"

#define SELECTOR_CONFIG_SPACE                "--space"

#define ARGUMENT_CONFIG_FFM_AUTOFOCUS        "autofocus"
#define ARGUMENT_CONFIG_FFM_AUTORAISE        "autoraise"
#define ARGUMENT_CONFIG_WINDOW_PLACEMENT_FST "first_child"
#define ARGUMENT_CONFIG_WINDOW_PLACEMENT_SND "second_child"
#define ARGUMENT_CONFIG_SHADOW_FLT           "float"
#define ARGUMENT_CONFIG_LAYOUT_BSP           "bsp"
#define ARGUMENT_CONFIG_LAYOUT_STACK         "stack"
#define ARGUMENT_CONFIG_LAYOUT_FLOAT         "float"
#define ARGUMENT_CONFIG_MOUSE_MOD_ALT        "alt"
#define ARGUMENT_CONFIG_MOUSE_MOD_SHIFT      "shift"
#define ARGUMENT_CONFIG_MOUSE_MOD_CMD        "cmd"
#define ARGUMENT_CONFIG_MOUSE_MOD_CTRL       "ctrl"
#define ARGUMENT_CONFIG_MOUSE_MOD_FN         "fn"
#define ARGUMENT_CONFIG_MOUSE_ACTION_MOVE    "move"
#define ARGUMENT_CONFIG_MOUSE_ACTION_RESIZE  "resize"
#define ARGUMENT_CONFIG_MOUSE_ACTION_SWAP    "swap"
#define ARGUMENT_CONFIG_MOUSE_ACTION_STACK   "stack"
#define ARGUMENT_CONFIG_EXTERNAL_BAR_MAIN    "main"
#define ARGUMENT_CONFIG_EXTERNAL_BAR_ALL     "all"
#define ARGUMENT_CONFIG_EXTERNAL_BAR         "%5[^:]:%d:%d"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN DISPLAY------------------------------- */
#define COMMAND_DISPLAY_FOCUS "--focus"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN SPACE--------------------------------- */
#define COMMAND_SPACE_FOCUS   "--focus"
#define COMMAND_SPACE_SWITCH  "--switch"
#define COMMAND_SPACE_CREATE  "--create"
#define COMMAND_SPACE_DESTROY "--destroy"
#define COMMAND_SPACE_MOVE    "--move"
#define COMMAND_SPACE_SWAP    "--swap"
#define COMMAND_SPACE_DISPLAY "--display"
#define COMMAND_SPACE_BALANCE "--balance"
#define COMMAND_SPACE_MIRROR  "--mirror"
#define COMMAND_SPACE_ROTATE  "--rotate"
#define COMMAND_SPACE_PADDING "--padding"
#define COMMAND_SPACE_GAP     "--gap"
#define COMMAND_SPACE_TOGGLE  "--toggle"
#define COMMAND_SPACE_LAYOUT  "--layout"
#define COMMAND_SPACE_LABEL   "--label"

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
#define ARGUMENT_SPACE_LAYOUT_STACK "stack"
#define ARGUMENT_SPACE_LAYOUT_FLT   "float"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN WINDOW-------------------------------- */
#define COMMAND_WINDOW_FOCUS   "--focus"
#define COMMAND_WINDOW_SWAP    "--swap"
#define COMMAND_WINDOW_WARP    "--warp"
#define COMMAND_WINDOW_STACK   "--stack"
#define COMMAND_WINDOW_INSERT  "--insert"
#define COMMAND_WINDOW_GRID    "--grid"
#define COMMAND_WINDOW_MOVE    "--move"
#define COMMAND_WINDOW_RESIZE  "--resize"
#define COMMAND_WINDOW_RATIO   "--ratio"
#define COMMAND_WINDOW_MIN     "--minimize"
#define COMMAND_WINDOW_DEMIN   "--deminimize"
#define COMMAND_WINDOW_CLOSE   "--close"
#define COMMAND_WINDOW_LAYER   "--layer"
#define COMMAND_WINDOW_OPACITY "--opacity"
#define COMMAND_WINDOW_TOGGLE  "--toggle"
#define COMMAND_WINDOW_DISPLAY "--display"
#define COMMAND_WINDOW_SPACE   "--space"
#define COMMAND_WINDOW_SWITCH  "--switch"

#define ARGUMENT_WINDOW_SEL_LARGEST   "largest"
#define ARGUMENT_WINDOW_SEL_SMALLEST  "smallest"
#define ARGUMENT_WINDOW_GRID          "%d:%d:%d:%d:%d:%d"
#define ARGUMENT_WINDOW_MOVE          "%255[^:]:%f:%f"
#define ARGUMENT_WINDOW_RESIZE        "%255[^:]:%f:%f"
#define ARGUMENT_WINDOW_RATIO         "%255[^:]:%f"
#define ARGUMENT_WINDOW_LAYER_BELOW   "below"
#define ARGUMENT_WINDOW_LAYER_NORMAL  "normal"
#define ARGUMENT_WINDOW_LAYER_ABOVE   "above"
#define ARGUMENT_WINDOW_TOGGLE_ON_TOP "topmost"
#define ARGUMENT_WINDOW_TOGGLE_FLOAT  "float"
#define ARGUMENT_WINDOW_TOGGLE_STICKY "sticky"
#define ARGUMENT_WINDOW_TOGGLE_SHADOW "shadow"
#define ARGUMENT_WINDOW_TOGGLE_SPLIT  "split"
#define ARGUMENT_WINDOW_TOGGLE_PARENT "zoom-parent"
#define ARGUMENT_WINDOW_TOGGLE_FULLSC "zoom-fullscreen"
#define ARGUMENT_WINDOW_TOGGLE_NATIVE "native-fullscreen"
#define ARGUMENT_WINDOW_TOGGLE_EXPOSE "expose"
#define ARGUMENT_WINDOW_TOGGLE_PIP    "pip"
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
#define COMMAND_RULE_LS  "--list"

#define ARGUMENT_RULE_KEY_APP     "app"
#define ARGUMENT_RULE_KEY_TITLE   "title"
#define ARGUMENT_RULE_KEY_DISPLAY "display"
#define ARGUMENT_RULE_KEY_SPACE   "space"
#define ARGUMENT_RULE_KEY_ALPHA   "opacity"
#define ARGUMENT_RULE_KEY_MANAGE  "manage"
#define ARGUMENT_RULE_KEY_STICKY  "sticky"
#define ARGUMENT_RULE_KEY_MFF     "mouse_follows_focus"
#define ARGUMENT_RULE_KEY_LAYER   "layer"
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
#define COMMAND_SIGNAL_LS  "--list"

#define ARGUMENT_SIGNAL_KEY_APP      "app"
#define ARGUMENT_SIGNAL_KEY_TITLE    "title"
#define ARGUMENT_SIGNAL_KEY_EVENT    "event"
#define ARGUMENT_SIGNAL_KEY_ACTION   "action"
#define ARGUMENT_SIGNAL_KEY_LABEL    "label"
/* ----------------------------------------------------------------------------- */

/* --------------------------------COMMON ARGUMENTS----------------------------- */
#define ARGUMENT_COMMON_VAL_ON           "on"
#define ARGUMENT_COMMON_VAL_OFF          "off"
#define ARGUMENT_COMMON_SEL_PREV         "prev"
#define ARGUMENT_COMMON_SEL_NEXT         "next"
#define ARGUMENT_COMMON_SEL_FIRST        "first"
#define ARGUMENT_COMMON_SEL_LAST         "last"
#define ARGUMENT_COMMON_SEL_RECENT       "recent"
#define ARGUMENT_COMMON_SEL_NORTH        "north"
#define ARGUMENT_COMMON_SEL_EAST         "east"
#define ARGUMENT_COMMON_SEL_SOUTH        "south"
#define ARGUMENT_COMMON_SEL_WEST         "west"
#define ARGUMENT_COMMON_SEL_MOUSE        "mouse"
#define ARGUMENT_COMMON_SEL_STACK        "stack"
#define ARGUMENT_COMMON_SEL_STACK_PREV   "stack.prev"
#define ARGUMENT_COMMON_SEL_STACK_NEXT   "stack.next"
#define ARGUMENT_COMMON_SEL_STACK_FIRST  "stack.first"
#define ARGUMENT_COMMON_SEL_STACK_LAST   "stack.last"
#define ARGUMENT_COMMON_SEL_STACK_RECENT "stack.recent"
/* ----------------------------------------------------------------------------- */

struct token
{
    char *text;
    unsigned int length;
};

enum token_type
{
    TOKEN_TYPE_INVALID,
    TOKEN_TYPE_UNKNOWN,
    TOKEN_TYPE_INT,
    TOKEN_TYPE_FLOAT,
    TOKEN_TYPE_U32,
    TOKEN_TYPE_STRING
};

struct token_value
{
    struct token token;
    enum token_type type;

    union {
        int int_value;
        float float_value;
        uint32_t u32_value;
        char *string_value;
    };
};

static const int token_char_int_table[] =
{
    ['0'] = 0x0, ['1'] = 0x1,
    ['2'] = 0x2, ['3'] = 0x3,
    ['4'] = 0x4, ['5'] = 0x5,
    ['6'] = 0x6, ['7'] = 0x7,
    ['8'] = 0x8, ['9'] = 0x9,
    ['a'] = 0xA, ['A'] = 0xA,
    ['b'] = 0xB, ['B'] = 0xB,
    ['c'] = 0xC, ['C'] = 0xC,
    ['d'] = 0xD, ['D'] = 0xD,
    ['e'] = 0xE, ['E'] = 0xE,
    ['f'] = 0xF, ['F'] = 0xF,
};

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

static bool token_is_positive_integer(struct token token, int *value)
{
    *value = 0;

    for (int i = 0; i < token.length; ++i) {
        char c = token.text[i];
        if (!(c >= '0' && c <= '9')) {
            return false;
        }
        *value = *value * 10 + token_char_int_table[(int)c];
    }

    return true;
}

static bool token_is_hexadecimal(struct token token, uint32_t *value)
{
    *value = 0;

    if (token.length <= 2) return false;

    if (!(token.text[0] == '0' &&
         (token.text[1] == 'x' ||
          token.text[1] == 'X'))) {
        return false;
    }

    for (int i = 2; i < token.length; ++i) {
        char c = token.text[i];
        if (!((c >= '0' && c <= '9') ||
              (c >= 'a' && c <= 'f') ||
              (c >= 'A' && c <= 'F'))) {
            return false;
        }
        *value = *value * 16 + (uint32_t)token_char_int_table[(int)c];
    }

    return true;
}

static bool token_is_float(struct token token, float *value)
{
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';

    char *end = NULL;
    float v = strtof(buffer, &end);

    if (!end || *end) {
        *value = 0.0f;
        return false;
    } else {
        *value = v;
        return true;
    }
}

static char *token_to_string(struct token token, bool temp)
{
    unsigned int length = token.length + 1;
    char *result = temp ? ts_alloc(length) : malloc(length);
    if (!result) return NULL;

    memcpy(result, token.text, token.length);
    result[token.length] = '\0';
    return result;
}

static struct token_value token_to_value(struct token token, bool allocate_string)
{
    struct token_value value = { .token = token, .type = TOKEN_TYPE_INVALID };

    if (token_is_valid(token)) {
        if (token_is_positive_integer(token, &value.int_value)) {
            value.type = TOKEN_TYPE_INT;
        } else if (token_is_hexadecimal(token, &value.u32_value)) {
            value.type = TOKEN_TYPE_U32;
        } else if (token_is_float(token, &value.float_value)) {
            value.type = TOKEN_TYPE_FLOAT;
        } else if (!allocate_string) {
            value.type = TOKEN_TYPE_STRING;
        } else if ((value.string_value = token_to_string(token, true))) {
            value.type = TOKEN_TYPE_STRING;
        } else {
            value.type = TOKEN_TYPE_UNKNOWN;
        }
    }

    return value;
}

static void get_key_value_pair(char *token, char **key, char **value, bool *exclusion)
{
    *key = token;

    while (*token) {
        char fst = token[0];
        char snd = token[1];

        if (fst == '!' && snd == '=') {
            break;
        } else if (fst == '=') {
            break;
        }

        ++token;
    }

    int index = (token[0] == '!' && token[1] == '=') ? 2 : 1;
    char check = (index == 2) ? '!' : '=';

    if (*token != check) {
        *key = NULL;
        *value = NULL;
    } else if (token[index]) {
        *token = '\0';
        *value = token+index;
        *exclusion = index == 2;
    } else {
        *value = NULL;
    }
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

__unused static void daemon_deprecated(FILE *rsp, char *fmt, ...)
{
    if (!rsp) return;

    fprintf(rsp, "deprecation warning: ");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(rsp, fmt, ap);
    va_end(ap);
}

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

enum label_type
{
    LABEL_SPACE,
};

static char *reserved_space_identifiers[] =
{
    ARGUMENT_COMMON_SEL_PREV,
    ARGUMENT_COMMON_SEL_NEXT,
    ARGUMENT_COMMON_SEL_FIRST,
    ARGUMENT_COMMON_SEL_LAST,
    ARGUMENT_COMMON_SEL_RECENT
};

static bool parse_label(FILE *rsp, char **message, enum label_type type, char **label)
{
    struct token token = get_token(message);
    struct token_value value = token_to_value(token, false);

    if (value.type == TOKEN_TYPE_INVALID) {
        *label = NULL;
        return true;
    }

    if (value.type != TOKEN_TYPE_STRING) {
        daemon_fail(rsp, "'%.*s' cannot be used as a label.\n", token.length, token.text);
        return false;
    }

    switch (type) {
    default: break;
    case LABEL_SPACE: {
        for (int i = 0; i < array_count(reserved_space_identifiers); ++i) {
            if (token_equals(token, reserved_space_identifiers[i])) {
                daemon_fail(rsp, "'%.*s' is a reserved keyword and cannot be used as a label.\n", token.length, token.text);
                return false;
            }
        }
    } break;
    }

    *label = token_to_string(token, false);
    return true;
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
    struct selector result = { .token = get_token(message), .did_parse = true };

    struct token_value value = token_to_value(result.token, false);
    if (value.type == TOKEN_TYPE_INT) {
        uint32_t did = display_manager_arrangement_display_id(value.int_value);
        if (did) {
            result.did = did;
        } else {
            daemon_fail(rsp, "could not locate display with arrangement index '%d'.\n", value.int_value);
        }
    } else if (value.type == TOKEN_TYPE_STRING) {
        if (token_equals(result.token, ARGUMENT_COMMON_SEL_NORTH)) {
            if (acting_did) {
                uint32_t did = display_manager_find_closest_display_in_direction(acting_did, DIR_NORTH);
                if (did) {
                    result.did = did;
                } else {
                    daemon_fail(rsp, "could not locate a northward display.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected display.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_EAST)) {
            if (acting_did) {
                uint32_t did = display_manager_find_closest_display_in_direction(acting_did, DIR_EAST);
                if (did) {
                    result.did = did;
                } else {
                    daemon_fail(rsp, "could not locate a eastward display.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected display.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_SOUTH)) {
            if (acting_did) {
                uint32_t did = display_manager_find_closest_display_in_direction(acting_did, DIR_SOUTH);
                if (did) {
                    result.did = did;
                } else {
                    daemon_fail(rsp, "could not locate a southward display.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected display.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_WEST)) {
            if (acting_did) {
                uint32_t did = display_manager_find_closest_display_in_direction(acting_did, DIR_WEST);
                if (did) {
                    result.did = did;
                } else {
                    daemon_fail(rsp, "could not locate a westward display.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected display.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_PREV)) {
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
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_MOUSE)) {
            uint32_t did = display_manager_cursor_display_id();
            if (did) {
                result.did = did;
            } else {
                daemon_fail(rsp, "could not locate display containing cursor.\n");
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
    struct selector result = { .token = get_token(message), .did_parse = true };

    struct token_value value = token_to_value(result.token, true);
    if (value.type == TOKEN_TYPE_INT) {
        uint64_t sid = space_manager_mission_control_space(value.int_value);
        if (sid) {
            result.sid = sid;
        } else {
            daemon_fail(rsp, "could not locate space with mission-control index '%d'.\n", value.int_value);
        }
    } else if (value.type == TOKEN_TYPE_STRING) {
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
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_MOUSE)) {
            uint64_t sid = space_manager_cursor_space();
            if (sid) {
                result.sid = sid;
            } else {
                daemon_fail(rsp, "could not locate space containing cursor.\n");
            }
        } else {
            struct space_label *space_label = space_manager_get_space_for_label(&g_space_manager, value.string_value);
            if (space_label) {
                result.did_parse = true;
                result.sid = space_label->sid;
            } else {
                result.did_parse = false;
                daemon_fail(rsp, "value '%.*s' is not a valid option for SPACE_SEL\n", result.token.length, result.token.text);
            }
        }
    } else {
        result.did_parse = false;
        daemon_fail(rsp, "value '%.*s' is not a valid option for SPACE_SEL\n", result.token.length, result.token.text);
    }

    return result;
}

static struct selector parse_window_selector(FILE *rsp, char **message, struct window *acting_window)
{
    struct selector result = { .token = get_token(message), .did_parse = true };

    struct token_value value = token_to_value(result.token, false);
    if (value.type == TOKEN_TYPE_INT) {
        struct window *window = window_manager_find_window(&g_window_manager, value.int_value);
        if (window) {
            result.window = window;
        } else {
            daemon_fail(rsp, "could not locate window with the specified id '%d'.\n", value.int_value);
        }
    } else if (value.type == TOKEN_TYPE_STRING) {
        if (token_equals(result.token, ARGUMENT_COMMON_SEL_NORTH)) {
            if (acting_window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, acting_window, DIR_NORTH);
                if (closest_window) {
                    result.window = closest_window;
                } else {
                    daemon_fail(rsp, "could not locate a northward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_EAST)) {
            if (acting_window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, acting_window, DIR_EAST);
                if (closest_window) {
                    result.window = closest_window;
                } else {
                    daemon_fail(rsp, "could not locate a eastward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_SOUTH)) {
            if (acting_window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, acting_window, DIR_SOUTH);
                if (closest_window) {
                    result.window = closest_window;
                } else {
                    daemon_fail(rsp, "could not locate a southward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_WEST)) {
            if (acting_window) {
                struct window *closest_window = window_manager_find_closest_managed_window_in_direction(&g_window_manager, acting_window, DIR_WEST);
                if (closest_window) {
                    result.window = closest_window;
                } else {
                    daemon_fail(rsp, "could not locate a westward managed window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_MOUSE)) {
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
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_PREV)) {
            if (acting_window) {
                struct window *prev_window = window_manager_find_prev_window_in_stack(&g_space_manager, &g_window_manager, acting_window);
                if (prev_window) {
                    result.window = prev_window;
                } else {
                    daemon_fail(rsp, "could not locate the prev stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_NEXT)) {
            if (acting_window) {
                struct window *next_window = window_manager_find_next_window_in_stack(&g_space_manager, &g_window_manager, acting_window);
                if (next_window) {
                    result.window = next_window;
                } else {
                    daemon_fail(rsp, "could not locate the next stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_FIRST)) {
            if (acting_window) {
                struct window *first_window = window_manager_find_first_window_in_stack(&g_space_manager, &g_window_manager, acting_window);
                if (first_window) {
                    result.window = first_window;
                } else {
                    daemon_fail(rsp, "could not locate the first stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_LAST)) {
            if (acting_window) {
                struct window *last_window = window_manager_find_last_window_in_stack(&g_space_manager, &g_window_manager, acting_window);
                if (last_window) {
                    result.window = last_window;
                } else {
                    daemon_fail(rsp, "could not locate the last stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_RECENT)) {
            if (acting_window) {
                struct window *recent_window = window_manager_find_recent_window_in_stack(&g_space_manager, &g_window_manager, acting_window);
                if (recent_window) {
                    result.window = recent_window;
                } else {
                    daemon_fail(rsp, "could not locate the recent stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
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

static struct selector parse_insert_selector(FILE *rsp, char **message)
{
    struct selector result = { .token = get_token(message), .did_parse = true };

    if (token_equals(result.token, ARGUMENT_COMMON_SEL_NORTH)) {
        result.dir = DIR_NORTH;
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_EAST)) {
        result.dir = DIR_EAST;
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_SOUTH)) {
        result.dir = DIR_SOUTH;
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_WEST)) {
        result.dir = DIR_WEST;
    } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK)) {
        result.dir = STACK;
    } else {
        result.did_parse = false;
        daemon_fail(rsp, "value '%.*s' is not a valid option for DIR_SEL\n", result.token.length, result.token.text);
    }

    return result;
}

static void handle_domain_config(FILE *rsp, struct token domain, char *message)
{
    uint64_t sel_sid = 0;
    struct token selector = get_token(&message);
    struct token command  = selector;

    bool found_selector = token_equals(selector, SELECTOR_CONFIG_SPACE);
    if (found_selector) {
        struct selector space_selector = parse_space_selector(rsp, &message, 0);
        if (!space_selector.did_parse || !space_selector.sid) return;

        sel_sid = space_selector.sid;
        command = get_token(&message);
    }

    if (token_equals(command, COMMAND_CONFIG_DEBUG_OUTPUT)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", bool_str[g_verbose]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            g_verbose = false;
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            g_verbose = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_MFF)) {
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
            window_manager_set_window_opacity_enabled(&g_window_manager, false);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            window_manager_set_window_opacity_enabled(&g_window_manager, true);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_OPACITY_DURATION)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (value.type == TOKEN_TYPE_INVALID) {
            fprintf(rsp, "%f\n", g_window_manager.window_opacity_duration);
        } else if (value.type == TOKEN_TYPE_FLOAT) {
            if (!workspace_is_macos_catalina() && !workspace_is_macos_bigsur()) {
                g_window_manager.window_opacity_duration = value.float_value;
            } else {
                daemon_fail(rsp, "'%s' cannot be changed on macOS Catalina/Big Sur because of an Apple bug in the WindowServer\n", COMMAND_CONFIG_OPACITY_DURATION);
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", bool_str[g_window_manager.enable_window_border]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            window_manager_set_window_border_enabled(&g_window_manager, false);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            window_manager_set_window_border_enabled(&g_window_manager, true);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_WIDTH)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (value.type == TOKEN_TYPE_INVALID) {
            fprintf(rsp, "%d\n", g_window_manager.border_width);
        } else if (value.type == TOKEN_TYPE_INT && value.int_value) {
            window_manager_set_window_border_width(&g_window_manager, value.int_value + ((value.int_value&0x1) ? 1 : 0));
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_ACTIVE_COLOR)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (value.type == TOKEN_TYPE_INVALID) {
            fprintf(rsp, "0x%x\n", g_window_manager.active_border_color.p);
        } else if (value.type == TOKEN_TYPE_U32 && value.u32_value) {
            window_manager_set_active_window_border_color(&g_window_manager, value.u32_value);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_NORMAL_COLOR)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (value.type == TOKEN_TYPE_INVALID) {
            fprintf(rsp, "0x%x\n", g_window_manager.normal_border_color.p);
        } else if (value.type == TOKEN_TYPE_U32 && value.u32_value) {
            window_manager_set_normal_window_border_color(&g_window_manager, value.u32_value);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
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
    } else if (token_equals(command, COMMAND_CONFIG_ACTIVE_WINDOW_OPACITY)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (value.type == TOKEN_TYPE_INVALID) {
            fprintf(rsp, "%.4f\n", g_window_manager.active_window_opacity);
        } else if (value.type == TOKEN_TYPE_FLOAT && in_range_ei(value.float_value, 0.0f, 1.0f)) {
            window_manager_set_active_window_opacity(&g_window_manager, value.float_value);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_NORMAL_WINDOW_OPACITY)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (value.type == TOKEN_TYPE_INVALID) {
            fprintf(rsp, "%.4f\n", g_window_manager.normal_window_opacity);
        } else if (value.type == TOKEN_TYPE_FLOAT && in_range_ei(value.float_value, 0.0f, 1.0f)) {
            window_manager_set_normal_window_opacity(&g_window_manager, value.float_value);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_INSERT_FEEDBACK_COLOR)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (value.type == TOKEN_TYPE_INVALID) {
            fprintf(rsp, "0x%x\n", g_window_manager.insert_feedback_color.p);
        } else if (value.type == TOKEN_TYPE_U32 && value.u32_value) {
            g_window_manager.insert_feedback_color = rgba_color_from_hex(value.u32_value);
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_TOP_PADDING)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (sel_sid) {
            struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%d\n", view->top_padding);
            } else if (value.type == TOKEN_TYPE_INT) {
                view->custom_top_padding = true;
                view->top_padding = value.int_value;
                view_update(view);
                view_flush(view);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else {
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%d\n", g_space_manager.top_padding);
            } else if (value.type == TOKEN_TYPE_INT) {
                space_manager_set_top_padding_for_all_spaces(&g_space_manager, value.int_value);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_BOTTOM_PADDING)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (sel_sid) {
            struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%d\n", view->bottom_padding);
            } else if (value.type == TOKEN_TYPE_INT) {
                view->custom_bottom_padding = true;
                view->bottom_padding = value.int_value;
                view_update(view);
                view_flush(view);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else {
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%d\n", g_space_manager.bottom_padding);
            } else if (value.type == TOKEN_TYPE_INT) {
                space_manager_set_bottom_padding_for_all_spaces(&g_space_manager, value.int_value);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_LEFT_PADDING)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (sel_sid) {
            struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%d\n", view->left_padding);
            } else if (value.type == TOKEN_TYPE_INT) {
                view->custom_left_padding = true;
                view->left_padding = value.int_value;
                view_update(view);
                view_flush(view);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else {
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%d\n", g_space_manager.left_padding);
            } else if (value.type == TOKEN_TYPE_INT) {
                space_manager_set_left_padding_for_all_spaces(&g_space_manager, value.int_value);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_RIGHT_PADDING)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (sel_sid) {
            struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%d\n", view->right_padding);
            } else if (value.type == TOKEN_TYPE_INT) {
                view->custom_right_padding = true;
                view->right_padding = value.int_value;
                view_update(view);
                view_flush(view);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else {
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%d\n", g_space_manager.right_padding);
            } else if (value.type == TOKEN_TYPE_INT) {
                space_manager_set_right_padding_for_all_spaces(&g_space_manager, value.int_value);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_WINDOW_GAP)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (sel_sid) {
            struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%d\n", view->window_gap);
            } else if (value.type == TOKEN_TYPE_INT) {
                view->custom_window_gap = true;
                view->window_gap = value.int_value;
                view_update(view);
                view_flush(view);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else {
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%d\n", g_space_manager.window_gap);
            } else if (value.type == TOKEN_TYPE_INT) {
                space_manager_set_window_gap_for_all_spaces(&g_space_manager, value.int_value);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_LAYOUT)) {
        struct token value = get_token(&message);
        if (sel_sid) {
            struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
            if (!token_is_valid(value)) {
                fprintf(rsp, "%s\n", view_type_str[view->layout]);
            } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_BSP)) {
                if (space_is_user(sel_sid)) {
                    view->layout = VIEW_BSP;
                    view->custom_layout = true;
                    view_clear(view);
                    window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, sel_sid);
                } else {
                    daemon_fail(rsp, "cannot set layout for a macOS fullscreen space!\n");
                }
            } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_STACK)) {
                if (space_is_user(sel_sid)) {
                    view->layout = VIEW_STACK;
                    view->custom_layout = true;
                    view_clear(view);
                    window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, sel_sid);
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
            if (!token_is_valid(value)) {
                fprintf(rsp, "%s\n", view_type_str[g_space_manager.layout]);
            } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_BSP)) {
                space_manager_set_layout_for_all_spaces(&g_space_manager, VIEW_BSP);
            } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_STACK)) {
                space_manager_set_layout_for_all_spaces(&g_space_manager, VIEW_STACK);
            } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_FLOAT)) {
                space_manager_set_layout_for_all_spaces(&g_space_manager, VIEW_FLOAT);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_SPLIT_RATIO)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (value.type == TOKEN_TYPE_INVALID) {
            fprintf(rsp, "%.4f\n", g_space_manager.split_ratio);
        } else if (value.type == TOKEN_TYPE_FLOAT && in_range_ii(value.float_value, 0.1f, 0.9f)) {
            g_space_manager.split_ratio = value.float_value;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
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
    } else if (token_equals(command, COMMAND_CONFIG_MOUSE_DROP_ACTION)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", mouse_mode_str[g_mouse_state.drop_action]);
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_ACTION_SWAP)) {
            g_mouse_state.drop_action = MOUSE_MODE_SWAP;
        } else if (token_equals(value, ARGUMENT_CONFIG_MOUSE_ACTION_STACK)) {
            g_mouse_state.drop_action = MOUSE_MODE_STACK;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_EXTERNAL_BAR)) {
        int t, b;
        char mode[6];
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_CONFIG_EXTERNAL_BAR, mode, &t, &b) == 3)) {
            if (string_equals(mode, ARGUMENT_CONFIG_EXTERNAL_BAR_MAIN)) {
                g_display_manager.mode = EXTERNAL_BAR_MAIN;
                g_display_manager.top_padding = t;
                g_display_manager.bottom_padding = b;
                space_manager_mark_spaces_invalid(&g_space_manager);
            } else if (string_equals(mode, ARGUMENT_CONFIG_EXTERNAL_BAR_ALL)) {
                g_display_manager.mode = EXTERNAL_BAR_ALL;
                g_display_manager.top_padding = t;
                g_display_manager.bottom_padding = b;
                space_manager_mark_spaces_invalid(&g_space_manager);
            } else if (string_equals(mode, ARGUMENT_COMMON_VAL_OFF)) {
                g_display_manager.mode = EXTERNAL_BAR_OFF;
                g_display_manager.top_padding = t;
                g_display_manager.bottom_padding = b;
                space_manager_mark_spaces_invalid(&g_space_manager);
            } else {
                daemon_fail(rsp, "unknown mode '%s' specified in value '%.*s' given to command '%.*s' for domain '%.*s'\n", mode, value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        } else {
            fprintf(rsp, "%s:%d:%d\n", external_bar_mode_str[g_display_manager.mode], g_display_manager.top_padding, g_display_manager.bottom_padding);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
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
            enum space_op_error result = space_manager_focus_space(selector.sid);
            if (result == SPACE_OP_ERROR_SAME_SPACE) {
                daemon_fail(rsp, "cannot focus an already focused space.\n");
            } else if (result == SPACE_OP_ERROR_DISPLAY_IS_ANIMATING) {
                daemon_fail(rsp, "cannot focus space because the display is in the middle of an animation.\n");
            } else if (result == SPACE_OP_ERROR_IN_MISSION_CONTROL) {
                daemon_fail(rsp, "cannot focus space because mission-control is active.\n");
            } else if (result == SPACE_OP_ERROR_SCRIPTING_ADDITION) {
                daemon_fail(rsp, "cannot focus space due to an error with the scripting-addition.\n");
            }
        }
    } else if (token_equals(command, COMMAND_SPACE_SWITCH)) {
        struct selector selector = parse_space_selector(rsp, &message, acting_sid);
        if (selector.did_parse && selector.sid) {
            enum space_op_error result = space_manager_switch_to_space(selector.sid, space_manager_active_space());
            if (result == SPACE_OP_ERROR_SAME_SPACE) {
                daemon_fail(rsp, "cannot focus an already focused space.\n");
            } else if (result == SPACE_OP_ERROR_DISPLAY_IS_ANIMATING) {
                daemon_fail(rsp, "cannot focus space because the display is in the middle of an animation.\n");
            } else if (result == SPACE_OP_ERROR_IN_MISSION_CONTROL) {
                daemon_fail(rsp, "cannot focus space because mission-control is active.\n");
            }
        }
    } else if (token_equals(command, COMMAND_SPACE_MOVE)) {
        struct selector selector = parse_space_selector(rsp, &message, acting_sid);
        if (selector.did_parse && selector.sid) {
            enum space_op_error result = space_manager_move_space_to_space(acting_sid, selector.sid);
            if (result == SPACE_OP_ERROR_SAME_SPACE) {
                daemon_fail(rsp, "cannot move space to itself.\n");
            } else if (result == SPACE_OP_ERROR_SAME_DISPLAY) {
                daemon_fail(rsp, "cannot move space across display boundaries. use --display instead.\n");
            } else if (result == SPACE_OP_ERROR_DISPLAY_IS_ANIMATING) {
                daemon_fail(rsp, "cannot move space because the display is in the middle of an animation.\n");
            } else if (result == SPACE_OP_ERROR_IN_MISSION_CONTROL) {
                daemon_fail(rsp, "cannot move space because mission-control is active.\n");
            } else if (result == SPACE_OP_ERROR_SCRIPTING_ADDITION) {
                daemon_fail(rsp, "cannot move space due to an error with the scripting-addition.\n");
            }
        }
    } else if (token_equals(command, COMMAND_SPACE_SWAP)) {
        struct selector selector = parse_space_selector(rsp, &message, acting_sid);
        if (selector.did_parse && selector.sid) {
            enum space_op_error result = space_manager_swap_space_with_space(acting_sid, selector.sid);
            if (result == SPACE_OP_ERROR_SAME_SPACE) {
                daemon_fail(rsp, "cannot swap space with itself.\n");
            } else if (result == SPACE_OP_ERROR_SAME_DISPLAY) {
                daemon_fail(rsp, "cannot swap space across display boundaries. use --display instead.\n");
            } else if (result == SPACE_OP_ERROR_DISPLAY_IS_ANIMATING) {
                daemon_fail(rsp, "cannot swap space because the display is in the middle of an animation.\n");
            } else if (result == SPACE_OP_ERROR_IN_MISSION_CONTROL) {
                daemon_fail(rsp, "cannot swap space because mission-control is active.\n");
            } else if (result == SPACE_OP_ERROR_SCRIPTING_ADDITION) {
                daemon_fail(rsp, "cannot swap space due to an error with the scripting-addition.\n");
            }
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
            } else if (result == SPACE_OP_ERROR_DISPLAY_IS_ANIMATING) {
                daemon_fail(rsp, "cannot send space to display because it is in the middle of an animation.\n");
            } else if (result == SPACE_OP_ERROR_IN_MISSION_CONTROL) {
                daemon_fail(rsp, "cannot send space to display because mission-control is active.\n");
            } else if (result == SPACE_OP_ERROR_SCRIPTING_ADDITION) {
                daemon_fail(rsp, "cannot send space to display due to an error with the scripting-addition.\n");
            }
        }
    } else if (token_equals(command, COMMAND_SPACE_CREATE)) {
        enum space_op_error result = space_manager_add_space(acting_sid);
        if (result == SPACE_OP_ERROR_MISSING_SRC) {
            daemon_fail(rsp, "could not locate the space to act on.\n");
        } else if (result == SPACE_OP_ERROR_DISPLAY_IS_ANIMATING) {
            daemon_fail(rsp, "cannot create space because the display is in the middle of an animation.\n");
        } else if (result == SPACE_OP_ERROR_IN_MISSION_CONTROL) {
            daemon_fail(rsp, "cannot create space because mission-control is active.\n");
        } else if (result == SPACE_OP_ERROR_SCRIPTING_ADDITION) {
            daemon_fail(rsp, "cannot create space due to an error with the scripting-addition.\n");
        }
    } else if (token_equals(command, COMMAND_SPACE_DESTROY)) {
        enum space_op_error result = space_manager_destroy_space(acting_sid);
        if (result == SPACE_OP_ERROR_MISSING_SRC) {
            daemon_fail(rsp, "could not locate the space to act on.\n");
        } else if (result == SPACE_OP_ERROR_INVALID_SRC) {
            daemon_fail(rsp, "acting space is the last user-space on the source display and cannot be destroyed.\n");
        } else if (result == SPACE_OP_ERROR_INVALID_TYPE) {
            daemon_fail(rsp, "cannot destroy a macOS fullscreen space.\n");
        } else if (result == SPACE_OP_ERROR_DISPLAY_IS_ANIMATING) {
            daemon_fail(rsp, "cannot destroy space because the display is in the middle of an animation.\n");
        } else if (result == SPACE_OP_ERROR_IN_MISSION_CONTROL) {
            daemon_fail(rsp, "cannot destroy space because mission-control is active.\n");
        } else if (result == SPACE_OP_ERROR_SCRIPTING_ADDITION) {
            daemon_fail(rsp, "cannot destroy space due to an error with the scripting-addition.\n");
        }
    } else if (token_equals(command, COMMAND_SPACE_BALANCE)) {
        if (!space_manager_balance_space(&g_space_manager, acting_sid)) {
            daemon_fail(rsp, "cannot balance a non-managed space.\n");
        }
    } else if (token_equals(command, COMMAND_SPACE_MIRROR)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_MIRROR_X)) {
            if (!space_manager_mirror_space(&g_space_manager, acting_sid, SPLIT_X)) {
                daemon_fail(rsp, "cannot mirror a non-managed space.\n");
            }
        } else if (token_equals(value, ARGUMENT_SPACE_MIRROR_Y)) {
            if (!space_manager_mirror_space(&g_space_manager, acting_sid, SPLIT_Y)) {
                daemon_fail(rsp, "cannot mirror a non-managed space.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_ROTATE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_ROTATE_90)) {
            if (!space_manager_rotate_space(&g_space_manager, acting_sid, 90)) {
                daemon_fail(rsp, "cannot rotate a non-managed space.\n");
            }
        } else if (token_equals(value, ARGUMENT_SPACE_ROTATE_180)) {
            if (!space_manager_rotate_space(&g_space_manager, acting_sid, 180)) {
                daemon_fail(rsp, "cannot rotate a non-managed space.\n");
            }
        } else if (token_equals(value, ARGUMENT_SPACE_ROTATE_270)) {
            if (!space_manager_rotate_space(&g_space_manager, acting_sid, 270)) {
                daemon_fail(rsp, "cannot rotate a non-managed space.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_PADDING)) {
        int t, b, l, r;
        char type[MAXLEN];
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_SPACE_PADDING, type, &t, &b, &l, &r) == 5)) {
            if (!space_manager_set_padding_for_space(&g_space_manager, acting_sid, parse_value_type(type), t, b, l, r)) {
                daemon_fail(rsp, "cannot set padding for a non-managed space.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_GAP)) {
        int gap;
        char type[MAXLEN];
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_SPACE_GAP, type, &gap) == 2)) {
            if (!space_manager_set_gap_for_space(&g_space_manager, acting_sid, parse_value_type(type), gap)) {
                daemon_fail(rsp, "cannot set gap for a non-managed space.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_SPACE_TOGGLE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_SPACE_TGL_PADDING)) {
            if (!space_manager_toggle_padding_for_space(&g_space_manager, acting_sid)) {
                daemon_fail(rsp, "cannot toggle padding for a non-managed space.\n");
            }
        } else if (token_equals(value, ARGUMENT_SPACE_TGL_GAP)) {
            if (!space_manager_toggle_gap_for_space(&g_space_manager, acting_sid)) {
                daemon_fail(rsp, "cannot toggle gap for a non-managed space.\n");
            }
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
        } else if (token_equals(value, ARGUMENT_SPACE_LAYOUT_STACK)) {
            if (space_is_user(acting_sid)) {
                space_manager_set_layout_for_space(&g_space_manager, acting_sid, VIEW_STACK);
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
    } else if (token_equals(command, COMMAND_SPACE_LABEL)) {
        char *label;
        if (parse_label(rsp, &message, LABEL_SPACE, &label)) {
            if (label) {
                space_manager_set_label_for_space(&g_space_manager, acting_sid, label);
            } else {
                if (!space_manager_remove_label_for_space(&g_space_manager, acting_sid)) {
                    daemon_fail(rsp, "the selected space was not associated with a label!\n");
                }
            }
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
            enum window_op_error result = window_manager_swap_window(&g_space_manager, &g_window_manager, acting_window, selector.window);
            if (result == WINDOW_OP_ERROR_INVALID_SRC_VIEW) {
                daemon_fail(rsp, "the acting window is not within a bsp space.\n");
            } else if (result == WINDOW_OP_ERROR_INVALID_DST_VIEW) {
                daemon_fail(rsp, "the selected window is not within a bsp space.\n");
            } else if (result == WINDOW_OP_ERROR_INVALID_SRC_NODE) {
                daemon_fail(rsp, "the acting window is not managed.\n");
            } else if (result == WINDOW_OP_ERROR_INVALID_DST_NODE) {
                daemon_fail(rsp, "the selected window is not managed.\n");
            } else if (result == WINDOW_OP_ERROR_SAME_STACK) {
                daemon_fail(rsp, "cannot swap a window with a window in the same stack.\n");
            } else if (result == WINDOW_OP_ERROR_SAME_WINDOW) {
                daemon_fail(rsp, "cannot swap a window with itself.\n");
            }
        }
    } else if (token_equals(command, COMMAND_WINDOW_WARP)) {
        struct selector selector = parse_window_selector(rsp, &message, acting_window);
        if (selector.did_parse && selector.window) {
            enum window_op_error result = window_manager_warp_window(&g_space_manager, &g_window_manager, acting_window, selector.window);
            if (result == WINDOW_OP_ERROR_INVALID_SRC_VIEW) {
                daemon_fail(rsp, "the acting window is not within a bsp space.\n");
            } else if (result == WINDOW_OP_ERROR_INVALID_DST_VIEW) {
                daemon_fail(rsp, "the selected window is not within a bsp space.\n");
            } else if (result == WINDOW_OP_ERROR_INVALID_SRC_NODE) {
                daemon_fail(rsp, "the acting window is not managed.\n");
            } else if (result == WINDOW_OP_ERROR_INVALID_DST_NODE) {
                daemon_fail(rsp, "the selected window is not managed.\n");
            } else if (result == WINDOW_OP_ERROR_SAME_STACK) {
                daemon_fail(rsp, "cannot warp a window with a window in the same stack.\n");
            } else if (result == WINDOW_OP_ERROR_SAME_WINDOW) {
                daemon_fail(rsp, "cannot warp a window onto itself.\n");
            }
        }
    } else if (token_equals(command, COMMAND_WINDOW_STACK)) {
        struct selector selector = parse_window_selector(rsp, &message, acting_window);
        if (selector.did_parse && selector.window) {
            enum window_op_error result = window_manager_stack_window(&g_space_manager, &g_window_manager, acting_window, selector.window);
            if (result == WINDOW_OP_ERROR_INVALID_SRC_NODE) {
                daemon_fail(rsp, "the acting window is not managed.\n");
            } else if (result == WINDOW_OP_ERROR_MAX_STACK) {
                daemon_fail(rsp, "cannot stack window, max capacity of %d reached.\n", NODE_MAX_WINDOW_COUNT);
            } else if (result == WINDOW_OP_ERROR_SAME_WINDOW) {
                daemon_fail(rsp, "cannot stack a window onto itself.\n");
            }
        }
    } else if (token_equals(command, COMMAND_WINDOW_INSERT)) {
        struct selector selector = parse_insert_selector(rsp, &message);
        if (selector.did_parse && selector.dir) {
            enum window_op_error result = window_manager_set_window_insertion(&g_space_manager, &g_window_manager, acting_window, selector.dir);
            if (result == WINDOW_OP_ERROR_INVALID_SRC_VIEW) {
                daemon_fail(rsp, "the acting window is not within a bsp space.\n");
            } else if (result == WINDOW_OP_ERROR_INVALID_SRC_NODE) {
                daemon_fail(rsp, "the acting window is not managed.\n");
            }
        }
    } else if (token_equals(command, COMMAND_WINDOW_GRID)) {
        unsigned r, c, x, y, w, h;
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_WINDOW_GRID, &r, &c, &x, &y, &w, &h) == 6)) {
            enum window_op_error result = window_manager_apply_grid(&g_space_manager, &g_window_manager, acting_window, r, c, x, y, w, h);
            if (result == WINDOW_OP_ERROR_INVALID_SRC_VIEW) {
                daemon_fail(rsp, "cannot apply grid layout to a managed window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_MOVE)) {
        float x, y;
        char type[MAXLEN];
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_WINDOW_MOVE, type, &x, &y) == 3)) {
            enum window_op_error result = window_manager_move_window_relative(&g_window_manager, acting_window, parse_value_type(type), x, y);
            if (result == WINDOW_OP_ERROR_INVALID_SRC_VIEW) {
                daemon_fail(rsp, "cannot move a managed window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_RESIZE)) {
        float w, h;
        char handle[MAXLEN];
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_WINDOW_RESIZE, handle, &w, &h) == 3)) {
            enum window_op_error result = window_manager_resize_window_relative(&g_window_manager, acting_window, parse_resize_handle(handle), w, h);
            if (result == WINDOW_OP_ERROR_INVALID_SRC_NODE) {
                daemon_fail(rsp, "cannot locate bsp node for the managed window.\n");
            } else if (result == WINDOW_OP_ERROR_INVALID_DST_NODE) {
                daemon_fail(rsp, "cannot locate a bsp node fence.\n");
            } else if (result == WINDOW_OP_ERROR_INVALID_OPERATION) {
                daemon_fail(rsp, "cannot use absolute resizing on a managed window.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_RATIO)) {
        float r;
        char type[MAXLEN];
        struct token value = get_token(&message);
        if ((sscanf(value.text, ARGUMENT_WINDOW_RATIO, type, &r) == 2)) {
            enum window_op_error result = window_manager_adjust_window_ratio(&g_window_manager, acting_window, parse_value_type(type), r);
            if (result == WINDOW_OP_ERROR_INVALID_SRC_VIEW) {
                daemon_fail(rsp, "cannot adjust ratio of a non-managed window.\n");
            } else if (result == WINDOW_OP_ERROR_INVALID_SRC_NODE) {
                daemon_fail(rsp, "cannot adjust ratio of a root node.\n");
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_MIN)) {
        enum window_op_error result = window_manager_minimize_window(acting_window);
        if (result == WINDOW_OP_ERROR_CANT_MINIMIZE) {
            daemon_fail(rsp, "window with id '%d' does not support the minimize operation.\n", acting_window->id);
        } else if (result == WINDOW_OP_ERROR_ALREADY_MINIMIZED) {
            daemon_fail(rsp, "window with id '%d' is already minimized.\n", acting_window->id);
        } else if (result == WINDOW_OP_ERROR_MINIMIZE_FAILED) {
            daemon_fail(rsp, "could not minimize window with id '%d'.\n", acting_window->id);
        }
    } else if (token_equals(command, COMMAND_WINDOW_DEMIN)) {
        enum window_op_error result = window_manager_deminimize_window(acting_window);
        if (result == WINDOW_OP_ERROR_NOT_MINIMIZED) {
            daemon_fail(rsp, "window with id '%d' is not minimized.\n", acting_window->id);
        } else if (result == WINDOW_OP_ERROR_DEMINIMIZE_FAILED) {
            daemon_fail(rsp, "could not deminimize window with id '%d'.\n", acting_window->id);
        }
    } else if (token_equals(command, COMMAND_WINDOW_CLOSE)) {
        if (!window_manager_close_window(acting_window)) {
            daemon_fail(rsp, "could not close window with id '%d'.\n", acting_window->id);
        }
    } else if (token_equals(command, COMMAND_WINDOW_LAYER)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_LAYER_BELOW)) {
            if (!window_manager_set_window_layer(acting_window, LAYER_BELOW)) {
                daemon_fail(rsp, "could not change layer of window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_LAYER_NORMAL)) {
            if (!window_manager_set_window_layer(acting_window, LAYER_NORMAL)) {
                daemon_fail(rsp, "could not change layer of window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
            }
        } else if (token_equals(value, ARGUMENT_WINDOW_LAYER_ABOVE)) {
            if (!window_manager_set_window_layer(acting_window, LAYER_ABOVE)) {
                daemon_fail(rsp, "could not change layer of window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_OPACITY)) {
        struct token_value value = token_to_value(get_token(&message), false);
        if (value.type == TOKEN_TYPE_FLOAT && in_range_ii(value.float_value, 0.0f, 1.0f)) {
            if (window_manager_set_opacity(&g_window_manager, acting_window, value.float_value)) {
                acting_window->opacity = value.float_value;
            } else {
                daemon_fail(rsp, "could not change opacity of window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
            }
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_WINDOW_TOGGLE)) {
        struct token value = get_token(&message);
        if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_FLOAT)) {
            window_manager_make_window_floating(&g_space_manager, &g_window_manager, acting_window, !acting_window->is_floating);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_ON_TOP)) {
            window_manager_toggle_window_topmost(acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_STICKY)) {
            window_manager_make_window_sticky(&g_space_manager, &g_window_manager, acting_window, !acting_window->is_sticky);
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
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_EXPOSE)) {
            window_manager_toggle_window_expose(&g_window_manager, acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_PIP)) {
            window_manager_toggle_window_pip(&g_space_manager, &g_window_manager, acting_window);
        } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_BORDER)) {
            window_manager_toggle_window_border(&g_window_manager, acting_window);
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
                window_manager_send_window_to_space(&g_space_manager, &g_window_manager, acting_window, sid, false);
            }
        }
    } else if (token_equals(command, COMMAND_WINDOW_SPACE)) {
        struct selector selector = parse_space_selector(rsp, &message, space_manager_active_space());
        if (selector.did_parse && selector.sid) {
            if (space_is_fullscreen(selector.sid)) {
                daemon_fail(rsp, "can not move window to a macOS fullscreen space!\n");
            } else {
                window_manager_send_window_to_space(&g_space_manager, &g_window_manager, acting_window, selector.sid, false);
            }
        }
    } else if (token_equals(command, COMMAND_WINDOW_SWITCH)) {
        uint64_t cur_sid = space_manager_active_space();

        struct selector selector = parse_space_selector(rsp, &message, cur_sid);
        if (selector.did_parse && selector.sid) {
            if (space_is_fullscreen(selector.sid)) {
                daemon_fail(rsp, "can not switch window to a macOS fullscreen space!\n");
            } else {
                window_manager_send_window_to_space(&g_space_manager, &g_window_manager, acting_window, selector.sid, false);
                space_manager_switch_to_space(selector.sid, cur_sid);
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
                    daemon_fail(rsp, "could not find window to retrieve display details.\n");
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
                        daemon_fail(rsp, "could not retrieve spaces for display.\n");
                    }
                } else {
                    daemon_fail(rsp, "could not locate the selected display.\n");
                }
            } else if (!space_manager_query_spaces_for_display(rsp, acting_did)) {
                daemon_fail(rsp, "could not retrieve spaces for display.\n");
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
            } else if (!space_manager_query_active_space(rsp)) {
                daemon_fail(rsp, "could not retrieve active space.\n");
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
            } else if (acting_window) {
                space_manager_query_spaces_for_window(rsp, acting_window);
            } else {
                daemon_fail(rsp, "could not find window to retrieve space details.\n");
            }
        } else if (token_is_valid(option)) {
            daemon_fail(rsp, "unknown option '%.*s' given to command '%.*s' for domain '%.*s'\n", option.length, option.text, command.length, command.text, domain.length, domain.text);
        } else if (!space_manager_query_spaces_for_displays(rsp)) {
            daemon_fail(rsp, "could not retrieve spaces for displays.\n");
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
                    window_manager_query_windows_for_spaces(rsp, &selector.sid, 1);
                } else {
                    daemon_fail(rsp, "could not locate the selected space.\n");
                }
            } else {
                window_manager_query_windows_for_spaces(rsp, &acting_sid, 1);
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
            } else if (acting_window) {
                window_serialize(rsp, acting_window);
                fprintf(rsp, "\n");
            } else {
                daemon_fail(rsp, "could not retrieve window details.\n");
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
        char *unsupported_exclusion = NULL;
        bool did_parse = true;
        bool has_filter = false;
        struct rule rule = {};

        struct token token = get_token(&message);
        while (token.text && token.length > 0) {
            char *key = NULL;
            char *value = NULL;
            bool exclusion = false;
            get_key_value_pair(token.text, &key, &value, &exclusion);

            if (!key || !value) {
                daemon_fail(rsp, "invalid key-value pair '%s'\n", token.text);
                did_parse = false;
                goto rnext;
            }

            if (string_equals(key, ARGUMENT_RULE_KEY_LABEL)) {
                if (exclusion) unsupported_exclusion = key;
                rule.label = string_copy(value);
            } else if (string_equals(key, ARGUMENT_RULE_KEY_APP)) {
                has_filter = true;
                rule.app = string_copy(value);
                rule.app_regex_exclude = exclusion;
                rule.app_regex_valid = regcomp(&rule.app_regex, value, REG_EXTENDED) == 0;
                if (!rule.app_regex_valid) {
                    daemon_fail(rsp, "invalid regex pattern '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_TITLE)) {
                has_filter = true;
                rule.title = string_copy(value);
                rule.title_regex_exclude = exclusion;
                rule.title_regex_valid = regcomp(&rule.title_regex, value, REG_EXTENDED) == 0;
                if (!rule.title_regex_valid) {
                    daemon_fail(rsp, "invalid regex pattern '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_DISPLAY)) {
                if (exclusion) unsupported_exclusion = key;

                if (value[0] == ARGUMENT_RULE_VALUE_SPACE) {
                    ++value;
                    rule.follow_space = true;
                }

                struct selector selector = parse_display_selector(rsp, &value, display_manager_active_display_id());
                if (selector.did_parse && selector.did) {
                    rule.did = selector.did;
                } else {
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_SPACE)) {
                if (exclusion) unsupported_exclusion = key;

                if (value[0] == ARGUMENT_RULE_VALUE_SPACE) {
                    ++value;
                    rule.follow_space = true;
                }

                struct selector selector = parse_space_selector(rsp, &value, space_manager_active_space());
                if (selector.did_parse && selector.sid) {
                    rule.sid = selector.sid;
                } else {
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_GRID)) {
                if (exclusion) unsupported_exclusion = key;

                if ((sscanf(value, ARGUMENT_RULE_VALUE_GRID,
                            &rule.grid[0], &rule.grid[1],
                            &rule.grid[2], &rule.grid[3],
                            &rule.grid[4], &rule.grid[5]) != 6)) {
                    daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_ALPHA)) {
                if (exclusion) unsupported_exclusion = key;

                if ((sscanf(value, "%f", &rule.alpha) != 1) || (!in_range_ei(rule.alpha, 0.0f, 1.0f))) {
                    daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_MANAGE)) {
                if (exclusion) unsupported_exclusion = key;

                if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    rule.manage = RULE_PROP_ON;
                } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    rule.manage = RULE_PROP_OFF;
                } else {
                    daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_STICKY)) {
                if (exclusion) unsupported_exclusion = key;

                if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    rule.sticky = RULE_PROP_ON;
                } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    rule.sticky = RULE_PROP_OFF;
                } else {
                    daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_MFF)) {
                if (exclusion) unsupported_exclusion = key;

                if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    rule.mff = RULE_PROP_ON;
                } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    rule.mff = RULE_PROP_OFF;
                } else {
                    daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_LAYER)) {
                if (exclusion) unsupported_exclusion = key;

                if (string_equals(value, ARGUMENT_WINDOW_LAYER_BELOW)) {
                    rule.layer = LAYER_BELOW;
                } else if (string_equals(value, ARGUMENT_WINDOW_LAYER_NORMAL)) {
                    rule.layer = LAYER_NORMAL;
                } else if (string_equals(value, ARGUMENT_WINDOW_LAYER_ABOVE)) {
                    rule.layer = LAYER_ABOVE;
                } else {
                    daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_BORDER)) {
                if (exclusion) unsupported_exclusion = key;

                if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    rule.border = RULE_PROP_ON;
                } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    rule.border = RULE_PROP_OFF;
                } else {
                    daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_RULE_KEY_FULLSCR)) {
                if (exclusion) unsupported_exclusion = key;

                if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    rule.fullscreen = RULE_PROP_ON;
                } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    rule.fullscreen = RULE_PROP_OFF;
                } else {
                    daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else {
                daemon_fail(rsp, "unknown key '%s'\n", key);
                did_parse = false;
            }

rnext:
            token = get_token(&message);
        }

        if (!has_filter) {
            daemon_fail(rsp, "missing required key-value pair 'app[!]=..' or 'title[!]=..'\n");
            did_parse = false;
        }

        if (unsupported_exclusion) {
            daemon_fail(rsp, "unsupported token '!' (exclusion) given for key '%s'\n", unsupported_exclusion);
            did_parse = false;
        }

        if (did_parse) {
            rule_add(&rule);
        } else {
            rule_destroy(&rule);
        }
    } else if (token_equals(command, COMMAND_RULE_REM)) {
        struct token_value value = token_to_value(get_token(&message), true);
        if (value.type == TOKEN_TYPE_INT) {
            if (!rule_remove_by_index(value.int_value)) {
                daemon_fail(rsp, "rule with index '%d' not found.\n", value.int_value);
            }
        } else if (value.type == TOKEN_TYPE_STRING) {
            if (!rule_remove(value.string_value)) {
                daemon_fail(rsp, "rule with label '%s' not found.\n", value.string_value);
            }
        } else {
            daemon_fail(rsp, "value '%.*s' is not a valid option for RULE_SEL\n", value.token.length, value.token.text);
        }
    } else if (token_equals(command, COMMAND_RULE_LS)) {
        window_manager_query_window_rules(rsp);
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static void handle_domain_signal(FILE *rsp, struct token domain, char *message)
{
    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_SIGNAL_ADD)) {
        char *unsupported_exclusion = NULL;
        bool did_parse = true;
        bool has_command = false;
        bool has_signal_type = false;
        enum event_type signal_type = EVENT_TYPE_UNKNOWN;
        struct signal signal = {};

        struct token token = get_token(&message);
        while (token.text && token.length > 0) {
            char *key = NULL;
            char *value = NULL;
            bool exclusion = false;
            get_key_value_pair(token.text, &key, &value, &exclusion);

            if (!key || !value) {
                daemon_fail(rsp, "invalid key-value pair '%s'\n", token.text);
                did_parse = false;
                goto snext;
            }

            if (string_equals(key, ARGUMENT_SIGNAL_KEY_LABEL)) {
                if (exclusion) unsupported_exclusion = key;
                signal.label = string_copy(value);
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_APP)) {
                signal.app = string_copy(value);
                signal.app_regex_exclude = exclusion;
                signal.app_regex_valid = regcomp(&signal.app_regex, value, REG_EXTENDED) == 0;
                if (!signal.app_regex_valid) {
                    daemon_fail(rsp, "invalid regex pattern '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_TITLE)) {
                signal.title = string_copy(value);
                signal.title_regex_exclude = exclusion;
                signal.title_regex_valid = regcomp(&signal.title_regex, value, REG_EXTENDED) == 0;
                if (!signal.title_regex_valid) {
                    daemon_fail(rsp, "invalid regex pattern '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_ACTION)) {
                if (exclusion) unsupported_exclusion = key;

                has_command = true;
                signal.command = string_copy(value);
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_EVENT)) {
                if (exclusion) unsupported_exclusion = key;

                has_signal_type = true;
                signal_type = event_signal_type_from_string(value);
                if (signal_type == EVENT_TYPE_UNKNOWN) {
                    daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else {
                daemon_fail(rsp, "unknown key '%s'\n", key);
                did_parse = false;
            }

snext:
            token = get_token(&message);
        }

        if (!has_signal_type) {
            daemon_fail(rsp, "missing required key-value pair 'event=..'\n");
            did_parse = false;
        }

        if (!has_command) {
            daemon_fail(rsp, "missing required key-value pair 'action=..'\n");
            did_parse = false;
        }

        if (unsupported_exclusion) {
            daemon_fail(rsp, "unsupported token '!' (exclusion) given for key '%s'\n", unsupported_exclusion);
            did_parse = false;
        }

        if (did_parse) {
            event_signal_add(signal_type, &signal);
        } else {
            event_signal_destroy(&signal);
        }
    } else if (token_equals(command, COMMAND_SIGNAL_REM)) {
        struct token_value value = token_to_value(get_token(&message), true);
        if (value.type == TOKEN_TYPE_INT) {
            if (!event_signal_remove_by_index(value.int_value)) {
                daemon_fail(rsp, "signal with index '%d' not found.\n", value.int_value);
            }
        } else if (value.type == TOKEN_TYPE_STRING) {
            if (!event_signal_remove(value.string_value)) {
                daemon_fail(rsp, "signal with label '%s' not found.\n", value.string_value);
            }
        } else {
            daemon_fail(rsp, "value '%.*s' is not a valid option for SIGNAL_SEL\n", value.token.length, value.token.text);
        }
    } else if (token_equals(command, COMMAND_SIGNAL_LS)) {
        event_signal_list(rsp);
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
    event_loop_post(&g_event_loop, DAEMON_MESSAGE, NULL, sockfd, NULL);
}

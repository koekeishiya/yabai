static struct {
    int sockfd;
    bool is_running;
    pthread_t thread;
} g_message_loop;

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
#define COMMAND_CONFIG_DISPLAY_ORDER         "display_arrangement_order"
#define COMMAND_CONFIG_WINDOW_ORIGIN         "window_origin_display"
#define COMMAND_CONFIG_WINDOW_PLACEMENT      "window_placement"
#define COMMAND_CONFIG_WINDOW_ZOOM_PERSIST   "window_zoom_persist"
#define COMMAND_CONFIG_OPACITY               "window_opacity"
#define COMMAND_CONFIG_OPACITY_DURATION      "window_opacity_duration"
#define COMMAND_CONFIG_ANIMATION_DURATION    "window_animation_duration"
#define COMMAND_CONFIG_ANIMATION_EASING      "window_animation_easing"
#define COMMAND_CONFIG_SHADOW                "window_shadow"
#define COMMAND_CONFIG_MENUBAR_OPACITY       "menubar_opacity"
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
#define COMMAND_CONFIG_SPLIT_TYPE            "split_type"
#define COMMAND_CONFIG_AUTO_BALANCE          "auto_balance"
#define COMMAND_CONFIG_MOUSE_MOD             "mouse_modifier"
#define COMMAND_CONFIG_MOUSE_ACTION1         "mouse_action1"
#define COMMAND_CONFIG_MOUSE_ACTION2         "mouse_action2"
#define COMMAND_CONFIG_MOUSE_DROP_ACTION     "mouse_drop_action"
#define COMMAND_CONFIG_EXTERNAL_BAR          "external_bar"

#define SELECTOR_CONFIG_SPACE                "--space"

#define ARGUMENT_CONFIG_FFM_AUTOFOCUS         "autofocus"
#define ARGUMENT_CONFIG_FFM_AUTORAISE         "autoraise"
#define ARGUMENT_CONFIG_DISPLAY_ORDER_DEFAULT "default"
#define ARGUMENT_CONFIG_DISPLAY_ORDER_X       "horizontal"
#define ARGUMENT_CONFIG_DISPLAY_ORDER_Y       "vertical"
#define ARGUMENT_CONFIG_WINDOW_ORIGIN_DEFAULT "default"
#define ARGUMENT_CONFIG_WINDOW_ORIGIN_FOCUSED "focused"
#define ARGUMENT_CONFIG_WINDOW_ORIGIN_CURSOR  "cursor"
#define ARGUMENT_CONFIG_WINDOW_PLACEMENT_FST  "first_child"
#define ARGUMENT_CONFIG_WINDOW_PLACEMENT_SND  "second_child"
#define ARGUMENT_CONFIG_SHADOW_FLT            "float"
#define ARGUMENT_CONFIG_LAYOUT_BSP            "bsp"
#define ARGUMENT_CONFIG_LAYOUT_STACK          "stack"
#define ARGUMENT_CONFIG_LAYOUT_FLOAT          "float"
#define ARGUMENT_CONFIG_SPLIT_TYPE_Y          "vertical"
#define ARGUMENT_CONFIG_SPLIT_TYPE_X          "horizontal"
#define ARGUMENT_CONFIG_SPLIT_TYPE_AUTO       "auto"
#define ARGUMENT_CONFIG_MOUSE_MOD_ALT         "alt"
#define ARGUMENT_CONFIG_MOUSE_MOD_SHIFT       "shift"
#define ARGUMENT_CONFIG_MOUSE_MOD_CMD         "cmd"
#define ARGUMENT_CONFIG_MOUSE_MOD_CTRL        "ctrl"
#define ARGUMENT_CONFIG_MOUSE_MOD_FN          "fn"
#define ARGUMENT_CONFIG_MOUSE_ACTION_MOVE     "move"
#define ARGUMENT_CONFIG_MOUSE_ACTION_RESIZE   "resize"
#define ARGUMENT_CONFIG_MOUSE_ACTION_SWAP     "swap"
#define ARGUMENT_CONFIG_MOUSE_ACTION_STACK    "stack"
#define ARGUMENT_CONFIG_EXTERNAL_BAR_MAIN     "main"
#define ARGUMENT_CONFIG_EXTERNAL_BAR_ALL      "all"
#define ARGUMENT_CONFIG_EXTERNAL_BAR          "%5[^:]:%d:%d"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN DISPLAY------------------------------- */
#define COMMAND_DISPLAY_FOCUS "--focus"
#define COMMAND_DISPLAY_SPACE "--space"
#define COMMAND_DISPLAY_LABEL "--label"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN SPACE--------------------------------- */
#define COMMAND_SPACE_FOCUS    "--focus"
#define COMMAND_SPACE_SWITCH   "--switch"
#define COMMAND_SPACE_CREATE   "--create"
#define COMMAND_SPACE_DESTROY  "--destroy"
#define COMMAND_SPACE_MOVE     "--move"
#define COMMAND_SPACE_SWAP     "--swap"
#define COMMAND_SPACE_DISPLAY  "--display"
#define COMMAND_SPACE_EQUALIZE "--equalize"
#define COMMAND_SPACE_BALANCE  "--balance"
#define COMMAND_SPACE_MIRROR   "--mirror"
#define COMMAND_SPACE_ROTATE   "--rotate"
#define COMMAND_SPACE_PADDING  "--padding"
#define COMMAND_SPACE_GAP      "--gap"
#define COMMAND_SPACE_TOGGLE   "--toggle"
#define COMMAND_SPACE_LAYOUT   "--layout"
#define COMMAND_SPACE_LABEL    "--label"

#define ARGUMENT_SPACE_AXIS_X       "x-axis"
#define ARGUMENT_SPACE_AXIS_Y       "y-axis"
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
#define COMMAND_WINDOW_FOCUS      "--focus"
#define COMMAND_WINDOW_CLOSE      "--close"
#define COMMAND_WINDOW_MINIMIZE   "--minimize"
#define COMMAND_WINDOW_DEMINIMIZE "--deminimize"
#define COMMAND_WINDOW_DISPLAY    "--display"
#define COMMAND_WINDOW_SPACE      "--space"
#define COMMAND_WINDOW_SWAP       "--swap"
#define COMMAND_WINDOW_WARP       "--warp"
#define COMMAND_WINDOW_STACK      "--stack"
#define COMMAND_WINDOW_INSERT     "--insert"
#define COMMAND_WINDOW_GRID       "--grid"
#define COMMAND_WINDOW_MOVE       "--move"
#define COMMAND_WINDOW_RESIZE     "--resize"
#define COMMAND_WINDOW_RATIO      "--ratio"
#define COMMAND_WINDOW_SUB_LAYER  "--sub-layer"
#define COMMAND_WINDOW_OPACITY    "--opacity"
#define COMMAND_WINDOW_RAISE      "--raise"
#define COMMAND_WINDOW_LOWER      "--lower"
#define COMMAND_WINDOW_TOGGLE     "--toggle"
#define COMMAND_WINDOW_SCRATCHPAD "--scratchpad"

#define ARGUMENT_WINDOW_SEL_LARGEST   "largest"
#define ARGUMENT_WINDOW_SEL_SMALLEST  "smallest"
#define ARGUMENT_WINDOW_SEL_SIBLING   "sibling"
#define ARGUMENT_WINDOW_SEL_FNEPHEW   "first_nephew"
#define ARGUMENT_WINDOW_SEL_SNEPHEW   "second_nephew"
#define ARGUMENT_WINDOW_SEL_UNCLE     "uncle"
#define ARGUMENT_WINDOW_SEL_FCOUSIN   "first_cousin"
#define ARGUMENT_WINDOW_SEL_SCOUSIN   "second_cousin"
#define ARGUMENT_WINDOW_GRID          "%d:%d:%d:%d:%d:%d"
#define ARGUMENT_WINDOW_MOVE          "%255[^:]:%f:%f"
#define ARGUMENT_WINDOW_RESIZE        "%255[^:]:%f:%f"
#define ARGUMENT_WINDOW_RATIO         "%255[^:]:%f"
#define ARGUMENT_WINDOW_LAYER_BELOW   "below"
#define ARGUMENT_WINDOW_LAYER_NORMAL  "normal"
#define ARGUMENT_WINDOW_LAYER_ABOVE   "above"
#define ARGUMENT_WINDOW_LAYER_AUTO    "auto"
#define ARGUMENT_WINDOW_TOGGLE_FLOAT  "float"
#define ARGUMENT_WINDOW_TOGGLE_STICKY "sticky"
#define ARGUMENT_WINDOW_TOGGLE_SHADOW "shadow"
#define ARGUMENT_WINDOW_TOGGLE_SPLIT  "split"
#define ARGUMENT_WINDOW_TOGGLE_PARENT "zoom-parent"
#define ARGUMENT_WINDOW_TOGGLE_FULLSC "zoom-fullscreen"
#define ARGUMENT_WINDOW_TOGGLE_NATIVE "native-fullscreen"
#define ARGUMENT_WINDOW_TOGGLE_EXPOSE "expose"
#define ARGUMENT_WINDOW_TOGGLE_PIP    "pip"

#define ARGUMENT_WINDOW_SCRATCHPAD_RECOVER "recover"
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
#define COMMAND_RULE_ADD     "--add"
#define COMMAND_RULE_REM     "--remove"
#define COMMAND_RULE_APPLY   "--apply"
#define COMMAND_RULE_LS      "--list"

#define ARGUMENT_RULE_ONE_SHOT       "--one-shot"
#define ARGUMENT_RULE_KEY_APP        "app"
#define ARGUMENT_RULE_KEY_TITLE      "title"
#define ARGUMENT_RULE_KEY_ROLE       "role"
#define ARGUMENT_RULE_KEY_SUBROLE    "subrole"
#define ARGUMENT_RULE_KEY_DISPLAY    "display"
#define ARGUMENT_RULE_KEY_SPACE      "space"
#define ARGUMENT_RULE_KEY_OPACITY    "opacity"
#define ARGUMENT_RULE_KEY_MANAGE     "manage"
#define ARGUMENT_RULE_KEY_STICKY     "sticky"
#define ARGUMENT_RULE_KEY_MFF        "mouse_follows_focus"
#define ARGUMENT_RULE_KEY_SUB_LAYER  "sub-layer"
#define ARGUMENT_RULE_KEY_FULLSCR    "native-fullscreen"
#define ARGUMENT_RULE_KEY_GRID       "grid"
#define ARGUMENT_RULE_KEY_LABEL      "label"
#define ARGUMENT_RULE_KEY_SCRATCHPAD "scratchpad"

#define ARGUMENT_RULE_VALUE_SPACE '^'
#define ARGUMENT_RULE_VALUE_GRID  "%d:%d:%d:%d:%d:%d"
/* ----------------------------------------------------------------------------- */

/* --------------------------------DOMAIN SIGNAL-------------------------------- */
#define COMMAND_SIGNAL_ADD "--add"
#define COMMAND_SIGNAL_REM "--remove"
#define COMMAND_SIGNAL_LS  "--list"

#define ARGUMENT_SIGNAL_KEY_APP      "app"
#define ARGUMENT_SIGNAL_KEY_TITLE    "title"
#define ARGUMENT_SIGNAL_KEY_ACTIVE   "active"
#define ARGUMENT_SIGNAL_KEY_EVENT    "event"
#define ARGUMENT_SIGNAL_KEY_ACTION   "action"
#define ARGUMENT_SIGNAL_KEY_LABEL    "label"

#define ARGUMENT_SIGNAL_VALUE_YES    "yes"
#define ARGUMENT_SIGNAL_VALUE_NO     "no"
/* ----------------------------------------------------------------------------- */

/* --------------------------------COMMON ARGUMENTS----------------------------- */
#define ARGUMENT_COMMON_VAL_ON              "on"
#define ARGUMENT_COMMON_VAL_OFF             "off"
#define ARGUMENT_COMMON_SEL_PREV            "prev"
#define ARGUMENT_COMMON_SEL_NEXT            "next"
#define ARGUMENT_COMMON_SEL_FIRST           "first"
#define ARGUMENT_COMMON_SEL_LAST            "last"
#define ARGUMENT_COMMON_SEL_RECENT          "recent"
#define ARGUMENT_COMMON_SEL_NORTH           "north"
#define ARGUMENT_COMMON_SEL_EAST            "east"
#define ARGUMENT_COMMON_SEL_SOUTH           "south"
#define ARGUMENT_COMMON_SEL_WEST            "west"
#define ARGUMENT_COMMON_SEL_MOUSE           "mouse"
#define ARGUMENT_COMMON_SEL_STACK           "stack"
#define ARGUMENT_COMMON_SEL_STACK_PREV      "stack.prev"
#define ARGUMENT_COMMON_SEL_STACK_NEXT      "stack.next"
#define ARGUMENT_COMMON_SEL_STACK_FIRST     "stack.first"
#define ARGUMENT_COMMON_SEL_STACK_SECOND    "stack.second"
#define ARGUMENT_COMMON_SEL_STACK_THIRD     "stack.third"
#define ARGUMENT_COMMON_SEL_STACK_FOURTH    "stack.fourth"
#define ARGUMENT_COMMON_SEL_STACK_FIFTH     "stack.fifth"
#define ARGUMENT_COMMON_SEL_STACK_SIXTH     "stack.sixth"
#define ARGUMENT_COMMON_SEL_STACK_SEVENTH   "stack.seventh"
#define ARGUMENT_COMMON_SEL_STACK_EIGHTH    "stack.eighth"
#define ARGUMENT_COMMON_SEL_STACK_NINTH     "stack.ninth"
#define ARGUMENT_COMMON_SEL_STACK_LAST      "stack.last"
#define ARGUMENT_COMMON_SEL_STACK_RECENT    "stack.recent"
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

static bool token_prefix(struct token token, char *match)
{
    char *at = match;
    for (int i = 0; i < token.length; ++i, ++at) {
        if (*at == 0)             return true;
        if (token.text[i] != *at) return false;
    }
    return *at == 0;
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

static inline bool token_is_valid(struct token token)
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
    char *end = NULL;
    float v = strtof(token.text, &end);

    if (!end || *end) {
        *value = 0.0f;
        return false;
    } else {
        *value = v;
        return true;
    }
}

static struct token_value token_to_value(struct token token)
{
    struct token_value value = { .token = token, .type = TOKEN_TYPE_INVALID };

    if (token_is_valid(token)) {
        if (token_is_positive_integer(token, &value.int_value)) {
            value.type = TOKEN_TYPE_INT;
        } else if (token_is_hexadecimal(token, &value.u32_value)) {
            value.type = TOKEN_TYPE_U32;
        } else if (token_is_float(token, &value.float_value)) {
            value.type = TOKEN_TYPE_FLOAT;
        } else if ((value.string_value = token.text)) {
            value.type = TOKEN_TYPE_STRING;
        } else {
            value.type = TOKEN_TYPE_UNKNOWN;
        }
    }

    return value;
}

static inline void daemon_fail(FILE *rsp, char *fmt, ...)
{
    if (!rsp) return;

    va_list ap;
    va_start(ap, fmt);
    fprintf(rsp, FAILURE_MESSAGE);
    vfprintf(rsp, fmt, ap);
    va_end(ap);
}

__unused static inline void daemon_deprecated(FILE *rsp, char *fmt, ...)
{
    if (!rsp) return;

    va_list ap;
    va_start(ap, fmt);
    fprintf(rsp, "deprecation warning: ");
    vfprintf(rsp, fmt, ap);
    va_end(ap);
}

static void parse_key_value_pair(char *token, char **key, char **value, bool *exclusion)
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

enum label_type
{
    LABEL_DISPLAY,
    LABEL_SPACE,
    LABEL_WINDOW
};

static char *reserved_display_identifiers[] =
{
    ARGUMENT_COMMON_SEL_NORTH,
    ARGUMENT_COMMON_SEL_EAST,
    ARGUMENT_COMMON_SEL_SOUTH,
    ARGUMENT_COMMON_SEL_WEST,
    ARGUMENT_COMMON_SEL_PREV,
    ARGUMENT_COMMON_SEL_NEXT,
    ARGUMENT_COMMON_SEL_FIRST,
    ARGUMENT_COMMON_SEL_LAST,
    ARGUMENT_COMMON_SEL_RECENT,
    ARGUMENT_COMMON_SEL_MOUSE
};

static char *reserved_space_identifiers[] =
{
    ARGUMENT_COMMON_SEL_PREV,
    ARGUMENT_COMMON_SEL_NEXT,
    ARGUMENT_COMMON_SEL_FIRST,
    ARGUMENT_COMMON_SEL_LAST,
    ARGUMENT_COMMON_SEL_RECENT,
    ARGUMENT_COMMON_SEL_MOUSE
};

static char *reserved_window_identifiers[] =
{
    ARGUMENT_WINDOW_TOGGLE_FLOAT,
    ARGUMENT_WINDOW_TOGGLE_STICKY,
    ARGUMENT_WINDOW_TOGGLE_SHADOW,
    ARGUMENT_WINDOW_TOGGLE_SPLIT,
    ARGUMENT_WINDOW_TOGGLE_PARENT,
    ARGUMENT_WINDOW_TOGGLE_FULLSC,
    ARGUMENT_WINDOW_TOGGLE_NATIVE,
    ARGUMENT_WINDOW_TOGGLE_EXPOSE,
    ARGUMENT_WINDOW_TOGGLE_PIP,
    ARGUMENT_WINDOW_SCRATCHPAD_RECOVER
};

static bool parse_label(FILE *rsp, struct token token, enum label_type type, char **label)
{
    struct token_value value = token_to_value(token);

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
    case LABEL_DISPLAY: {
        for (int i = 0; i < array_count(reserved_display_identifiers); ++i) {
            if (token_equals(token, reserved_display_identifiers[i])) {
                daemon_fail(rsp, "'%.*s' is a reserved keyword and cannot be used as a label.\n", token.length, token.text);
                return false;
            }
        }
    } break;
    case LABEL_SPACE: {
        for (int i = 0; i < array_count(reserved_space_identifiers); ++i) {
            if (token_equals(token, reserved_space_identifiers[i])) {
                daemon_fail(rsp, "'%.*s' is a reserved keyword and cannot be used as a label.\n", token.length, token.text);
                return false;
            }
        }
    } break;
    case LABEL_WINDOW: {
        for (int i = 0; i < array_count(reserved_window_identifiers); ++i) {
            if (token_equals(token, reserved_window_identifiers[i])) {
                daemon_fail(rsp, "'%.*s' is a reserved keyword and cannot be used as a scratchpad.\n", token.length, token.text);
                return false;
            }
        }
    } break;
    }

    *label = malloc(token.length + 1);
    if (!(*label)) return false;

    memcpy(*label, token.text, token.length);
    (*label)[token.length] = '\0';

    return true;
}

struct properties
{
    struct token token;
    bool did_parse;
    bool did_error;
    uint64_t flags;
};

static inline bool parse_property(struct properties *properties, char *property, uint64_t *property_val, char **property_str, int property_count)
{
    for (int i = 0; i < property_count; ++i) {
        if (string_equals(property, property_str[i])) {
            properties->flags |= property_val[i];
            return true;
        }
    }

    return false;
}

static struct properties parse_properties(FILE *rsp, struct token token, uint64_t *property_val, char **property_str, int property_count)
{
    struct properties result = { .token = token, .did_parse = true, .did_error = false };

    if (token_is_valid(token) && !token_prefix(token, "--")) {
        for (int i = 0, cursor = 0; i < token.length; ++i) {
            if (i+1 == token.length) {
                if (!parse_property(&result, token.text+cursor, property_val, property_str, property_count)) {
                    daemon_fail(rsp, "'%.*s' is not a valid property.\n", i-cursor+1, token.text+cursor);
                    result.did_error = true;
                }
            } else if (token.text[i] == ',') {
                token.text[i] = '\0';

                if (!parse_property(&result, token.text+cursor, property_val, property_str, property_count)) {
                    daemon_fail(rsp, "'%.*s' is not a valid property.\n", i-cursor+1, token.text+cursor);
                    result.did_error = true;
                }

                cursor = i+1;
            }
        }
    } else {
        result.did_parse = false;
    }

    return result;
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

static struct selector parse_display_selector(FILE *rsp, char **message, uint32_t acting_did, bool optional)
{
    TIME_FUNCTION;

    struct selector result = { .token = get_token(message), .did_parse = true };

    struct token_value value = token_to_value(result.token);
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
            struct display_label *display_label = display_manager_get_display_for_label(&g_display_manager, value.string_value);
            if (display_label) {
                result.did_parse = true;
                result.did = display_label->did;
            } else {
                result.did_parse = false;
                daemon_fail(rsp, "value '%.*s' is not a valid option for DISPLAY_SEL\n", result.token.length, result.token.text);
            }
        }
    } else if (value.type == TOKEN_TYPE_INVALID) {
        result.did_parse = false;
        if (!optional) daemon_fail(rsp, "value '%.*s' is not a valid option for DISPLAY_SEL\n", result.token.length, result.token.text);
    } else {
        result.did_parse = false;
        daemon_fail(rsp, "value '%.*s' is not a valid option for DISPLAY_SEL\n", result.token.length, result.token.text);
    }

    return result;
}

static struct selector parse_space_selector(FILE *rsp, char **message, uint64_t acting_sid, bool optional)
{
    TIME_FUNCTION;

    struct selector result = { .token = get_token(message), .did_parse = true };

    struct token_value value = token_to_value(result.token);
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
    } else if (value.type == TOKEN_TYPE_INVALID) {
        result.did_parse = false;
        if (!optional) daemon_fail(rsp, "value '%.*s' is not a valid option for SPACE_SEL\n", result.token.length, result.token.text);
    } else {
        result.did_parse = false;
        daemon_fail(rsp, "value '%.*s' is not a valid option for SPACE_SEL\n", result.token.length, result.token.text);
    }

    return result;
}

static struct selector parse_window_selector(FILE *rsp, char **message, struct window *acting_window, bool optional)
{
    TIME_FUNCTION;

    struct selector result = { .token = get_token(message), .did_parse = true };

    struct token_value value = token_to_value(result.token);
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
        } else if (token_equals(result.token, ARGUMENT_WINDOW_SEL_SIBLING)) {
            if (acting_window) {
                struct window *sibling_window = window_manager_find_sibling_for_managed_window(&g_window_manager, acting_window);
                if (sibling_window) {
                    result.window = sibling_window;
                } else {
                    daemon_fail(rsp, "could not locate sibling of window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_WINDOW_SEL_FNEPHEW)) {
            if (acting_window) {
                struct window *nephew_window = window_manager_find_first_nephew_for_managed_window(&g_window_manager, acting_window);
                if (nephew_window) {
                    result.window = nephew_window;
                } else {
                    daemon_fail(rsp, "could not locate first nephew of window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_WINDOW_SEL_SNEPHEW)) {
            if (acting_window) {
                struct window *nephew_window = window_manager_find_second_nephew_for_managed_window(&g_window_manager, acting_window);
                if (nephew_window) {
                    result.window = nephew_window;
                } else {
                    daemon_fail(rsp, "could not locate second nephew of window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_WINDOW_SEL_UNCLE)) {
            if (acting_window) {
                struct window *uncle_window = window_manager_find_uncle_for_managed_window(&g_window_manager, acting_window);
                if (uncle_window) {
                    result.window = uncle_window;
                } else {
                    daemon_fail(rsp, "could not locate uncle of window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_WINDOW_SEL_FCOUSIN)) {
            if (acting_window) {
                struct window *cousin_window = window_manager_find_first_cousin_for_managed_window(&g_window_manager, acting_window);
                if (cousin_window) {
                    result.window = cousin_window;
                } else {
                    daemon_fail(rsp, "could not locate first cousin of window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_WINDOW_SEL_SCOUSIN)) {
            if (acting_window) {
                struct window *cousin_window = window_manager_find_second_cousin_for_managed_window(&g_window_manager, acting_window);
                if (cousin_window) {
                    result.window = cousin_window;
                } else {
                    daemon_fail(rsp, "could not locate second cousin of window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
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
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_SECOND)) {
            if (acting_window) {
                struct window *second_window = window_manager_find_nth_window_in_stack(&g_space_manager, &g_window_manager, acting_window, 1);
                if (second_window) {
                    result.window = second_window;
                } else {
                    daemon_fail(rsp, "could not locate the first stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_THIRD)) {
            if (acting_window) {
                struct window *third_window = window_manager_find_nth_window_in_stack(&g_space_manager, &g_window_manager, acting_window, 2);
                if (third_window) {
                    result.window = third_window;
                } else {
                    daemon_fail(rsp, "could not locate the first stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_FOURTH)) {
            if (acting_window) {
                struct window *fourth_window = window_manager_find_nth_window_in_stack(&g_space_manager, &g_window_manager, acting_window, 3);
                if (fourth_window) {
                    result.window = fourth_window;
                } else {
                    daemon_fail(rsp, "could not locate the first stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_FIFTH)) {
            if (acting_window) {
                struct window *fifth_window = window_manager_find_nth_window_in_stack(&g_space_manager, &g_window_manager, acting_window, 4);
                if (fifth_window) {
                    result.window = fifth_window;
                } else {
                    daemon_fail(rsp, "could not locate the first stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_SIXTH)) {
            if (acting_window) {
                struct window *sixth_window = window_manager_find_nth_window_in_stack(&g_space_manager, &g_window_manager, acting_window, 5);
                if (sixth_window) {
                    result.window = sixth_window;
                } else {
                    daemon_fail(rsp, "could not locate the first stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_SEVENTH)) {
            if (acting_window) {
                struct window *seventh_window = window_manager_find_nth_window_in_stack(&g_space_manager, &g_window_manager, acting_window, 6);
                if (seventh_window) {
                    result.window = seventh_window;
                } else {
                    daemon_fail(rsp, "could not locate the first stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_EIGHTH)) {
            if (acting_window) {
                struct window *eighth_window = window_manager_find_nth_window_in_stack(&g_space_manager, &g_window_manager, acting_window, 7);
                if (eighth_window) {
                    result.window = eighth_window;
                } else {
                    daemon_fail(rsp, "could not locate the first stacked window.\n");
                }
            } else {
                daemon_fail(rsp, "could not locate the selected window.\n");
            }
        } else if (token_equals(result.token, ARGUMENT_COMMON_SEL_STACK_NINTH)) {
            if (acting_window) {
                struct window *ninth_window = window_manager_find_nth_window_in_stack(&g_space_manager, &g_window_manager, acting_window, 8);
                if (ninth_window) {
                    result.window = ninth_window;
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
    } else if (value.type == TOKEN_TYPE_INVALID) {
        result.did_parse = false;
        if (!optional) daemon_fail(rsp, "value '%.*s' is not a valid option for WINDOW_SEL\n", result.token.length, result.token.text);
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
    TIME_FUNCTION;

    uint64_t sel_sid = 0;
    struct token selector = get_token(&message);
    struct token command  = selector;

    bool found_selector = token_equals(selector, SELECTOR_CONFIG_SPACE);
    if (found_selector) {
        struct selector space_selector = parse_space_selector(rsp, &message, 0, false);
        if (!space_selector.did_parse || !space_selector.sid) return;

        sel_sid = space_selector.sid;
        command = get_token(&message);
    }

    while (token_is_valid(command)) {
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
                window_manager_set_focus_follows_mouse(&g_window_manager, FFM_DISABLED);
            } else if (token_equals(value, ARGUMENT_CONFIG_FFM_AUTOFOCUS)) {
                window_manager_set_focus_follows_mouse(&g_window_manager, FFM_AUTOFOCUS);
            } else if (token_equals(value, ARGUMENT_CONFIG_FFM_AUTORAISE)) {
                window_manager_set_focus_follows_mouse(&g_window_manager, FFM_AUTORAISE);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_CONFIG_DISPLAY_ORDER)) {
            struct token value = get_token(&message);
            if (!token_is_valid(value)) {
                fprintf(rsp, "%s\n", display_arrangement_order_str[g_display_manager.order]);
            } else if (token_equals(value, ARGUMENT_CONFIG_DISPLAY_ORDER_DEFAULT)) {
                g_display_manager.order = DISPLAY_ARRANGEMENT_ORDER_DEFAULT;
            } else if (token_equals(value, ARGUMENT_CONFIG_DISPLAY_ORDER_X)) {
                g_display_manager.order = DISPLAY_ARRANGEMENT_ORDER_X;
            } else if (token_equals(value, ARGUMENT_CONFIG_DISPLAY_ORDER_Y)) {
                g_display_manager.order = DISPLAY_ARRANGEMENT_ORDER_Y;
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_CONFIG_WINDOW_ORIGIN)) {
            struct token value = get_token(&message);
            if (!token_is_valid(value)) {
                fprintf(rsp, "%s\n", window_origin_mode_str[g_window_manager.window_origin_mode]);
            } else if (token_equals(value, ARGUMENT_CONFIG_WINDOW_ORIGIN_DEFAULT)) {
                g_window_manager.window_origin_mode = WINDOW_ORIGIN_DEFAULT;
            } else if (token_equals(value, ARGUMENT_CONFIG_WINDOW_ORIGIN_FOCUSED)) {
                g_window_manager.window_origin_mode = WINDOW_ORIGIN_FOCUSED;
            } else if (token_equals(value, ARGUMENT_CONFIG_WINDOW_ORIGIN_CURSOR)) {
                g_window_manager.window_origin_mode = WINDOW_ORIGIN_CURSOR;
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
        } else if (token_equals(command, COMMAND_CONFIG_WINDOW_ZOOM_PERSIST)) {
            struct token value = get_token(&message);
            if (!token_is_valid(value)) {
                fprintf(rsp, "%s\n", bool_str[g_space_manager.window_zoom_persist]);
            } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                g_space_manager.window_zoom_persist = false;
            } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                g_space_manager.window_zoom_persist = true;
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
            struct token_value value = token_to_value(get_token(&message));
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%f\n", g_window_manager.window_opacity_duration);
            } else if (value.type == TOKEN_TYPE_FLOAT) {
                g_window_manager.window_opacity_duration = value.float_value;
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_CONFIG_ANIMATION_DURATION)) {
            struct token_value value = token_to_value(get_token(&message));
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%f\n", g_window_manager.window_animation_duration);
            } else if (value.type == TOKEN_TYPE_FLOAT) {
                if (CGPreflightScreenCaptureAccess()) {
                    if (scripting_addition_is_sip_friendly()) {
                        g_window_manager.window_animation_duration = value.float_value;
                    } else {
                        daemon_fail(rsp, "command '%.*s' for domain '%.*s' requires System Integrity Protection to be partially disabled! ignoring request..\n", command.length, command.text, domain.length, domain.text);
                    }
                } else {
                    daemon_fail(rsp, "command '%.*s' for domain '%.*s' requires Screen Recording permissions! ignoring request..\n", command.length, command.text, domain.length, domain.text);
                    CGRequestScreenCaptureAccess();
                }
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_CONFIG_ANIMATION_EASING)) {
            struct token value = get_token(&message);
            if (!token_is_valid(value)) {
                fprintf(rsp, "%s\n", animation_easing_type_str[g_window_manager.window_animation_easing]);
            } else {
                bool match = false;
                for (int i = 0; i < EASING_TYPE_COUNT; ++i) {
                    if (token_equals(value, animation_easing_type_str[i])) {
                        g_window_manager.window_animation_easing = i;
                        match = true;
                        break;
                    }
                }
                if (!match) daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
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
        } else if (token_equals(command, COMMAND_CONFIG_MENUBAR_OPACITY)) {
            struct token_value value = token_to_value(get_token(&message));
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%.4f\n", g_window_manager.menubar_opacity);
            } else if (value.type == TOKEN_TYPE_FLOAT && in_range_ii(value.float_value, 0.0f, 1.0f)) {
                window_manager_set_menubar_opacity(&g_window_manager, value.float_value);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_CONFIG_ACTIVE_WINDOW_OPACITY)) {
            struct token_value value = token_to_value(get_token(&message));
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%.4f\n", g_window_manager.active_window_opacity);
            } else if (value.type == TOKEN_TYPE_FLOAT && in_range_ei(value.float_value, 0.0f, 1.0f)) {
                window_manager_set_active_window_opacity(&g_window_manager, value.float_value);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_CONFIG_NORMAL_WINDOW_OPACITY)) {
            struct token_value value = token_to_value(get_token(&message));
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%.4f\n", g_window_manager.normal_window_opacity);
            } else if (value.type == TOKEN_TYPE_FLOAT && in_range_ei(value.float_value, 0.0f, 1.0f)) {
                window_manager_set_normal_window_opacity(&g_window_manager, value.float_value);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_CONFIG_INSERT_FEEDBACK_COLOR)) {
            struct token_value value = token_to_value(get_token(&message));
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "0x%x\n", g_window_manager.insert_feedback_color.p);
            } else if (value.type == TOKEN_TYPE_U32 && value.u32_value) {
                g_window_manager.insert_feedback_color = rgba_color_from_hex(value.u32_value);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_CONFIG_TOP_PADDING)) {
            struct token_value value = token_to_value(get_token(&message));
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (value.type == TOKEN_TYPE_INVALID) {
                    fprintf(rsp, "%d\n", view->top_padding);
                } else if (value.type == TOKEN_TYPE_INT) {
                    view_set_flag(view, VIEW_TOP_PADDING);
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
            struct token_value value = token_to_value(get_token(&message));
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (value.type == TOKEN_TYPE_INVALID) {
                    fprintf(rsp, "%d\n", view->bottom_padding);
                } else if (value.type == TOKEN_TYPE_INT) {
                    view_set_flag(view, VIEW_BOTTOM_PADDING);
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
            struct token_value value = token_to_value(get_token(&message));
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (value.type == TOKEN_TYPE_INVALID) {
                    fprintf(rsp, "%d\n", view->left_padding);
                } else if (value.type == TOKEN_TYPE_INT) {
                    view_set_flag(view, VIEW_LEFT_PADDING);
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
            struct token_value value = token_to_value(get_token(&message));
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (value.type == TOKEN_TYPE_INVALID) {
                    fprintf(rsp, "%d\n", view->right_padding);
                } else if (value.type == TOKEN_TYPE_INT) {
                    view_set_flag(view, VIEW_RIGHT_PADDING);
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
            struct token_value value = token_to_value(get_token(&message));
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (value.type == TOKEN_TYPE_INVALID) {
                    fprintf(rsp, "%d\n", view->window_gap);
                } else if (value.type == TOKEN_TYPE_INT) {
                    view_set_flag(view, VIEW_WINDOW_GAP);
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
                        view_set_flag(view, VIEW_LAYOUT);
                        view->layout = VIEW_BSP;
                        view_clear(view);
                        window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, sel_sid);
                    } else {
                        daemon_fail(rsp, "cannot set layout for a macOS fullscreen space!\n");
                    }
                } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_STACK)) {
                    if (space_is_user(sel_sid)) {
                        view_set_flag(view, VIEW_LAYOUT);
                        view->layout = VIEW_STACK;
                        view_clear(view);
                        window_manager_validate_and_check_for_windows_on_space(&g_space_manager, &g_window_manager, sel_sid);
                    } else {
                        daemon_fail(rsp, "cannot set layout for a macOS fullscreen space!\n");
                    }
                } else if (token_equals(value, ARGUMENT_CONFIG_LAYOUT_FLOAT)) {
                    if (space_is_user(sel_sid)) {
                        view_set_flag(view, VIEW_LAYOUT);
                        view->layout = VIEW_FLOAT;
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
            struct token_value value = token_to_value(get_token(&message));
            if (value.type == TOKEN_TYPE_INVALID) {
                fprintf(rsp, "%.4f\n", g_space_manager.split_ratio);
            } else if (value.type == TOKEN_TYPE_FLOAT && in_range_ii(value.float_value, 0.1f, 0.9f)) {
                g_space_manager.split_ratio = value.float_value;
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_CONFIG_SPLIT_TYPE)) {
            struct token value = get_token(&message);
            if (!token_is_valid(value)) {
                fprintf(rsp, "%s\n", window_node_split_str[g_space_manager.split_type]);
            } else if (token_equals(value, ARGUMENT_CONFIG_SPLIT_TYPE_Y)) {
                g_space_manager.split_type = SPLIT_Y;
            } else if (token_equals(value, ARGUMENT_CONFIG_SPLIT_TYPE_X)) {
                g_space_manager.split_type = SPLIT_X;
            } else if (token_equals(value, ARGUMENT_CONFIG_SPLIT_TYPE_AUTO)) {
                g_space_manager.split_type = SPLIT_AUTO;
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_CONFIG_AUTO_BALANCE)) {
            struct token value = get_token(&message);
            if (sel_sid) {
                struct view *view = space_manager_find_view(&g_space_manager, sel_sid);
                if (!token_is_valid(value)) {
                    fprintf(rsp, "%s\n", bool_str[view->auto_balance]);
                } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    view_set_flag(view, VIEW_AUTO_BALANCE);
                    view->auto_balance = false;
                } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    view_set_flag(view, VIEW_AUTO_BALANCE);
                    view->auto_balance = true;
                } else {
                    daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
                }
            } else {
                if (!token_is_valid(value)) {
                    fprintf(rsp, "%s\n", bool_str[g_space_manager.auto_balance]);
                } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                    space_manager_set_auto_balance_for_all_spaces(&g_space_manager, false);
                } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                    space_manager_set_auto_balance_for_all_spaces(&g_space_manager, true);
                } else {
                    daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
                }
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

        command = get_token(&message);
    }
}

static void handle_domain_display(FILE *rsp, struct token domain, char *message)
{
    TIME_FUNCTION;

    struct token command;
    uint32_t acting_did = display_manager_active_display_id();
    struct selector selector = parse_display_selector(NULL, &message, acting_did, true);

    if (selector.did_parse) {
        acting_did = selector.did;
        command = get_token(&message);
    } else {
        command = selector.token;
    }

    if (!acting_did) {
        daemon_fail(rsp, "could not locate the display to act on!\n");
        return;
    }

    if (token_equals(command, COMMAND_DISPLAY_FOCUS)) {
        struct selector selector = parse_display_selector(rsp, &message, acting_did, false);
        if (selector.did_parse && selector.did) {
            display_manager_focus_display(selector.did, display_space_id(selector.did));
        }
    } else if (token_equals(command, COMMAND_DISPLAY_SPACE)) {
        struct selector selector = parse_space_selector(rsp, &message, display_space_id(acting_did), false);
        if (selector.did_parse && selector.sid) {
            enum space_op_error result = display_manager_focus_space(acting_did, selector.sid);
            if (result == SPACE_OP_ERROR_SAME_DISPLAY) {
                daemon_fail(rsp, "acting display does not contain the given space.\n");
            } else if (result == SPACE_OP_ERROR_DISPLAY_IS_ANIMATING) {
                daemon_fail(rsp, "cannot focus space because the display is in the middle of an animation.\n");
            } else if (result == SPACE_OP_ERROR_IN_MISSION_CONTROL) {
                daemon_fail(rsp, "cannot focus space because mission-control is active.\n");
            } else if (result == SPACE_OP_ERROR_SCRIPTING_ADDITION) {
                daemon_fail(rsp, "cannot focus space due to an error with the scripting-addition.\n");
            }
        }
    } else if (token_equals(command, COMMAND_DISPLAY_LABEL)) {
        char *label;
        if (parse_label(rsp, get_token(&message), LABEL_DISPLAY, &label)) {
            if (label) {
                display_manager_set_label_for_display(&g_display_manager, acting_did, label);
            } else {
                if (!display_manager_remove_label_for_display(&g_display_manager, acting_did)) {
                    daemon_fail(rsp, "the selected display was not associated with a label!\n");
                }
            }
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static void handle_domain_space(FILE *rsp, struct token domain, char *message)
{
    TIME_FUNCTION;

    struct token command;
    uint64_t acting_sid = space_manager_active_space();
    struct selector selector = parse_space_selector(NULL, &message, acting_sid, true);

    if (selector.did_parse) {
        acting_sid = selector.sid;
        command = get_token(&message);
    } else {
        command = selector.token;
    }

    if (!acting_sid) {
        daemon_fail(rsp, "could not locate the space to act on!\n");
        return;
    }

    while(token_is_valid(command)) {
        if (token_equals(command, COMMAND_SPACE_FOCUS)) {
            struct selector selector = parse_space_selector(rsp, &message, acting_sid, false);
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
            struct selector selector = parse_space_selector(rsp, &message, acting_sid, false);
            if (selector.did_parse && selector.sid) {
                enum space_op_error result = space_manager_switch_space(selector.sid);
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
        } else if (token_equals(command, COMMAND_SPACE_MOVE)) {
            struct selector selector = parse_space_selector(rsp, &message, acting_sid, false);
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
            struct selector selector = parse_space_selector(rsp, &message, acting_sid, false);
            if (selector.did_parse && selector.sid) {
                enum space_op_error result = space_manager_swap_space_with_space(acting_sid, selector.sid);
                if (result == SPACE_OP_ERROR_SAME_SPACE) {
                    daemon_fail(rsp, "cannot swap space with itself.\n");
                } else if (result == SPACE_OP_ERROR_DISPLAY_IS_ANIMATING) {
                    daemon_fail(rsp, "cannot swap space because the display is in the middle of an animation.\n");
                } else if (result == SPACE_OP_ERROR_IN_MISSION_CONTROL) {
                    daemon_fail(rsp, "cannot swap space because mission-control is active.\n");
                } else if (result == SPACE_OP_ERROR_SCRIPTING_ADDITION) {
                    daemon_fail(rsp, "cannot swap space due to an error with the scripting-addition.\n");
                }
            }
        } else if (token_equals(command, COMMAND_SPACE_DISPLAY)) {
            struct selector selector = parse_display_selector(rsp, &message, display_manager_active_display_id(), false);
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
            struct selector selector = parse_display_selector(rsp, &message, display_manager_active_display_id(), true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.did) {
                    acting_sid = display_space_id(selector.did);
                } else {
                    return;
                }
            }

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
            struct selector selector = parse_space_selector(rsp, &message, acting_sid, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.sid) {
                    acting_sid = selector.sid;
                } else {
                    return;
                }
            }

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
        } else if (token_equals(command, COMMAND_SPACE_EQUALIZE)) {
            struct token value = get_token(&message);
            if (!token_is_valid(value)) {
                if (!space_manager_equalize_space(&g_space_manager, acting_sid, SPLIT_X | SPLIT_Y)) {
                    daemon_fail(rsp, "cannot equalize a non-managed space.\n");
                }
            } else if (token_equals(value, ARGUMENT_SPACE_AXIS_X)) {
                if (!space_manager_equalize_space(&g_space_manager, acting_sid, SPLIT_X)) {
                    daemon_fail(rsp, "cannot equalize a non-managed space.\n");
                }
            } else if (token_equals(value, ARGUMENT_SPACE_AXIS_Y)) {
                if (!space_manager_equalize_space(&g_space_manager, acting_sid, SPLIT_Y)) {
                    daemon_fail(rsp, "cannot equalize a non-managed space.\n");
                }
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_SPACE_BALANCE)) {
            struct token value = get_token(&message);
            if (!token_is_valid(value)) {
                if (!space_manager_balance_space(&g_space_manager, acting_sid, SPLIT_X | SPLIT_Y)) {
                    daemon_fail(rsp, "cannot balance a non-managed space.\n");
                }
            } else if (token_equals(value, ARGUMENT_SPACE_AXIS_X)) {
                if (!space_manager_balance_space(&g_space_manager, acting_sid, SPLIT_X)) {
                    daemon_fail(rsp, "cannot balance a non-managed space.\n");
                }
            } else if (token_equals(value, ARGUMENT_SPACE_AXIS_Y)) {
                if (!space_manager_balance_space(&g_space_manager, acting_sid, SPLIT_Y)) {
                    daemon_fail(rsp, "cannot balance a non-managed space.\n");
                }
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_SPACE_MIRROR)) {
            struct token value = get_token(&message);
            if (token_equals(value, ARGUMENT_SPACE_AXIS_X)) {
                if (!space_manager_mirror_space(&g_space_manager, acting_sid, SPLIT_X)) {
                    daemon_fail(rsp, "cannot mirror a non-managed space.\n");
                }
            } else if (token_equals(value, ARGUMENT_SPACE_AXIS_Y)) {
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
            if (parse_label(rsp, get_token(&message), LABEL_SPACE, &label)) {
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

        command = get_token(&message);
    }
}

static void handle_domain_window(FILE *rsp, struct token domain, char *message)
{
    TIME_FUNCTION;

    struct token command;
    struct window *acting_window = window_manager_focused_window(&g_window_manager);
    struct selector selector = parse_window_selector(NULL, &message, acting_window, true);

    if (selector.did_parse) {
        acting_window = selector.window;
        command = get_token(&message);
    } else {
        command = selector.token;
    }

    if (!acting_window &&
        !token_equals(command, COMMAND_WINDOW_FOCUS) &&
        !token_equals(command, COMMAND_WINDOW_CLOSE) &&
        !token_equals(command, COMMAND_WINDOW_MINIMIZE) &&
        !token_equals(command, COMMAND_WINDOW_DEMINIMIZE) &&
        !token_equals(command, COMMAND_WINDOW_TOGGLE)) {
        daemon_fail(rsp, "could not locate the window to act on!\n");
        return;
    }

    while(token_is_valid(command)) {
        if (token_equals(command, COMMAND_WINDOW_FOCUS)) {
            struct selector selector = parse_window_selector(rsp, &message, acting_window, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.window) {
                    acting_window = selector.window;
                } else {
                    return;
                }
            }

            if (acting_window) {
                window_manager_focus_window_with_raise(&acting_window->application->psn, acting_window->id, acting_window->ref);
            } else {
                daemon_fail(rsp, "could not locate the window to act on!\n");
            }
        } else if (token_equals(command, COMMAND_WINDOW_CLOSE)) {
            struct selector selector = parse_window_selector(rsp, &message, acting_window, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.window) {
                    acting_window = selector.window;
                } else {
                    return;
                }
            }

            if (acting_window) {
                if (!window_manager_close_window(acting_window)) {
                    daemon_fail(rsp, "could not close window with id '%d'.\n", acting_window->id);
                }
            } else {
                daemon_fail(rsp, "could not locate the window to act on!\n");
            }
        } else if (token_equals(command, COMMAND_WINDOW_MINIMIZE)) {
            struct selector selector = parse_window_selector(rsp, &message, acting_window, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.window) {
                    acting_window = selector.window;
                } else {
                    return;
                }
            }

            if (acting_window) {
                enum window_op_error result = window_manager_minimize_window(acting_window);
                if (result == WINDOW_OP_ERROR_CANT_MINIMIZE) {
                    daemon_fail(rsp, "window with id '%d' does not support the minimize operation.\n", acting_window->id);
                } else if (result == WINDOW_OP_ERROR_ALREADY_MINIMIZED) {
                    daemon_fail(rsp, "window with id '%d' is already minimized.\n", acting_window->id);
                } else if (result == WINDOW_OP_ERROR_MINIMIZE_FAILED) {
                    daemon_fail(rsp, "could not minimize window with id '%d'.\n", acting_window->id);
                }
            } else {
                daemon_fail(rsp, "could not locate the window to act on!\n");
            }
        } else if (token_equals(command, COMMAND_WINDOW_DEMINIMIZE)) {
            struct selector selector = parse_window_selector(rsp, &message, acting_window, false);
            if (selector.did_parse && selector.window) {
                enum window_op_error result = window_manager_deminimize_window(selector.window);
                if (result == WINDOW_OP_ERROR_NOT_MINIMIZED) {
                    daemon_fail(rsp, "window with id '%d' is not minimized.\n", selector.window->id);
                } else if (result == WINDOW_OP_ERROR_DEMINIMIZE_FAILED) {
                    daemon_fail(rsp, "could not deminimize window with id '%d'.\n", selector.window->id);
                }
            }
        } else if (token_equals(command, COMMAND_WINDOW_DISPLAY)) {
            struct selector selector = parse_display_selector(rsp, &message, display_manager_active_display_id(), false);
            if (selector.did_parse && selector.did) {
                uint64_t sid = display_space_id(selector.did);
                if (space_is_fullscreen(sid)) {
                    daemon_fail(rsp, "can not move window to a macOS fullscreen space!\n");
                } else {
                    window_manager_send_window_to_space(&g_space_manager, &g_window_manager, acting_window, sid, false);
                }
            }
        } else if (token_equals(command, COMMAND_WINDOW_SPACE)) {
            struct selector selector = parse_space_selector(rsp, &message, space_manager_active_space(), false);
            if (selector.did_parse && selector.sid) {
                if (space_is_fullscreen(selector.sid)) {
                    daemon_fail(rsp, "can not move window to a macOS fullscreen space!\n");
                } else {
                    window_manager_send_window_to_space(&g_space_manager, &g_window_manager, acting_window, selector.sid, false);
                }
            }
        } else if (token_equals(command, COMMAND_WINDOW_SWAP)) {
            struct selector selector = parse_window_selector(rsp, &message, acting_window, false);
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
            struct selector selector = parse_window_selector(rsp, &message, acting_window, false);
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
            struct selector selector = parse_window_selector(rsp, &message, acting_window, false);
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
                enum window_op_error result = window_manager_resize_window_relative(&g_window_manager, acting_window, parse_resize_handle(handle), w, h, true);
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
        } else if (token_equals(command, COMMAND_WINDOW_TOGGLE)) {
            struct token value = get_token(&message);
            if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_FLOAT)) {
                if (acting_window) {
                    window_manager_make_window_floating(&g_space_manager, &g_window_manager, acting_window, !window_check_flag(acting_window, WINDOW_FLOAT), false);
                } else {
                    daemon_fail(rsp, "could not locate the window to act on!\n");
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_STICKY)) {
                if (acting_window) {
                    window_manager_make_window_sticky(&g_space_manager, &g_window_manager, acting_window, !window_check_flag(acting_window, WINDOW_STICKY));
                } else {
                    daemon_fail(rsp, "could not locate the window to act on!\n");
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_SHADOW)) {
                if (acting_window) {
                    window_manager_toggle_window_shadow(&g_space_manager, &g_window_manager, acting_window);
                } else {
                    daemon_fail(rsp, "could not locate the window to act on!\n");
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_SPLIT)) {
                if (acting_window) {
                    space_manager_toggle_window_split(&g_space_manager, acting_window);
                } else {
                    daemon_fail(rsp, "could not locate the window to act on!\n");
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_PARENT)) {
                if (acting_window) {
                    window_manager_toggle_window_parent(&g_space_manager, &g_window_manager, acting_window);
                } else {
                    daemon_fail(rsp, "could not locate the window to act on!\n");
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_FULLSC)) {
                if (acting_window) {
                    window_manager_toggle_window_fullscreen(&g_space_manager, &g_window_manager, acting_window);
                } else {
                    daemon_fail(rsp, "could not locate the window to act on!\n");
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_NATIVE)) {
                if (acting_window) {
                    window_manager_toggle_window_native_fullscreen(&g_space_manager, &g_window_manager, acting_window);
                } else {
                    daemon_fail(rsp, "could not locate the window to act on!\n");
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_EXPOSE)) {
                if (acting_window) {
                    window_manager_toggle_window_expose(&g_window_manager, acting_window);
                } else {
                    daemon_fail(rsp, "could not locate the window to act on!\n");
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_TOGGLE_PIP)) {
                if (acting_window) {
                    window_manager_toggle_window_pip(&g_space_manager, &g_window_manager, acting_window);
                } else {
                    daemon_fail(rsp, "could not locate the window to act on!\n");
                }
            } else if (!window_manager_toggle_scratchpad_window_by_label(&g_window_manager, value.text)) {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_WINDOW_SUB_LAYER)) {
            struct token value = get_token(&message);
            if (token_equals(value, ARGUMENT_WINDOW_LAYER_BELOW)) {
                if (!window_manager_set_window_layer(acting_window, LAYER_BELOW)) {
                    daemon_fail(rsp, "could not change sub-layer of window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_LAYER_NORMAL)) {
                if (!window_manager_set_window_layer(acting_window, LAYER_NORMAL)) {
                    daemon_fail(rsp, "could not change sub-layer of window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_LAYER_ABOVE)) {
                if (!window_manager_set_window_layer(acting_window, LAYER_ABOVE)) {
                    daemon_fail(rsp, "could not change sub-layer of window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
                }
            } else if (token_equals(value, ARGUMENT_WINDOW_LAYER_AUTO)) {
                if (!window_manager_set_window_layer(acting_window, LAYER_AUTO)) {
                    daemon_fail(rsp, "could not change sub-layer of window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
                }
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_WINDOW_OPACITY)) {
            struct token_value value = token_to_value(get_token(&message));
            if (value.type == TOKEN_TYPE_FLOAT && in_range_ii(value.float_value, 0.0f, 1.0f)) {
                if (window_manager_set_opacity(&g_window_manager, acting_window, value.float_value)) {
                    acting_window->opacity = value.float_value;
                } else {
                    daemon_fail(rsp, "could not change opacity of window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
                }
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.token.length, value.token.text, command.length, command.text, domain.length, domain.text);
            }
        } else if (token_equals(command, COMMAND_WINDOW_RAISE)) {
            struct selector selector = parse_window_selector(rsp, &message, acting_window, true);
            uint32_t selector_wid = 0;

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.window) {
                    selector_wid = selector.window->id;
                } else {
                    return;
                }
            }

            if (!scripting_addition_order_window(acting_window->id, 1, selector_wid)) {
                daemon_fail(rsp, "could not raise window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
            }
        } else if (token_equals(command, COMMAND_WINDOW_LOWER)) {
            struct selector selector = parse_window_selector(rsp, &message, acting_window, true);
            uint32_t selector_wid = 0;

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.window) {
                    selector_wid = selector.window->id;
                } else {
                    return;
                }
            }

            if (!scripting_addition_order_window(acting_window->id, -1, selector_wid)) {
                daemon_fail(rsp, "could not lower window with id '%d' due to an error with the scripting-addition.\n", acting_window->id);
            }
        } else if (token_equals(command, COMMAND_WINDOW_SCRATCHPAD)) {
            char *label;
            struct token token = get_token(&message);
            if (token_is_valid(token) && token_equals(token, ARGUMENT_WINDOW_SCRATCHPAD_RECOVER)) {
                window_manager_scratchpad_recover_windows();
            } else if (parse_label(rsp, token, LABEL_WINDOW, &label)) {
                if (label) {
                    if (!window_manager_set_scratchpad_for_window(&g_window_manager, acting_window, label)) {
                        daemon_fail(rsp, "the given scratchpad is already assigned to a different window!\n");
                    }
                } else {
                    if (!window_manager_remove_scratchpad_for_window(&g_window_manager, acting_window, true)) {
                        daemon_fail(rsp, "the selected window was not assigned to a scratchpad!\n");
                    }
                }
            }
        } else {
            daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
        }

        command = get_token(&message);
    }
}

static void handle_domain_query(FILE *rsp, struct token domain, char *message)
{
    TIME_FUNCTION;

    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_QUERY_DISPLAYS)) {
        struct properties properties = parse_properties(rsp, get_token(&message), display_property_val, display_property_str, array_count(display_property_str));
        if (properties.did_error) return;

        struct token option = properties.did_parse ? get_token(&message) : properties.token;
        if (token_equals(option, ARGUMENT_QUERY_DISPLAY)) {
            uint32_t acting_did = display_manager_active_display_id();
            struct selector selector = parse_display_selector(rsp, &message, acting_did, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.did) {
                    acting_did = selector.did;
                } else {
                    return;
                }
            }

            display_serialize(rsp, acting_did, properties.flags);
            fprintf(rsp, "\n");
        } else if (token_equals(option, ARGUMENT_QUERY_SPACE)) {
            uint64_t acting_sid = space_manager_active_space();
            struct selector selector = parse_space_selector(rsp, &message, acting_sid, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.sid) {
                    acting_sid = selector.sid;
                } else {
                    return;
                }
            }

            display_serialize(rsp, space_display_id(acting_sid), properties.flags);
            fprintf(rsp, "\n");
        } else if (token_equals(option, ARGUMENT_QUERY_WINDOW)) {
            struct window *acting_window = window_manager_focused_window(&g_window_manager);
            struct selector selector = parse_window_selector(rsp, &message, acting_window, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.window) {
                    acting_window = selector.window;
                } else {
                    return;
                }
            }

            if (acting_window) {
                display_serialize(rsp, window_display_id(acting_window->id), properties.flags);
                fprintf(rsp, "\n");
            } else {
                daemon_fail(rsp, "could not find window to retrieve display details.\n");
            }
        } else if (token_is_valid(option)) {
            daemon_fail(rsp, "unknown option '%.*s' given to command '%.*s' for domain '%.*s'\n", option.length, option.text, command.length, command.text, domain.length, domain.text);
        } else {
            display_manager_query_displays(rsp, properties.flags);
        }
    } else if (token_equals(command, COMMAND_QUERY_SPACES)) {
        struct properties properties = parse_properties(rsp, get_token(&message), space_property_val, space_property_str, array_count(space_property_str));
        if (properties.did_error) return;

        struct token option = properties.did_parse ? get_token(&message) : properties.token;
        if (token_equals(option, ARGUMENT_QUERY_DISPLAY)) {
            uint32_t acting_did = display_manager_active_display_id();
            struct selector selector = parse_display_selector(rsp, &message, acting_did, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.did) {
                    acting_did = selector.did;
                } else {
                    return;
                }
            }

            if (!space_manager_query_spaces_for_display(rsp, acting_did, properties.flags)) {
                daemon_fail(rsp, "could not retrieve spaces for display.\n");
            }
        } else if (token_equals(option, ARGUMENT_QUERY_SPACE)) {
            uint64_t acting_sid = space_manager_active_space();
            struct selector selector = parse_space_selector(rsp, &message, acting_sid, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.sid) {
                    acting_sid = selector.sid;
                } else {
                    return;
                }
            }

            if (!space_manager_query_space(rsp, acting_sid, properties.flags)) {
                daemon_fail(rsp, "could not retrieve space details.\n");
            }
        } else if (token_equals(option, ARGUMENT_QUERY_WINDOW)) {
            struct window *acting_window = window_manager_focused_window(&g_window_manager);
            struct selector selector = parse_window_selector(rsp, &message, acting_window, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.window) {
                    acting_window = selector.window;
                } else {
                    return;
                }
            }

            if (acting_window) {
                space_manager_query_spaces_for_window(rsp, acting_window, properties.flags);
            } else {
                daemon_fail(rsp, "could not find window to retrieve space details.\n");
            }
        } else if (token_is_valid(option)) {
            daemon_fail(rsp, "unknown option '%.*s' given to command '%.*s' for domain '%.*s'\n", option.length, option.text, command.length, command.text, domain.length, domain.text);
        } else if (!space_manager_query_spaces_for_displays(rsp, properties.flags)) {
            daemon_fail(rsp, "could not retrieve spaces for displays.\n");
        }
    } else if (token_equals(command, COMMAND_QUERY_WINDOWS)) {
        struct properties properties = parse_properties(rsp, get_token(&message), window_property_val, window_property_str, array_count(window_property_str));
        if (properties.did_error) return;

        struct token option = properties.did_parse ? get_token(&message) : properties.token;
        if (token_equals(option, ARGUMENT_QUERY_DISPLAY)) {
            uint32_t acting_did = display_manager_active_display_id();
            struct selector selector = parse_display_selector(rsp, &message, acting_did, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.did) {
                    acting_did = selector.did;
                } else {
                    return;
                }
            }

            window_manager_query_windows_for_display(rsp, acting_did, properties.flags);
        } else if (token_equals(option, ARGUMENT_QUERY_SPACE)) {
            uint64_t acting_sid = space_manager_active_space();
            struct selector selector = parse_space_selector(rsp, &message, acting_sid, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.sid) {
                    acting_sid = selector.sid;
                } else {
                    return;
                }
            }

            window_manager_query_windows_for_spaces(rsp, &acting_sid, 1, properties.flags);
        } else if (token_equals(option, ARGUMENT_QUERY_WINDOW)) {
            struct window *acting_window = window_manager_focused_window(&g_window_manager);
            struct selector selector = parse_window_selector(rsp, &message, acting_window, true);

            if (token_is_valid(selector.token)) {
                if (selector.did_parse && selector.window) {
                    acting_window = selector.window;
                } else {
                    return;
                }
            }

            if (acting_window) {
                window_serialize(rsp, acting_window, properties.flags);
                fprintf(rsp, "\n");
            } else {
                daemon_fail(rsp, "could not retrieve window details.\n");
            }
        } else if (token_is_valid(option)) {
            daemon_fail(rsp, "unknown option '%.*s' given to command '%.*s' for domain '%.*s'\n", option.length, option.text, command.length, command.text, domain.length, domain.text);
        } else {
            window_manager_query_windows_for_displays(rsp, properties.flags);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

static bool parse_rule(FILE *rsp, char **message, struct rule *rule, struct token token)
{
    TIME_FUNCTION;

    char *unsupported_exclusion = NULL;
    bool did_parse = true;
    bool has_filter = false;

    while (token_is_valid(token)) {
        char *key = NULL;
        char *value = NULL;
        bool exclusion = false;
        parse_key_value_pair(token.text, &key, &value, &exclusion);

        if (!key || !value) {
            daemon_fail(rsp, "invalid key-value pair '%s'\n", token.text);
            did_parse = false;
            goto rnext;
        }

        if (string_equals(key, ARGUMENT_RULE_KEY_LABEL)) {
            if (exclusion) unsupported_exclusion = key;
            rule->label = string_copy(value);
        } else if (string_equals(key, ARGUMENT_RULE_KEY_SCRATCHPAD)) {
            if (exclusion) unsupported_exclusion = key;

            bool valid = true;
            for (int i = 0; i < array_count(reserved_window_identifiers); ++i) {
                if (string_equals(value, reserved_window_identifiers[i])) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                rule->effects.scratchpad = string_copy(value);
                rule->effects.manage = RULE_PROP_OFF;
            } else {
                daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_APP)) {
            has_filter = true;
            rule->app = string_copy(value);
            if (exclusion) rule_set_flag(rule, RULE_APP_EXCLUDE);
            if (regcomp(&rule->app_regex, value, REG_EXTENDED) == 0) {
                rule_set_flag(rule, RULE_APP_VALID);
            } else {
                daemon_fail(rsp, "invalid regex pattern '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_TITLE)) {
            has_filter = true;
            rule->title = string_copy(value);
            if (exclusion) rule_set_flag(rule, RULE_TITLE_EXCLUDE);
            if (regcomp(&rule->title_regex, value, REG_EXTENDED) == 0) {
                rule_set_flag(rule, RULE_TITLE_VALID);
            } else {
                daemon_fail(rsp, "invalid regex pattern '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_ROLE)) {
            has_filter = true;
            rule->role = string_copy(value);
            if (exclusion) rule_set_flag(rule, RULE_ROLE_EXCLUDE);
            if (regcomp(&rule->role_regex, value, REG_EXTENDED) == 0) {
                rule_set_flag(rule, RULE_ROLE_VALID);
            } else {
                daemon_fail(rsp, "invalid regex pattern '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_SUBROLE)) {
            has_filter = true;
            rule->subrole = string_copy(value);
            if (exclusion) rule_set_flag(rule, RULE_SUBROLE_EXCLUDE);
            if (regcomp(&rule->subrole_regex, value, REG_EXTENDED) == 0) {
                rule_set_flag(rule, RULE_SUBROLE_VALID);
            } else {
                daemon_fail(rsp, "invalid regex pattern '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_DISPLAY)) {
            if (exclusion) unsupported_exclusion = key;

            if (value[0] == ARGUMENT_RULE_VALUE_SPACE) {
                ++value;
                rule_effects_set_flag(&rule->effects, RULE_FOLLOW_SPACE);
            }

            struct selector selector = parse_display_selector(rsp, &value, display_manager_active_display_id(), false);
            if (selector.did_parse && selector.did) {
                rule->effects.did = selector.did;
            } else {
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_SPACE)) {
            if (exclusion) unsupported_exclusion = key;

            if (value[0] == ARGUMENT_RULE_VALUE_SPACE) {
                ++value;
                rule_effects_set_flag(&rule->effects, RULE_FOLLOW_SPACE);
            }

            struct selector selector = parse_space_selector(rsp, &value, space_manager_active_space(), false);
            if (selector.did_parse && selector.sid) {
                rule->effects.sid = selector.sid;
            } else {
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_GRID)) {
            if (exclusion) unsupported_exclusion = key;

            if ((sscanf(value, ARGUMENT_RULE_VALUE_GRID,
                        &rule->effects.grid[0], &rule->effects.grid[1],
                        &rule->effects.grid[2], &rule->effects.grid[3],
                        &rule->effects.grid[4], &rule->effects.grid[5]) != 6)) {
                daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_OPACITY)) {
            if (exclusion) unsupported_exclusion = key;

            if ((sscanf(value, "%f", &rule->effects.opacity) == 1) && (in_range_ii(rule->effects.opacity, 0.0f, 1.0f))) {
                rule_effects_set_flag(&rule->effects, RULE_OPACITY);
            } else {
                daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_MANAGE)) {
            if (exclusion) unsupported_exclusion = key;

            if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                rule->effects.manage = RULE_PROP_ON;
            } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                rule->effects.manage = RULE_PROP_OFF;
            } else {
                daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_STICKY)) {
            if (exclusion) unsupported_exclusion = key;

            if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                rule->effects.sticky = RULE_PROP_ON;
            } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                rule->effects.sticky = RULE_PROP_OFF;
            } else {
                daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_MFF)) {
            if (exclusion) unsupported_exclusion = key;

            if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                rule->effects.mff = RULE_PROP_ON;
            } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                rule->effects.mff = RULE_PROP_OFF;
            } else {
                daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_SUB_LAYER)) {
            if (exclusion) unsupported_exclusion = key;

            if (string_equals(value, ARGUMENT_WINDOW_LAYER_BELOW)) {
                rule->effects.layer = LAYER_BELOW;
                rule_effects_set_flag(&rule->effects, RULE_LAYER);
            } else if (string_equals(value, ARGUMENT_WINDOW_LAYER_NORMAL)) {
                rule->effects.layer = LAYER_NORMAL;
                rule_effects_set_flag(&rule->effects, RULE_LAYER);
            } else if (string_equals(value, ARGUMENT_WINDOW_LAYER_ABOVE)) {
                rule->effects.layer = LAYER_ABOVE;
                rule_effects_set_flag(&rule->effects, RULE_LAYER);
            } else if (string_equals(value, ARGUMENT_WINDOW_LAYER_AUTO)) {
                rule->effects.layer = LAYER_AUTO;
                rule_effects_set_flag(&rule->effects, RULE_LAYER);
            } else {
                daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else if (string_equals(key, ARGUMENT_RULE_KEY_FULLSCR)) {
            if (exclusion) unsupported_exclusion = key;

            if (string_equals(value, ARGUMENT_COMMON_VAL_ON)) {
                rule->effects.fullscreen = RULE_PROP_ON;
            } else if (string_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
                rule->effects.fullscreen = RULE_PROP_OFF;
            } else {
                daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                did_parse = false;
            }
        } else {
            daemon_fail(rsp, "unknown key '%s'\n", key);
            did_parse = false;
        }

rnext:
        token = get_token(message);
    }

    if (!has_filter) {
        daemon_fail(rsp, "missing required key-value pair 'app[!]=..' or 'title[!]=..'\n");
        did_parse = false;
    }

    if (unsupported_exclusion) {
        daemon_fail(rsp, "unsupported token '!' (exclusion) given for key '%s'\n", unsupported_exclusion);
        did_parse = false;
    }

    return did_parse;
}

static void handle_domain_rule(FILE *rsp, struct token domain, char *message)
{
    TIME_FUNCTION;

    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_RULE_ADD)) {
        struct rule rule = {};

        struct token token = get_token(&message);
        if (token_equals(token, ARGUMENT_RULE_ONE_SHOT)) {
            rule_set_flag(&rule, RULE_ONE_SHOT);
            token = get_token(&message);
        }

        if (parse_rule(rsp, &message, &rule, token)) {
            rule_add(&rule);
        } else {
            rule_destroy(&rule);
        }
    } else if (token_equals(command, COMMAND_RULE_APPLY)) {
        struct token_value value = token_to_value(get_token(&message));
        if (value.type == TOKEN_TYPE_INT) {
            if (!rule_reapply_by_index(value.int_value)) {
                daemon_fail(rsp, "rule with index '%d' not found.\n", value.int_value);
            }
        } else if (value.type == TOKEN_TYPE_STRING) {
            if (!rule_reapply_by_label(value.string_value)) {
                struct rule rule = {};
                if (parse_rule(rsp, &message, &rule, value.token)) {
                    rule_apply(&rule);
                }
                rule_destroy(&rule);
            }
        } else if (value.type == TOKEN_TYPE_INVALID) {
            rule_reapply_all();
        } else {
            daemon_fail(rsp, "value '%.*s' is not a valid option for RULE_SEL\n", value.token.length, value.token.text);
        }
    } else if (token_equals(command, COMMAND_RULE_REM)) {
        struct token_value value = token_to_value(get_token(&message));
        if (value.type == TOKEN_TYPE_INT) {
            if (!rule_remove_by_index(value.int_value)) {
                daemon_fail(rsp, "rule with index '%d' not found.\n", value.int_value);
            }
        } else if (value.type == TOKEN_TYPE_STRING) {
            if (!rule_remove_by_label(value.string_value)) {
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
    TIME_FUNCTION;

    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_SIGNAL_ADD)) {
        char *unsupported_exclusion = NULL;
        bool did_parse = true;
        bool has_command = false;
        bool has_signal_type = false;
        enum signal_type signal_type = SIGNAL_TYPE_UNKNOWN;
        struct signal signal = {};

        struct token token = get_token(&message);
        while (token_is_valid(token)) {
            char *key = NULL;
            char *value = NULL;
            bool exclusion = false;
            parse_key_value_pair(token.text, &key, &value, &exclusion);

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
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_ACTIVE)) {
                if (exclusion) unsupported_exclusion = key;

                if (string_equals(value, ARGUMENT_SIGNAL_VALUE_YES)) {
                    signal.active = SIGNAL_PROP_YES;
                } else if (string_equals(value, ARGUMENT_SIGNAL_VALUE_NO)) {
                    signal.active = SIGNAL_PROP_NO;
                } else {
                    daemon_fail(rsp, "invalid value '%s' for key '%s'\n", value, key);
                    did_parse = false;
                }
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_ACTION)) {
                if (exclusion) unsupported_exclusion = key;

                has_command = true;
                signal.command = string_copy(value);
            } else if (string_equals(key, ARGUMENT_SIGNAL_KEY_EVENT)) {
                if (exclusion) unsupported_exclusion = key;

                has_signal_type = true;
                signal_type = signal_type_from_string(value);
                if (signal_type == SIGNAL_TYPE_UNKNOWN) {
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
        struct token_value value = token_to_value(get_token(&message));
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

static void *message_loop_run(void *context)
{
    while (g_message_loop.is_running) {
        int sockfd = accept(g_message_loop.sockfd, NULL, 0);
        if (sockfd == -1) continue;

        event_loop_post(&g_event_loop, DAEMON_MESSAGE, NULL, sockfd);
    }

    return NULL;
}

bool message_loop_begin(char *socket_path)
{
    struct sockaddr_un socket_address;
    socket_address.sun_family = AF_UNIX;
    snprintf(socket_address.sun_path, sizeof(socket_address.sun_path), "%s", socket_path);
    unlink(socket_path);

    if ((g_message_loop.sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        return false;
    }

    if (bind(g_message_loop.sockfd, (struct sockaddr *) &socket_address, sizeof(socket_address)) == -1) {
        return false;
    }

    if (chmod(socket_path, 0600) != 0) {
        return false;
    }

    if (listen(g_message_loop.sockfd, SOMAXCONN) == -1) {
        return false;
    }

    fcntl(g_message_loop.sockfd, F_SETFD, FD_CLOEXEC | fcntl(g_message_loop.sockfd, F_GETFD));

    g_message_loop.is_running = true;
    pthread_create(&g_message_loop.thread, NULL, &message_loop_run, NULL);

    return true;
}

#ifndef VIEW_H
#define VIEW_H

#define AX_ABS(a, b) (((a) - (b) < 0) ? (((a) - (b)) * -1) : ((a) - (b)))
#define AX_DIFF(a, b) (AX_ABS(a, b) >= 1.5f)

enum space_property
{
    SPACE_PROPERTY_ID            = 0x001,
    SPACE_PROPERTY_UUID          = 0x002,
    SPACE_PROPERTY_INDEX         = 0x004,
    SPACE_PROPERTY_LABEL         = 0x008,
    SPACE_PROPERTY_TYPE          = 0x010,
    SPACE_PROPERTY_DISPLAY       = 0x020,
    SPACE_PROPERTY_WINDOWS       = 0x040,
    SPACE_PROPERTY_FIRST_WINDOW  = 0x080,
    SPACE_PROPERTY_LAST_WINDOW   = 0x100,
    SPACE_PROPERTY_HAS_FOCUS     = 0x200,
    SPACE_PROPERTY_IS_VISIBLE    = 0x400,
    SPACE_PROPERTY_IS_FULLSCREEN = 0x800
};

static uint64_t space_property_val[] =
{
    [0x0] = SPACE_PROPERTY_ID,
    [0x1] = SPACE_PROPERTY_UUID,
    [0x2] = SPACE_PROPERTY_INDEX,
    [0x3] = SPACE_PROPERTY_LABEL,
    [0x4] = SPACE_PROPERTY_TYPE,
    [0x5] = SPACE_PROPERTY_DISPLAY,
    [0x6] = SPACE_PROPERTY_WINDOWS,
    [0x7] = SPACE_PROPERTY_FIRST_WINDOW,
    [0x8] = SPACE_PROPERTY_LAST_WINDOW,
    [0x9] = SPACE_PROPERTY_HAS_FOCUS,
    [0xA] = SPACE_PROPERTY_IS_VISIBLE,
    [0xB] = SPACE_PROPERTY_IS_FULLSCREEN
};

static char *space_property_str[] =
{
    "id",
    "uuid",
    "index",
    "label",
    "type",
    "display",
    "windows",
    "first-window",
    "last-window",
    "has-focus",
    "is-visible",
    "is-native-fullscreen"
};

struct area
{
    float x;
    float y;
    float w;
    float h;
};

struct window;
struct window_capture
{
    struct window *window;
    float x, y, w, h;
};

struct window_proxy
{
    uint32_t id;
    CGContextRef context;
    float tx, ty, tw, th;
    CGRect frame;
    int level;
    int sub_level;
    CGImageRef image;
};

struct window_animation
{
    struct window *window;
    uint32_t wid;
    float x, y, w, h;
    int cid;
    struct window_proxy proxy;
    volatile bool skip;
};

struct window_animation_context
{
    int animation_connection;
    int animation_easing;
    float animation_duration;
    uint64_t animation_clock;
    struct window_animation *animation_list;
    int animation_count;
};

struct balance_node
{
    int y_count;
    int x_count;
};

enum window_insertion_point
{
    INSERT_FOCUSED,
    INSERT_FIRST,
    INSERT_LAST
};

static const char *window_insertion_point_str[] =
{
    "focused",
    "first",
    "last"
};

enum window_node_child
{
    CHILD_NONE,
    CHILD_SECOND,
    CHILD_FIRST,
};

static const char *window_node_child_str[] =
{
    "none",
    "second_child",
    "first_child"
};

enum window_node_split
{
    SPLIT_NONE,
    SPLIT_Y,
    SPLIT_X,
    SPLIT_AUTO
};

static const char *window_node_split_str[] =
{
    "none",
    "vertical",
    "horizontal",
    "auto"
};

struct feedback_window
{
    uint32_t id;
    CGContextRef context;
};

#define NODE_MAX_WINDOW_COUNT 32
struct window_node
{
    struct area area;
    struct window_node *parent;
    struct window_node *left;
    struct window_node *right;
    struct window_node *zoom;
    uint32_t window_list[NODE_MAX_WINDOW_COUNT];
    uint32_t window_order[NODE_MAX_WINDOW_COUNT];
    int window_count;
    float ratio;
    enum window_node_split split;
    enum window_node_child child;
    int insert_dir;
    struct feedback_window feedback_window;
};

enum view_type
{
    VIEW_DEFAULT,
    VIEW_BSP,
    VIEW_STACK,
    VIEW_FLOAT
};

static const char *view_type_str[] =
{
    "default",
    "bsp",
    "stack",
    "float"
};

enum view_flag
{
    VIEW_LAYOUT         = 0x001,
    VIEW_TOP_PADDING    = 0x002,
    VIEW_BOTTOM_PADDING = 0x004,
    VIEW_LEFT_PADDING   = 0x008,
    VIEW_RIGHT_PADDING  = 0x010,
    VIEW_WINDOW_GAP     = 0x020,
    VIEW_AUTO_BALANCE   = 0x040,
    VIEW_ENABLE_PADDING = 0x080,
    VIEW_ENABLE_GAP     = 0x100,
    VIEW_IS_VALID       = 0x200,
    VIEW_IS_DIRTY       = 0x400,
};

struct view
{
    CFStringRef uuid;
    uint64_t sid;
    struct window_node *root;
    uint32_t insertion_point;
    enum view_type layout;
    int top_padding;
    int bottom_padding;
    int left_padding;
    int right_padding;
    int window_gap;
    bool auto_balance;
    uint64_t flags;
};

#define view_check_flag(v, x) ((v)->flags  &  (x))
#define view_clear_flag(v, x) ((v)->flags &= ~(x))
#define view_set_flag(v, x)   ((v)->flags |=  (x))

void insert_feedback_show(struct window_node *node);
void insert_feedback_destroy(struct window_node *node);

void window_node_flush(struct window_node *node);
void window_node_update(struct view *view, struct window_node *node);
bool window_node_contains_window(struct window_node *node, uint32_t window_id);
int window_node_index_of_window(struct window_node *node, uint32_t window_id);
void window_node_swap_window_list(struct window_node *a_node, struct window_node *b_node);
struct window_node *window_node_find_first_leaf(struct window_node *root);
struct window_node *window_node_find_last_leaf(struct window_node *root);
struct window_node *window_node_find_prev_leaf(struct window_node *node);
struct window_node *window_node_find_next_leaf(struct window_node *node);
void window_node_capture_windows(struct window_node *node, struct window_capture **window_list);

struct window_node *view_find_window_node_in_direction(struct view *view, struct window_node *source, int direction);
struct window_node *view_find_window_node(struct view *view, uint32_t window_id);
void view_stack_window_node(struct window_node *node, struct window *window);
struct window_node *view_add_window_node_with_insertion_point(struct view *view, struct window *window, uint32_t insertion_point);
struct window_node *view_add_window_node(struct view *view, struct window *window);
struct window_node *view_remove_window_node(struct view *view, struct window *window);
uint32_t *view_find_window_list(struct view *view, int *window_count);

void view_serialize(FILE *rsp, struct view *view, uint64_t flags);
bool view_is_invalid(struct view *view);
bool view_is_dirty(struct view *view);
void view_flush(struct view *view);
void view_update(struct view *view);
struct view *view_create(uint64_t sid);
void view_clear(struct view *view);

#endif

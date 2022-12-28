#ifndef VIEW_H
#define VIEW_H

struct window;

struct area
{
    float x;
    float y;
    float w;
    float h;
};

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
    CFArrayRef image;
};

struct window_animation
{
    uint32_t wid;
    float x, y, w, h;
    struct window_proxy proxy;
    volatile bool skip;
};

struct window_animation_context
{
    int animation_connection;
    float animation_duration;
    int animation_frame_rate;
    struct window_animation *animation_list;
};

struct equalize_node
{
    int y_count;
    int x_count;
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
    uint32_t window_count;
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

struct view
{
    CFStringRef suuid;
    uint64_t sid;
    struct window_node *root;
    enum view_type layout;
    uint32_t insertion_point;
    int top_padding;
    int bottom_padding;
    int left_padding;
    int right_padding;
    int window_gap;
    bool custom_layout;
    bool custom_top_padding;
    bool custom_bottom_padding;
    bool custom_left_padding;
    bool custom_right_padding;
    bool custom_window_gap;
    bool enable_padding;
    bool enable_gap;
    bool is_valid;
    bool is_dirty;
};

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
void view_stack_window_node(struct view *view, struct window_node *node, struct window *window);
struct window_node *view_add_window_node_with_insertion_point(struct view *view, struct window *window, uint32_t insertion_point);
struct window_node *view_add_window_node(struct view *view, struct window *window);
struct window_node *view_remove_window_node(struct view *view, struct window *window);
uint32_t *view_find_window_list(struct view *view, int *window_count);

void view_serialize(FILE *rsp, struct view *view);
bool view_is_invalid(struct view *view);
bool view_is_dirty(struct view *view);
void view_flush(struct view *view);
void view_update(struct view *view);
struct view *view_create(uint64_t sid);
void view_clear(struct view *view);

#endif

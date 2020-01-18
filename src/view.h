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
    SPLIT_X
};

static const char *window_node_split_str[] =
{
    "none",
    "vertical",
    "horizontal"
};

struct window_node
{
    struct area area;
    uint32_t window_id;
    struct window_node *parent;
    struct window_node *left;
    struct window_node *right;
    struct window_node *zoom;
    enum window_node_split split;
    enum window_node_child child;
    int insert_direction;
    float ratio;
};

enum view_type
{
    VIEW_DEFAULT,
    VIEW_BSP,
    VIEW_FLOAT
};

static const char *view_type_str[] =
{
    "default",
    "bsp",
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

float window_node_border_window_offset(struct window *window);
void window_node_flush(struct window_node *node);
void window_node_update(struct view *view, struct window_node *node);
struct window_node *window_node_find_first_leaf(struct window_node *root);
struct window_node *window_node_find_last_leaf(struct window_node *root);
struct window_node *window_node_find_prev_leaf(struct window_node *node);
struct window_node *window_node_find_next_leaf(struct window_node *node);

struct window_node *view_find_window_node(struct view *view, uint32_t window_id);
void view_remove_window_node(struct view *view, struct window *window);
void view_add_window_node(struct view *view, struct window *window);
uint32_t *view_find_window_list(struct view *view);

void view_serialize(FILE *rsp, struct view *view);
bool view_is_invalid(struct view *view);
bool view_is_dirty(struct view *view);
void view_flush(struct view *view);
void view_update(struct view *view);
struct view *view_create(uint64_t sid);
void view_clear(struct view *view);

#endif

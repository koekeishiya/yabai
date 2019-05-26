#ifndef VIEW_H
#define VIEW_H

struct ax_window;

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
    CHILD_RIGHT,
    CHILD_LEFT,
};

enum window_node_split
{
    SPLIT_NONE,
    SPLIT_Y,
    SPLIT_X
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
    uint64_t sid;
    enum view_type type;
    struct window_node *root;
    uint32_t insertion_point;
    uint32_t top_padding;
    uint32_t bottom_padding;
    uint32_t left_padding;
    uint32_t right_padding;
    uint32_t window_gap;
    bool enable_padding;
    bool enable_gap;
    bool is_valid;
    bool is_dirty;
};

#define view_lookup_layout(p, i) (p[i] == VIEW_DEFAULT ? p[0] : p[i])
#define view_lookup_padding(p, i) (p[i] == -1 ? p[0] : p[i])
#define view_lookup_gap(p, i) (p[i] == -1 ? p[0] : p[i])

float window_node_border_window_offset(struct ax_window *window);
void window_node_flush(struct window_node *node);
struct window_node *window_node_find_prev_leaf(struct window_node *node);
struct window_node *window_node_find_next_leaf(struct window_node *node);

struct window_node *view_find_window_node(struct window_node *node, uint32_t window_id);
void view_remove_window_node(struct view *view, struct ax_window *window);
void view_add_window_node(struct view *view, struct ax_window *window);
uint32_t *view_find_window_list(struct view *view);

void view_serialize(FILE *rsp, struct view *view);
bool view_is_invalid(struct view *view);
bool view_is_dirty(struct view *view);
void view_flush(struct view *view);
void view_update(struct view *view);
struct view *view_create(uint64_t sid);

#endif

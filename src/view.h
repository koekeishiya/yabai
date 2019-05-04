#ifndef VIEW_H
#define VIEW_H

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

enum window_node_split
{
    SPLIT_NONE,
    SPLIT_Y,
    SPLIT_X
};

struct window_node
{
    struct area area;
    struct ax_window *window;
    struct window_node *parent;
    struct window_node *left;
    struct window_node *right;
    enum window_node_split split;
    float ratio;
};

enum view_type
{
    VIEW_FLOAT,
    VIEW_MANAGED
};

struct view
{
    uint64_t sid;
    enum view_type type;
    struct window_node *root;
    bool is_valid;
    bool is_dirty;
};

void view_remove_window_node(struct view *view, struct ax_window *window);
void view_add_window_node(struct view *view, struct ax_window *window);

bool view_is_invalid(struct view *view);
bool view_is_dirty(struct view *view);
void view_flush(struct view *view);
void view_update(struct view *view);
struct view *view_create(uint64_t sid);

#endif

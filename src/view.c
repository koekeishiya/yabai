#include "view.h"

extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;

static struct area area_from_cgrect(CGRect rect)
{
    struct area area = {
        rect.origin.x,
        rect.origin.y,
        rect.size.width,
        rect.size.height
    };
    return  area;
}

static enum window_node_child window_node_get_child(struct window_node *node)
{
    if (node->child != CHILD_NONE) return node->child;
    return g_space_manager.window_placement;
}

static enum window_node_split window_node_get_split(struct window_node *node)
{
    if (node->split != SPLIT_NONE) return node->split;
    return node->area.w / node->area.h >= 1.1618f ? SPLIT_Y : SPLIT_X;
}

static float window_node_get_ratio(struct window_node *node)
{
    if (node->ratio >= 0.1f && node->ratio <= 0.9f) return node->ratio;
    return g_space_manager.split_ratio;
}

static float window_node_get_gap(struct view *view)
{
    return view->enable_gap ? view->window_gap*0.5f : 0.0f;
}

static void area_make_pair(struct view *view, struct window_node *node)
{
    enum window_node_split split = window_node_get_split(node);
    float ratio = window_node_get_ratio(node);
    float gap   = window_node_get_gap(view);

    if (split == SPLIT_Y) {
        node->left->area = node->area;
        node->left->area.w *= ratio;
        node->left->area.w -= gap;

        node->right->area = node->area;
        node->right->area.x += (node->area.w * ratio);
        node->right->area.w *= (1 - ratio);
        node->right->area.x += gap;
        node->right->area.w -= gap;
    } else {
        node->left->area = node->area;
        node->left->area.h *= ratio;
        node->left->area.h -= gap;

        node->right->area = node->area;
        node->right->area.y += (node->area.h * ratio);
        node->right->area.h *= (1 - ratio);
        node->right->area.y += gap;
        node->right->area.h -= gap;
    }

    node->split = split;
    node->ratio = ratio;
}

static bool window_node_is_occupied(struct window_node *node)
{
    return node->window_id != 0;
}

static bool window_node_is_intermediate(struct window_node *node)
{
    return node->parent != NULL;
}

static bool window_node_is_leaf(struct window_node *node)
{
    return node->left == NULL && node->right == NULL;
}

static bool window_node_is_left_child(struct window_node *node)
{
    return node->parent && node->parent->left == node;
}

static bool window_node_is_right_child(struct window_node *node)
{
    return node->parent && node->parent->right == node;
}

static struct equalize_node equalize_node_add(struct equalize_node a, struct equalize_node b)
{
    return (struct equalize_node) {
        a.y_count + b.y_count,
        a.x_count + b.x_count,
    };
}

static struct equalize_node window_node_equalize(struct window_node *node)
{
    if (window_node_is_leaf(node)) {
        return (struct equalize_node) {
            node->parent ? node->parent->split == SPLIT_Y : 0,
            node->parent ? node->parent->split == SPLIT_X : 0
        };
    }

    struct equalize_node left_leafs  = window_node_equalize(node->left);
    struct equalize_node right_leafs = window_node_equalize(node->right);
    struct equalize_node total_leafs = equalize_node_add(left_leafs, right_leafs);

    if (node->split == SPLIT_Y) {
        node->ratio = (float) left_leafs.y_count / total_leafs.y_count;
        --total_leafs.y_count;
    } else if (node->split == SPLIT_X) {
        node->ratio = (float) left_leafs.x_count / total_leafs.x_count;
        --total_leafs.x_count;
    }

    if (node->parent) {
        total_leafs.y_count += node->parent->split == SPLIT_Y;
        total_leafs.x_count += node->parent->split == SPLIT_X;
    }

    return total_leafs;
}

static void window_node_split(struct view *view, struct window_node *node, struct window *window)
{
    struct window_node *left = malloc(sizeof(struct window_node));
    memset(left, 0, sizeof(struct window_node));

    struct window_node *right = malloc(sizeof(struct window_node));
    memset(right, 0, sizeof(struct window_node));

    if (window_node_get_child(node) == CHILD_SECOND) {
        left->window_id = node->window_id;
        right->window_id = window->id;
    } else {
        right->window_id = node->window_id;
        left->window_id = window->id;
    }

    left->parent = node;
    right->parent = node;

    node->window_id = 0;
    node->left = left;
    node->right = right;

    area_make_pair(view, node);
}

static void window_node_update(struct view *view, struct window_node *node)
{
    if (window_node_is_intermediate(node)) {
        area_make_pair(view, node->parent);
    }

    if (!window_node_is_leaf(node)) {
        window_node_update(view, node->left);
        window_node_update(view, node->right);
    }
}

static void window_node_destroy(struct window_node *node)
{
    if (node->left)  window_node_destroy(node->left);
    if (node->right) window_node_destroy(node->right);

    if (node->window_id) window_manager_remove_managed_window(&g_window_manager, node->window_id);
    free(node);
}

float window_node_border_window_offset(struct window *window)
{
    if (!window->border.enabled) return 0.0f;
    if (g_window_manager.window_border_placement == BORDER_PLACEMENT_EXTERIOR) return window->border.width;
    if (g_window_manager.window_border_placement == BORDER_PLACEMENT_INTERIOR) return 0.0f;
    if (g_window_manager.window_border_placement == BORDER_PLACEMENT_INSET)    return window->border.width*0.5f;
    return 0.0f; // shutup compiler
}

void window_node_flush(struct window_node *node)
{
    if (window_node_is_occupied(node)) {
        struct window *window = window_manager_find_window(&g_window_manager, node->window_id);
        if (window) {
            float offset = window_node_border_window_offset(window);
            if (node->zoom) {
                window_manager_set_window_frame(window, node->zoom->area.x + offset, node->zoom->area.y + offset, node->zoom->area.w - 2*offset, node->zoom->area.h - 2*offset);
            } else {
                window_manager_set_window_frame(window, node->area.x + offset, node->area.y + offset, node->area.w - 2*offset, node->area.h - 2*offset);
            }
        }
    }

    if (!window_node_is_leaf(node)) {
        window_node_flush(node->left);
        window_node_flush(node->right);
    }
}

struct window_node *window_node_find_first_leaf(struct window_node *root)
{
    struct window_node *node = root;
    while (!window_node_is_leaf(node)) {
        node = node->left;
    }
    return node;
}

struct window_node *window_node_find_last_leaf(struct window_node *root)
{
    struct window_node *node = root;
    while (!window_node_is_leaf(node)) {
        node = node->right;
    }
    return node;
}

struct window_node *window_node_find_prev_leaf(struct window_node *node)
{
    if (!node->parent) return NULL;

    if (window_node_is_left_child(node)) {
        return window_node_find_prev_leaf(node->parent);
    }

    if (window_node_is_leaf(node->parent->left)) {
        return node->parent->left;
    }

    return window_node_find_first_leaf(node->parent->left->right);
}

struct window_node *window_node_find_next_leaf(struct window_node *node)
{
    if (!node->parent) return NULL;

    if (window_node_is_right_child(node)) {
        return window_node_find_next_leaf(node->parent);
    }

    if (window_node_is_leaf(node->parent->right)) {
        return node->parent->right;
    }

    return window_node_find_first_leaf(node->parent->right->left);
}

void window_node_rotate(struct window_node *node, int degrees)
{
    if ((degrees ==  90 && node->split == SPLIT_Y) ||
        (degrees == 270 && node->split == SPLIT_X) ||
        (degrees == 180)) {
        struct window_node *temp = node->left;
        node->left  = node->right;
        node->right = temp;
        node->ratio = 1 - node->ratio;
    }

    if (degrees != 180) {
        if (node->split == SPLIT_X) {
            node->split = SPLIT_Y;
        } else if (node->split == SPLIT_Y) {
            node->split = SPLIT_X;
        }
    }

    if (!window_node_is_leaf(node)) {
        window_node_rotate(node->left, degrees);
        window_node_rotate(node->right, degrees);
    }
}

struct window_node *window_node_mirror(struct window_node *node, enum window_node_split axis)
{
    if (!window_node_is_leaf(node)) {
        struct window_node *left = window_node_mirror(node->left, axis);
        struct window_node *right = window_node_mirror(node->right, axis);

        if (node->split == axis) {
            node->left = right;
            node->right = left;
        }
    }

    return node;
}

struct window_node *window_node_fence(struct window_node *node, int dir)
{
    if (!node) return NULL;

    struct window_node *parent = node->parent;
    while (parent) {
    if ((dir == DIR_NORTH && parent->split == SPLIT_X && parent->area.y < node->area.y) ||
        (dir == DIR_WEST  && parent->split == SPLIT_Y && parent->area.x < node->area.x) ||
        (dir == DIR_SOUTH && parent->split == SPLIT_X && (parent->area.y + parent->area.h) > (node->area.y + node->area.h)) ||
        (dir == DIR_EAST  && parent->split == SPLIT_Y && (parent->area.x + parent->area.w) > (node->area.x + node->area.w))) {
            return parent;
        }

        parent = parent->parent;
    }

    return NULL;
}

struct window_node *view_find_min_depth_leaf_node(struct window_node *node)
{
    struct window_node *list[256] = { node };

    for (int i = 0, j = 0; i < 256; ++i) {
        if (window_node_is_leaf(list[i])) {
            return list[i];
        }

        list[++j] = list[i]->left;
        list[++j] = list[i]->right;
    }

    return NULL;
}

struct window_node *view_find_window_node(struct view *view, uint32_t window_id)
{
    struct window_node *node = window_node_find_first_leaf(view->root);
    while (node) {
        if (node->window_id == window_id) return node;
        node = window_node_find_next_leaf(node);
    }

    return NULL;
}

void view_remove_window_node(struct view *view, struct window *window)
{
    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return;

    if (node == view->root) {
        view->root->window_id = 0;
        return;
    }

    struct window_node *parent = node->parent;
    struct window_node *child  = window_node_is_right_child(node)
                               ? parent->left
                               : parent->right;

    parent->window_id = child->window_id;
    parent->left      = NULL;
    parent->right     = NULL;

    if (window_node_is_intermediate(child) && !window_node_is_leaf(child)) {
        parent->left = child->left;
        parent->left->parent = parent;

        parent->right = child->right;
        parent->right->parent = parent;

        window_node_update(view, parent);
    }

    free(child);
    free(node);

    if (g_space_manager.auto_balance) {
        window_node_equalize(view->root);
        view_update(view);
    }
}

void view_add_window_node(struct view *view, struct window *window)
{
    if (!window_node_is_occupied(view->root) &&
        window_node_is_leaf(view->root)) {
        view->root->window_id = window->id;
    } else {
        struct window_node *leaf = NULL;

        if (view->insertion_point) {
            leaf = view_find_window_node(view, view->insertion_point);
            view->insertion_point = 0;
        }

        if (!leaf) leaf = view_find_window_node(view, g_window_manager.focused_window_id);
        if (!leaf) leaf = view_find_min_depth_leaf_node(view->root);

        struct window *leaf_window = window_manager_find_window(&g_window_manager, leaf->window_id);
        window_node_split(view, leaf, window);

        if (leaf_window) {
            if (leaf_window->border.insert_active) {
                leaf_window->border.insert_active = false;
                leaf_window->border.insert_dir = 0;
            }
        }

        if (g_space_manager.auto_balance) {
            window_node_equalize(view->root);
            view_update(view);
        }
    }
}

uint32_t *view_find_window_list(struct view *view)
{
    uint32_t *window_list = NULL;

    struct window_node *node = window_node_find_first_leaf(view->root);
    while (node) {
        buf_push(window_list, node->window_id);
        node = window_node_find_next_leaf(node);
    }

    return window_list;
}

bool view_is_invalid(struct view *view)
{
    return !view->is_valid;
}

bool view_is_dirty(struct view *view)
{
    return view->is_dirty;
}

void view_flush(struct view *view)
{
    window_node_flush(view->root);
    view->is_dirty = false;
}

void view_serialize(FILE *rsp, struct view *view)
{
    int buffer_size = MAXLEN;
    size_t bytes_written = 0;
    char buffer[MAXLEN] = {};
    char *cursor = buffer;

    int count = 0;
    uint32_t windows[MAXLEN] = {};

    int window_count = 0;
    uint32_t *window_list = space_window_list(view->sid, &window_count);
    if (window_list) {
        for (int i = 0; i < window_count; ++i) {
            if (window_manager_find_window(&g_window_manager, window_list[i])) {
                windows[count++] = window_list[i];
            }
        }
        free(window_list);
    }

    for (int i = 0; i < count; ++i) {
        if (i < count - 1) {
            bytes_written = snprintf(cursor, buffer_size, "%d, ", windows[i]);
        } else {
            bytes_written = snprintf(cursor, buffer_size, "%d", windows[i]);
        }

        cursor += bytes_written;
        buffer_size -= bytes_written;
        if (buffer_size <= 0) break;
    }

    struct space_label *space_label = space_manager_get_label_for_space(&g_space_manager, view->sid);
    struct window_node *first_leaf = window_node_find_first_leaf(view->root);
    struct window_node *last_leaf = window_node_find_last_leaf(view->root);

    fprintf(rsp,
            "{\n"
            "\t\"id\":%lld,\n"
            "\t\"label\":\"%s\",\n"
            "\t\"index\":%d,\n"
            "\t\"display\":%d,\n"
            "\t\"windows\":[%s],\n"
            "\t\"type\":\"%s\",\n"
            "\t\"visible\":%d,\n"
            "\t\"focused\":%d,\n"
            "\t\"native-fullscreen\":%d,\n"
            "\t\"first-window\":%d,\n"
            "\t\"last-window\":%d\n"
            "}",
            view->sid,
            space_label ? space_label->label : "",
            space_manager_mission_control_index(view->sid),
            display_arrangement(space_display_id(view->sid)),
            buffer,
            view_type_str[view->layout],
            space_is_visible(view->sid),
            view->sid == g_space_manager.current_space_id,
            space_is_fullscreen(view->sid),
            first_leaf ? first_leaf->window_id : 0,
            last_leaf ? last_leaf->window_id : 0);
}

void view_update(struct view *view)
{
    uint32_t did = space_display_id(view->sid);
    CGRect frame = display_bounds_constrained(did);
    view->root->area = area_from_cgrect(frame);

    if (view->enable_padding) {
        view->root->area.x += view->left_padding;
        view->root->area.w -= (view->left_padding + view->right_padding);
        view->root->area.y += view->top_padding;
        view->root->area.h -= (view->top_padding + view->bottom_padding);
    }

    window_node_update(view, view->root);
    view->is_valid = true;
    view->is_dirty = true;
}

struct view *view_create(uint64_t sid)
{
    struct view *view = malloc(sizeof(struct view));
    memset(view, 0, sizeof(struct view));

    view->root = malloc(sizeof(struct window_node));
    memset(view->root, 0, sizeof(struct window_node));

    view->enable_padding = true;
    view->enable_gap = true;
    view->sid = sid;

    if (space_is_user(view->sid)) {
        if (!view->custom_layout)         view->layout         = g_space_manager.layout;
        if (!view->custom_top_padding)    view->top_padding    = g_space_manager.top_padding;
        if (!view->custom_bottom_padding) view->bottom_padding = g_space_manager.bottom_padding;
        if (!view->custom_left_padding)   view->left_padding   = g_space_manager.left_padding;
        if (!view->custom_right_padding)  view->right_padding  = g_space_manager.right_padding;
        if (!view->custom_window_gap)     view->window_gap     = g_space_manager.window_gap;
        view_update(view);
    } else {
        view->layout = VIEW_FLOAT;
    }

    return view;
}

void view_clear(struct view *view)
{
    if (view->root) {
        if (view->root->left)  window_node_destroy(view->root->left);
        if (view->root->right) window_node_destroy(view->root->right);
        memset(view->root, 0, sizeof(struct window_node));
        view_update(view);
    }
}

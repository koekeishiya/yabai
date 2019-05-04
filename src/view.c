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

static void area_make_pair(struct window_node *node)
{
    enum window_node_split split = window_node_get_split(node);
    float ratio = window_node_get_ratio(node);
    float gap   = g_space_manager.window_gap / 2.0f;

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


static void window_node_split(struct window_node *node, struct ax_window *window)
{
    struct window_node *left = malloc(sizeof(struct window_node));
    memset(left, 0, sizeof(struct window_node));

    struct window_node *right = malloc(sizeof(struct window_node));
    memset(right, 0, sizeof(struct window_node));

    left->window_id = node->window_id;
    left->parent = node;

    right->window_id = window->id;
    right->parent = node;

    node->window_id = 0;
    node->left = left;
    node->right = right;

    area_make_pair(node);
}

static void window_node_update(struct window_node *node)
{
    if (window_node_is_intermediate(node)) {
        area_make_pair(node->parent);
    }

    if (!window_node_is_leaf(node)) {
        window_node_update(node->left);
        window_node_update(node->right);
    }
}

static void window_node_flush(struct window_node *node)
{
    if (window_node_is_occupied(node)) {
        struct ax_window *window = window_manager_find_window(&g_window_manager, node->window_id);
        if (window) {
            window_manager_move_window(window, node->area.x, node->area.y);
            window_manager_resize_window(window, node->area.w, node->area.h);
        }
    }

    if (!window_node_is_leaf(node)) {
        window_node_flush(node->left);
        window_node_flush(node->right);
    }
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

struct window_node *view_find_window_node(struct window_node *node, uint32_t window_id)
{
    if (node->window_id == window_id) return node;

    if (!window_node_is_leaf(node)) {
        struct window_node *left = view_find_window_node(node->left, window_id);
        if (left) return left;

        struct window_node *right = view_find_window_node(node->right, window_id);
        if (right) return right;
    }

    return NULL;
}

void view_remove_window_node(struct view *view, struct ax_window *window)
{
    struct window_node *node = view_find_window_node(view->root, window->id);
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

        window_node_update(parent);
    }

    free(child);
    free(node);

    if (g_space_manager.auto_balance) {
        window_node_equalize(view->root);
        view_update(view);
    }
}

void view_add_window_node(struct view *view, struct ax_window *window)
{
    if (!window_node_is_occupied(view->root) &&
        window_node_is_leaf(view->root)) {
        view->root->window_id = window->id;
    } else {
        struct window_node *leaf = view_find_window_node(view->root, g_window_manager.focused_window_id);
        if (!leaf) leaf = view_find_min_depth_leaf_node(view->root);
        window_node_split(leaf, window);

        if (g_space_manager.auto_balance) {
            window_node_equalize(view->root);
            view_update(view);
        }
    }
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

void view_update(struct view *view)
{
    uint32_t did = space_display_id(view->sid);
    CGRect frame = display_bounds_constrained(did);

    view->root->area = area_from_cgrect(frame);
    view->root->area.x += g_space_manager.left_padding;
    view->root->area.w -= (g_space_manager.left_padding + g_space_manager.right_padding);
    view->root->area.y += g_space_manager.top_padding;
    view->root->area.h -= (g_space_manager.top_padding + g_space_manager.bottom_padding);
    window_node_update(view->root);

    view->is_valid = true;
    view->is_dirty = true;
}

struct view *view_create(uint64_t sid)
{
    struct view *view = malloc(sizeof(struct view));
    memset(view, 0, sizeof(struct view));

    view->root = malloc(sizeof(struct window_node));
    memset(view->root, 0, sizeof(struct window_node));

    view->type = VIEW_MANAGED;
    view->sid = sid;
    view_update(view);

    return view;
}

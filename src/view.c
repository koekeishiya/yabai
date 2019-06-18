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
    return CHILD_RIGHT;
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
    return view->enable_gap ? view->window_gap / 2.0f : 0.0f;
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


static void window_node_split(struct view *view, struct window_node *node, struct ax_window *window)
{
    struct window_node *left = malloc(sizeof(struct window_node));
    memset(left, 0, sizeof(struct window_node));

    struct window_node *right = malloc(sizeof(struct window_node));
    memset(right, 0, sizeof(struct window_node));

    if (window_node_get_child(node) == CHILD_RIGHT) {
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

float window_node_border_window_offset(struct ax_window *window)
{
    float offset = window->border.enabled ? window->border.width : 0.0f;
    return offset;
}

void window_node_flush(struct window_node *node)
{
    if (window_node_is_occupied(node)) {
        struct ax_window *window = window_manager_find_window(&g_window_manager, node->window_id);
        if (window) {
            float offset = window_node_border_window_offset(window);
            window_manager_move_window(window, node->area.x + offset, node->area.y + offset);
            window_manager_resize_window(window, node->area.w - 2*offset, node->area.h - 2*offset);
        }
    }

    if (!window_node_is_leaf(node)) {
        window_node_flush(node->left);
        window_node_flush(node->right);
    }
}

static struct window_node *window_node_find_first_leaf(struct window_node *root)
{
    struct window_node *node = root;
    while (!window_node_is_leaf(node)) {
        node = node->left;
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

        window_node_update(view, parent);
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
        struct window_node *leaf = NULL;

        if (view->insertion_point) {
            leaf = view_find_window_node(view->root, view->insertion_point);
            view->insertion_point = 0;
        }

        if (!leaf) leaf = view_find_window_node(view->root, g_window_manager.focused_window_id);
        if (!leaf) leaf = view_find_min_depth_leaf_node(view->root);
        struct ax_window *leaf_window = window_manager_find_window(&g_window_manager, leaf->window_id);
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
        if (window_node_is_leaf(node)) {
            buf_push(window_list, node->window_id);
        }

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
    int windows = 0;
    int count = 0;
    uint32_t *window_list = space_window_list(view->sid, &count);
    if (window_list) {
        for (int i = 0; i < count; ++i) {
            if (window_manager_find_window(&g_window_manager, window_list[i])) {
                ++windows;
            }
        }
        free(window_list);
    }

    fprintf(rsp,
            "{\n"
            "\t\"id\":%lld,\n"
            "\t\"index\":%d,\n"
            "\t\"display\":%d,\n"
            "\t\"windows\":%d,\n"
            "\t\"type\":\"%s\"\n"
            "}",
            view->sid,
            space_manager_mission_control_index(view->sid),
            display_arrangement(space_display_id(view->sid)),
            windows,
            view_type_str[view->type]);
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

    int mci = space_manager_mission_control_index(sid);
    view->enable_padding = true;
    view->enable_gap = true;
    view->top_padding = view_lookup_padding(g_space_manager.top_padding, mci);
    view->bottom_padding = view_lookup_padding(g_space_manager.bottom_padding, mci);
    view->left_padding = view_lookup_padding(g_space_manager.left_padding, mci);
    view->right_padding = view_lookup_padding(g_space_manager.right_padding, mci);
    view->type = view_lookup_layout(g_space_manager.layout, mci);
    view->window_gap = view_lookup_gap(g_space_manager.window_gap, mci);
    view->sid = sid;
    view_update(view);

    return view;
}

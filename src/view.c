#include "view.h"

extern int g_connection;
extern int g_floating_window_level;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;

void insert_feedback_show(struct window_node *node)
{
    CFTypeRef frame_region;
    CGRect frame = {{(int)node->area.x, (int)node->area.y},{(int)(node->area.w+0.5f), (int)(node->area.h+0.5f)}};
    CGSNewRegionWithRect(&frame, &frame_region);

    if (!node->feedback_window.id) {
        uint64_t tags = kCGSIgnoreForExposeTagBit | kCGSIgnoreForEventsTagBit | kCGSDisableShadowTagBit;
        SLSNewWindow(g_connection, 2, 0, 0, frame_region, &node->feedback_window.id);
        SLSSetWindowTags(g_connection, node->feedback_window.id, &tags, 64);
        SLSSetWindowResolution(g_connection, node->feedback_window.id, 1.0f);
        SLSSetWindowOpacity(g_connection, node->feedback_window.id, 0);
        SLSSetWindowLevel(g_connection, node->feedback_window.id, g_floating_window_level);
        node->feedback_window.context = SLWindowContextCreate(g_connection, node->feedback_window.id, 0);
        int width = g_window_manager.enable_window_border ? g_window_manager.border_width : 2;
        CGContextSetLineWidth(node->feedback_window.context, width);
        CGContextSetRGBFillColor(node->feedback_window.context,
                                   g_window_manager.insert_feedback_color.r,
                                   g_window_manager.insert_feedback_color.g,
                                   g_window_manager.insert_feedback_color.b,
                                   g_window_manager.insert_feedback_color.a*0.25f);
        CGContextSetRGBStrokeColor(node->feedback_window.context,
                                   g_window_manager.insert_feedback_color.r,
                                   g_window_manager.insert_feedback_color.g,
                                   g_window_manager.insert_feedback_color.b,
                                   g_window_manager.insert_feedback_color.a);
        buf_push(g_window_manager.insert_feedback_windows, node->feedback_window.id);
    }

    frame.origin.x = 0; frame.origin.y = 0;
    CGFloat x1, y1, x2, y2, x3, y3, x4, y4;
    CGFloat minx = CGRectGetMinX(frame), midx = CGRectGetMidX(frame), maxx = CGRectGetMaxX(frame);
    CGFloat miny = CGRectGetMinY(frame), midy = CGRectGetMidY(frame), maxy = CGRectGetMaxY(frame);

    switch (node->insert_dir) {
    case DIR_NORTH: {
        x1 = minx; y1 = midy;
        x2 = minx; y2 = maxy;
        x3 = maxx; y3 = maxy;
        x4 = maxx; y4 = midy;
    } break;
    case DIR_EAST: {
        x1 = midx; y1 = miny;
        x2 = maxx; y2 = miny;
        x3 = maxx; y3 = maxy;
        x4 = midx; y4 = maxy;
    } break;
    case DIR_SOUTH: {
        x1 = minx; y1 = midy;
        x2 = minx; y2 = miny;
        x3 = maxx; y3 = miny;
        x4 = maxx; y4 = midy;
    } break;
    case DIR_WEST: {
        x1 = midx; y1 = miny;
        x2 = minx; y2 = miny;
        x3 = minx; y3 = maxy;
        x4 = midx; y4 = maxy;
    } break;
    case STACK: {
        x1 = minx; y1 = miny;
        x2 = minx; y2 = maxy;
        x3 = maxx; y3 = maxy;
        x4 = maxx; y4 = miny;
    } break;
    }

    CGRect fill = { {x1, y1}, { x3 - x1, y3 - y1 } };
    CGMutablePathRef outline = CGPathCreateMutable();
    CGPathMoveToPoint(outline, NULL, x1, y1);
    CGPathAddLineToPoint(outline, NULL, x2, y2);
    CGPathAddLineToPoint(outline, NULL, x3, y3);
    CGPathAddLineToPoint(outline, NULL, x4, y4);

    SLSDisableUpdate(g_connection);
    SLSOrderWindow(g_connection, node->feedback_window.id, 0, node->window_order[0]);
    SLSSetWindowShape(g_connection, node->feedback_window.id, 0.0f, 0.0f, frame_region);
    CGContextClearRect(node->feedback_window.context, frame);
    CGContextFillRect(node->feedback_window.context, fill);
    CGContextAddPath(node->feedback_window.context, outline);
    CGContextStrokePath(node->feedback_window.context);
    CGContextFlush(node->feedback_window.context);
    SLSOrderWindow(g_connection, node->feedback_window.id, 1, node->window_order[0]);
    SLSReenableUpdate(g_connection);
    CGPathRelease(outline);
    CFRelease(frame_region);
}

void insert_feedback_destroy(struct window_node *node)
{
    if (node->feedback_window.id) {
        for (int i = 0; i < buf_len(g_window_manager.insert_feedback_windows); ++i) {
            if (g_window_manager.insert_feedback_windows[i] == node->feedback_window.id) {
                buf_del(g_window_manager.insert_feedback_windows, i);
                break;
            }
        }

        CGContextRelease(node->feedback_window.context);
        SLSReleaseWindow(g_connection, node->feedback_window.id);
        memset(&node->feedback_window, 0, sizeof(struct feedback_window));
    }
}

static inline CGPoint area_center(struct area a)
{
    return (CGPoint) { a.x + a.w*0.5f, a.y + a.h*0.5f };
}

static inline struct area area_from_cgrect(CGRect rect)
{
    return (struct area) { rect.origin.x, rect.origin.y, rect.size.width, rect.size.height };
}

static inline enum window_node_child window_node_get_child(struct window_node *node)
{
    return node->child != CHILD_NONE ? node->child : g_space_manager.window_placement;
}

static inline enum window_node_split window_node_get_split(struct window_node *node)
{
    return node->split != SPLIT_NONE ? node->split : node->area.w / node->area.h >= 1.1618f ? SPLIT_Y : SPLIT_X;
}

static inline float window_node_get_ratio(struct window_node *node)
{
    return in_range_ii(node->ratio, 0.1f, 0.9f) ? node->ratio : g_space_manager.split_ratio;
}

static inline float window_node_get_gap(struct view *view)
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

static inline bool window_node_is_occupied(struct window_node *node)
{
    return node->window_count != 0;
}

static inline bool window_node_is_intermediate(struct window_node *node)
{
    return node->parent != NULL;
}

static inline bool window_node_is_leaf(struct window_node *node)
{
    return node->left == NULL && node->right == NULL;
}

static inline bool window_node_is_left_child(struct window_node *node)
{
    return node->parent && node->parent->left == node;
}

static inline bool window_node_is_right_child(struct window_node *node)
{
    return node->parent && node->parent->right == node;
}

static inline struct equalize_node equalize_node_add(struct equalize_node a, struct equalize_node b)
{
    return (struct equalize_node) { a.y_count + b.y_count, a.x_count + b.x_count, };
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
        memcpy(left->window_list, node->window_list, sizeof(uint32_t) * node->window_count);
        memcpy(left->window_order, node->window_order, sizeof(uint32_t) * node->window_count);
        left->window_count = node->window_count;
        right->window_list[0] = window->id;
        right->window_order[0] = window->id;
        right->window_count = 1;
    } else {
        memcpy(right->window_list, node->window_list, sizeof(uint32_t) * node->window_count);
        memcpy(right->window_order, node->window_order, sizeof(uint32_t) * node->window_count);
        right->window_count = node->window_count;
        left->window_list[0] = window->id;
        left->window_order[0] = window->id;
        left->window_count = 1;
    }

    left->parent  = node;
    right->parent = node;

    node->window_count = 0;
    node->left  = left;
    node->right = right;
    node->zoom  = NULL;

    area_make_pair(view, node);
}

void window_node_update(struct view *view, struct window_node *node)
{
    if (window_node_is_intermediate(node)) {
        area_make_pair(view, node->parent);
    }

    if (window_node_is_leaf(node)) {
        if (node->insert_dir) insert_feedback_show(node);
    } else {
        window_node_update(view, node->left);
        window_node_update(view, node->right);
    }
}

static void window_node_destroy(struct window_node *node)
{
    if (node->left)  window_node_destroy(node->left);
    if (node->right) window_node_destroy(node->right);

    for (int i = 0; i < node->window_count; ++i) {
        window_manager_remove_managed_window(&g_window_manager, node->window_list[i]);
    }

    insert_feedback_destroy(node);
    free(node);
}

static void window_node_clear_zoom(struct window_node *node)
{
    node->zoom = NULL;

    if (!window_node_is_leaf(node)) {
        window_node_clear_zoom(node->left);
        window_node_clear_zoom(node->right);
    }
}

void window_node_flush(struct window_node *node)
{
    if (window_node_is_occupied(node)) {
        for (int i = 0; i < node->window_count; ++i) {
            struct window *window = window_manager_find_window(&g_window_manager, node->window_list[i]);
            if (window) {
                if (node->zoom) {
                    window_manager_set_window_frame(window, node->zoom->area.x, node->zoom->area.y, node->zoom->area.w, node->zoom->area.h);
                    border_resize(window);
                } else {
                    window_manager_set_window_frame(window, node->area.x, node->area.y, node->area.w, node->area.h);
                    border_resize(window);
                }
            }
        }
    }

    if (!window_node_is_leaf(node)) {
        window_node_flush(node->left);
        window_node_flush(node->right);
    }
}

bool window_node_contains_window(struct window_node *node, uint32_t window_id)
{
    for (int i = 0; i < node->window_count; ++i) {
        if (node->window_list[i] == window_id) return true;
    }

    return false;
}

int window_node_index_of_window(struct window_node *node, uint32_t window_id)
{
    for (int i = 0; i < node->window_count; ++i) {
        if (node->window_list[i] == window_id) return i;
    }

    return 0;
}

void window_node_swap_window_list(struct window_node *a_node, struct window_node *b_node)
{
    uint32_t tmp_window_list[NODE_MAX_WINDOW_COUNT];
    uint32_t tmp_window_order[NODE_MAX_WINDOW_COUNT];
    uint32_t tmp_window_count;

    memcpy(tmp_window_list, a_node->window_list, sizeof(uint32_t) * a_node->window_count);
    memcpy(tmp_window_order, a_node->window_order, sizeof(uint32_t) * a_node->window_count);
    tmp_window_count = a_node->window_count;

    memcpy(a_node->window_list, b_node->window_list, sizeof(uint32_t) * b_node->window_count);
    memcpy(a_node->window_order, b_node->window_order, sizeof(uint32_t) * b_node->window_count);
    a_node->window_count = b_node->window_count;

    memcpy(b_node->window_list, tmp_window_list, sizeof(uint32_t) * tmp_window_count);
    memcpy(b_node->window_order, tmp_window_order, sizeof(uint32_t) * tmp_window_count);
    b_node->window_count = tmp_window_count;

    a_node->zoom = NULL;
    b_node->zoom = NULL;
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

struct window_node *view_find_window_node_in_direction(struct view *view, struct window_node *source, int direction)
{
    int best_distance = INT_MAX;
    struct window_node *best_node = NULL;
    CGPoint source_point = area_center(source->area);

    struct window_node *target = window_node_find_first_leaf(view->root);
    while (target) {
        CGPoint target_point = area_center(target->area);
        int distance = euclidean_distance(source_point, target_point);
        if (distance >= best_distance) goto next;

        switch (direction) {
        case DIR_EAST: {
            if (target->area.x >= source->area.x + source->area.w) {
                best_node = target;
                best_distance = distance;
            }
        } break;
        case DIR_SOUTH: {
            if (target->area.y >= source->area.y + source->area.h) {
                best_node = target;
                best_distance = distance;
            }
        } break;
        case DIR_WEST: {
            if (target->area.x + target->area.w <= source->area.x) {
                best_node = target;
                best_distance = distance;
            }
        } break;
        case DIR_NORTH: {
            if (target->area.y + target->area.h <= source->area.y) {
                best_node = target;
                best_distance = distance;
            }
        } break;
        }

next:
        target = window_node_find_next_leaf(target);
    }

    return best_node;
}

struct window_node *view_find_window_node(struct view *view, uint32_t window_id)
{
    struct window_node *node = window_node_find_first_leaf(view->root);
    while (node) {
        if (window_node_contains_window(node, window_id)) return node;
        node = window_node_find_next_leaf(node);
    }

    return NULL;
}

void view_remove_window_node(struct view *view, struct window *window)
{
    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return;

    if (node->window_count > 1) {
        bool removed_entry = false;
        bool removed_order = false;

        for (int i = 0; i < node->window_count; ++i) {
            if (!removed_entry && node->window_list[i] == window->id) {
                memmove(node->window_list + i, node->window_list + i + 1, sizeof(uint32_t) * (node->window_count - i - 1));
                removed_entry = true;
            }

            if (!removed_order && node->window_order[i] == window->id) {
                memmove(node->window_order + i, node->window_order + i + 1, sizeof(uint32_t) * (node->window_count - i - 1));
                removed_order = true;
            }
        }

        assert(removed_entry);
        assert(removed_order);
        --node->window_count;

        return;
    }

    if (node == view->root) {
        node->window_count = 0;
        insert_feedback_destroy(node);
        node->split = SPLIT_NONE;
        node->child = CHILD_NONE;
        node->insert_dir = 0;
        view->insertion_point = 0;
        return;
    }

    struct window_node *parent = node->parent;
    struct window_node *child  = window_node_is_right_child(node)
                               ? parent->left
                               : parent->right;


    memcpy(parent->window_list, child->window_list, sizeof(uint32_t) * child->window_count);
    memcpy(parent->window_order, child->window_order, sizeof(uint32_t) * child->window_count);
    parent->window_count = child->window_count;

    parent->left      = NULL;
    parent->right     = NULL;
    parent->zoom      = NULL;

    if (child->insert_dir) {
        parent->feedback_window = child->feedback_window;
        parent->insert_dir      = child->insert_dir;
        parent->split           = child->split;
        parent->child           = child->child;
        insert_feedback_show(parent);
    }

    if (window_node_is_intermediate(child) && !window_node_is_leaf(child)) {
        parent->left = child->left;
        parent->left->parent = parent;
        parent->left->zoom = NULL;

        parent->right = child->right;
        parent->right->parent = parent;
        parent->right->zoom = NULL;

        window_node_clear_zoom(parent);
        window_node_update(view, parent);
    }

    insert_feedback_destroy(node);

    free(child);
    free(node);

    if (g_space_manager.auto_balance) {
        window_node_equalize(view->root);
        view_update(view);
    }
}

void view_stack_window_node(struct view *view, struct window_node *node, struct window *window)
{
    if (node->zoom) {
        window_manager_set_window_frame(window, node->zoom->area.x, node->zoom->area.y, node->zoom->area.w, node->zoom->area.h);
    } else {
        window_manager_set_window_frame(window, node->area.x, node->area.y, node->area.w, node->area.h);
    }

    node->window_list[node->window_count] = window->id;
    node->window_order[node->window_count] = window->id;
    ++node->window_count;
}

void view_add_window_node(struct view *view, struct window *window)
{
    if (!window_node_is_occupied(view->root) &&
        window_node_is_leaf(view->root)) {
        view->root->window_list[0] = window->id;
        view->root->window_order[0] = window->id;
        view->root->window_count = 1;
    } else if (view->layout == VIEW_BSP) {
        struct window_node *leaf = NULL;

        if (view->insertion_point) {
            leaf = view_find_window_node(view, view->insertion_point);
            view->insertion_point = 0;

            if (leaf) {
                bool do_stack = leaf->insert_dir == STACK;

                leaf->insert_dir = 0;
                insert_feedback_destroy(leaf);

                if (do_stack) {
                    view_stack_window_node(view, leaf, window);
                    return;
                }
            }
        }

        if (!leaf) leaf = view_find_window_node(view, g_window_manager.focused_window_id);
        if (!leaf) leaf = view_find_min_depth_leaf_node(view->root);

        window_node_split(view, leaf, window);

        if (g_space_manager.auto_balance) {
            window_node_equalize(view->root);
            view_update(view);
        }
    } else if (view->layout == VIEW_STACK) {
        view_stack_window_node(view, view->root, window);
    }
}

uint32_t *view_find_window_list(struct view *view, int *window_count)
{
    *window_count = 0;

    int capacity = 13;
    uint32_t *window_list = ts_alloc(sizeof(uint32_t) * capacity);

    struct window_node *node = window_node_find_first_leaf(view->root);
    while (node) {
        if (*window_count + node->window_count >= capacity) {
            ts_expand(window_list, sizeof(uint32_t) * capacity, sizeof(uint32_t) * capacity);
            capacity *= 2;
        }

        for (int i = 0; i < node->window_count; ++i) {
            window_list[(*window_count)++] = node->window_list[i];
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
    int buffer_size = MAXLEN;
    size_t bytes_written = 0;
    char buffer[MAXLEN] = {};
    char *cursor = buffer;

    int count = 0;
    uint32_t windows[MAXLEN] = {};

    int window_count = 0;
    uint32_t *window_list = space_window_list(view->sid, &window_count, true);
    if (window_list) {
        for (int i = 0; i < window_count; ++i) {
            if (window_manager_find_window(&g_window_manager, window_list[i])) {
                windows[count++] = window_list[i];
            }
        }
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
            first_leaf ? first_leaf->window_order[0] : 0,
            last_leaf ? last_leaf->window_order[0] : 0);
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
    view->suuid = space_uuid(sid);

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

        for (int i = 0; i < view->root->window_count; ++i) {
            window_manager_remove_managed_window(&g_window_manager, view->root->window_list[i]);
        }

        insert_feedback_destroy(view->root);
        memset(view->root, 0, sizeof(struct window_node));
        view_update(view);
    }
}

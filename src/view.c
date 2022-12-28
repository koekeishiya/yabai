extern int g_connection;
extern int g_floating_window_level;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;

void insert_feedback_show(struct window_node *node)
{
    CFTypeRef frame_region;
    CGRect frame = {{node->area.x, node->area.y},{node->area.w, node->area.h}};
    CGSNewRegionWithRect(&frame, &frame_region);

    if (!node->feedback_window.id) {
        uint64_t tag = 1ULL << 46;
        SLSNewWindow(g_connection, 2, 0, 0, frame_region, &node->feedback_window.id);
        SLSSetWindowTags(g_connection, node->feedback_window.id, &tag, 64);
        sls_window_disable_shadow(node->feedback_window.id);
        SLSSetWindowResolution(g_connection, node->feedback_window.id, 2.0f);
        SLSSetWindowOpacity(g_connection, node->feedback_window.id, 0);
        SLSSetWindowLevel(g_connection, node->feedback_window.id, g_floating_window_level);
        node->feedback_window.context = SLWindowContextCreate(g_connection, node->feedback_window.id, 0);
        CGContextSetLineWidth(node->feedback_window.context, g_window_manager.border_width);
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
    CGFloat clip_x, clip_y, clip_w, clip_h;
    CGFloat midx = CGRectGetMidX(frame);
    CGFloat midy = CGRectGetMidY(frame);

    switch (node->insert_dir) {
    case DIR_NORTH: {
        clip_x = -0.5f * g_window_manager.border_width;
        clip_y = midy - 0.5f * g_window_manager.border_width;
        clip_w = g_window_manager.border_width;
        clip_h = g_window_manager.border_width;
    } break;
    case DIR_EAST: {
        clip_x = midx - 0.5f * g_window_manager.border_width;
        clip_y = -0.5f * g_window_manager.border_width;
        clip_w = g_window_manager.border_width;
        clip_h = g_window_manager.border_width;
    } break;
    case DIR_SOUTH: {
        clip_x = -0.5f * g_window_manager.border_width;
        clip_y = -0.5f * g_window_manager.border_width;
        clip_w = g_window_manager.border_width;
        clip_h = -midy + g_window_manager.border_width;
    } break;
    case DIR_WEST: {
        clip_x = -0.5f * g_window_manager.border_width;
        clip_y = -0.5f * g_window_manager.border_width;
        clip_w = -midx + g_window_manager.border_width;
        clip_h = g_window_manager.border_width;
    } break;
    case STACK: {
        clip_x = -0.5f * g_window_manager.border_width;
        clip_y = -0.5f * g_window_manager.border_width;
        clip_w = g_window_manager.border_width;
        clip_h = g_window_manager.border_width;
    } break;
    }

    CGRect rect = (CGRect) {{ 0.5f*g_window_manager.border_width, 0.5f*g_window_manager.border_width }, { frame.size.width - g_window_manager.border_width, frame.size.height - g_window_manager.border_width }};
    CGRect fill = CGRectInset(rect, 0.5f*g_window_manager.border_width, 0.5f*g_window_manager.border_width);
    CGRect clip = { { rect.origin.x + clip_x, rect.origin.y + clip_y }, { rect.size.width + clip_w, rect.size.height + clip_h } };
    CGPathRef path = CGPathCreateWithRoundedRect(rect, cgrect_clamp_x_radius(rect, g_window_manager.border_radius), cgrect_clamp_y_radius(rect, g_window_manager.border_radius), NULL);

    SLSDisableUpdate(g_connection);
    SLSOrderWindow(g_connection, node->feedback_window.id, 0, node->window_order[0]);
    SLSSetWindowShape(g_connection, node->feedback_window.id, 0.0f, 0.0f, frame_region);
    CGContextClearRect(node->feedback_window.context, frame);
    CGContextClipToRect(node->feedback_window.context, clip);
    CGContextFillRect(node->feedback_window.context, fill);
    CGContextAddPath(node->feedback_window.context, path);
    CGContextStrokePath(node->feedback_window.context);
    CGContextResetClip(node->feedback_window.context);
    CGContextFlush(node->feedback_window.context);
    SLSOrderWindow(g_connection, node->feedback_window.id, 1, node->window_order[0]);
    SLSReenableUpdate(g_connection);
    CGPathRelease(path);
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
    if (node->split                != SPLIT_NONE) return node->split;
    if (g_space_manager.split_type != SPLIT_AUTO) return g_space_manager.split_type;
    return node->area.w >= node->area.h ? SPLIT_Y : SPLIT_X;
}

static inline float window_node_get_ratio(struct window_node *node)
{
    return in_range_ii(node->ratio, 0.1f, 0.9f) ? node->ratio : g_space_manager.split_ratio;
}

static inline float window_node_get_gap(struct view *view)
{
    return view->enable_gap ? view->window_gap*0.5f : 0.0f;
}

#define area_ax_truncate(a)  \
    a->x = (int)(a->x + 0.5f); \
    a->y = (int)(a->y + 0.5f); \
    a->w = (int)(a->w + 0.5f); \
    a->h = (int)(a->h + 0.5f);

static void area_make_pair(enum window_node_split split, float gap, float ratio, struct area *parent_area, struct area *left_area, struct area *right_area)
{
    if (split == SPLIT_Y) {
        *left_area = *parent_area;
        left_area->w *= ratio;
        left_area->w -= gap;

        *right_area = *parent_area;
        right_area->x += (parent_area->w * ratio);
        right_area->w *= (1 - ratio);
        right_area->x += gap;
        right_area->w -= gap;
    } else {
        *left_area = *parent_area;
        left_area->h *= ratio;
        left_area->h -= gap;

        *right_area = *parent_area;
        right_area->y += (parent_area->h * ratio);
        right_area->h *= (1 - ratio);
        right_area->y += gap;
        right_area->h -= gap;
    }

    area_ax_truncate(left_area);
    area_ax_truncate(right_area);
}

static void area_make_pair_for_node(struct view *view, struct window_node *node)
{
    enum window_node_split split = window_node_get_split(node);
    float ratio = window_node_get_ratio(node);
    float gap   = window_node_get_gap(view);

    area_make_pair(split, gap, ratio, &node->area, &node->left->area, &node->right->area);

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

static struct equalize_node window_node_equalize(struct window_node *node, uint32_t axis_flag)
{
    if (window_node_is_leaf(node)) {
        return (struct equalize_node) {
            node->parent ? node->parent->split == SPLIT_Y : 0,
            node->parent ? node->parent->split == SPLIT_X : 0
        };
    }

    struct equalize_node left_leafs  = window_node_equalize(node->left, axis_flag);
    struct equalize_node right_leafs = window_node_equalize(node->right, axis_flag);
    struct equalize_node total_leafs = equalize_node_add(left_leafs, right_leafs);

    if (axis_flag & SPLIT_Y) {
        if (node->split == SPLIT_Y) {
            node->ratio = (float) left_leafs.y_count / total_leafs.y_count;
            --total_leafs.y_count;
        }
    }

    if (axis_flag & SPLIT_X) {
        if (node->split == SPLIT_X) {
            node->ratio = (float) left_leafs.x_count / total_leafs.x_count;
            --total_leafs.x_count;
        }
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

    struct window_node *zoom = !g_space_manager.window_zoom_persist
                             ? NULL
                             : !node->zoom
                             ? NULL
                             : node->zoom == node->parent
                             ? node
                             : view->root;

    if (window_node_get_child(node) == CHILD_SECOND) {
        memcpy(left->window_list, node->window_list, sizeof(uint32_t) * node->window_count);
        memcpy(left->window_order, node->window_order, sizeof(uint32_t) * node->window_count);
        left->window_count = node->window_count;
        left->zoom = zoom;

        right->window_list[0] = window->id;
        right->window_order[0] = window->id;
        right->window_count = 1;
    } else {
        memcpy(right->window_list, node->window_list, sizeof(uint32_t) * node->window_count);
        memcpy(right->window_order, node->window_order, sizeof(uint32_t) * node->window_count);
        right->window_count = node->window_count;
        right->zoom = zoom;

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

    area_make_pair_for_node(view, node);
}

void window_node_update(struct view *view, struct window_node *node)
{
    if (window_node_is_intermediate(node)) {
        area_make_pair_for_node(view, node->parent);
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

void window_node_capture_windows(struct window_node *node, struct window_capture **window_list)
{
    if (window_node_is_occupied(node)) {
        for (int i = 0; i < node->window_count; ++i) {
            struct window *window = window_manager_find_window(&g_window_manager, node->window_list[i]);
            if (window) {
                struct area area = node->zoom ? node->zoom->area : node->area;
                if (window->border.id) {
                    area.x += g_window_manager.border_width;
                    area.y += g_window_manager.border_width;
                    area.w -= g_window_manager.border_width * 2.0f;
                    area.h -= g_window_manager.border_width * 2.0f;
                }
                ts_buf_push(*window_list, ((struct window_capture) { .window = window, .x = area.x, .y = area.y, .w = area.w, .h = area.h }));
            }
        }
    }

    if (!window_node_is_leaf(node)) {
        window_node_capture_windows(node->left, window_list);
        window_node_capture_windows(node->right, window_list);
    }
}

void window_node_flush(struct window_node *node)
{
    struct window_capture *window_list = NULL;
    window_node_capture_windows(node, &window_list);
    if (window_list) window_manager_animate_window_list(window_list, ts_buf_len(window_list));
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

    return window_node_find_last_leaf(node->parent->left->right);
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

static inline bool area_is_in_direction(struct area *r1, CGPoint r1_max, struct area *r2, CGPoint r2_max, int direction)
{
    if (direction == DIR_NORTH && r1_max.y <= r2->y) return false;
    if (direction == DIR_EAST  && r2_max.x <= r1->x) return false;
    if (direction == DIR_SOUTH && r2_max.y <= r1->y) return false;
    if (direction == DIR_WEST  && r1_max.x <= r2->x) return false;

    if (direction == DIR_NORTH || direction == DIR_SOUTH) {
        return ((r2_max.x >  r1->x && r2_max.x <= r1_max.x) ||
                (r2->x    <  r1->x && r2_max.x >  r1_max.x) ||
                (r2->x    >= r1->x && r2->x    <  r1_max.x));
    }

    if (direction == DIR_EAST || direction == DIR_WEST) {
        return ((r2_max.y >  r1->y && r2_max.y <= r1_max.y) ||
                (r2->y    <  r1->y && r2_max.y >  r1_max.y) ||
                (r2->y    >= r1->y && r2->y    <  r1_max.y));
    }

    return false;
}

static inline int area_distance_in_direction(struct area *r1, CGPoint r1_max, struct area *r2, CGPoint r2_max, int direction)
{
    switch (direction) {
    case DIR_NORTH: {
        return r2_max.y > r1->y ? r2_max.y - r1->y : r1->y - r2_max.y;
    } break;
    case DIR_EAST: {
        return r2->x < r1_max.x ? r1_max.x - r2->x : r2->x - r1_max.x;
    } break;
    case DIR_SOUTH: {
        return r2->y < r1_max.y ? r1_max.y - r2->y : r2->y - r1_max.y;
    } break;
    case DIR_WEST: {
        return r2_max.x > r1->x ? r2_max.x - r1->x : r1->x - r2_max.x;
    } break;
    }

    return INT_MAX;
}

struct window_node *view_find_window_node_in_direction(struct view *view, struct window_node *source, int direction)
{
    int window_count;
    uint32_t *window_list = space_window_list(view->sid, &window_count, false);
    if (!window_list) return NULL;

    int best_distance = INT_MAX;
    int best_rank = INT_MAX;
    struct window_node *best_node = NULL;

    CGPoint source_area_max = { source->area.x + source->area.w, source->area.y + source->area.h };

    struct window_node *target = window_node_find_first_leaf(view->root);
    while (target) {
        if (source == target) goto next;

        CGPoint target_area_max = { target->area.x + target->area.w, target->area.y + target->area.h };
        if (area_is_in_direction(&source->area, source_area_max, &target->area, target_area_max, direction)) {
            int distance = area_distance_in_direction(&source->area, source_area_max, &target->area, target_area_max, direction);
            int rank = window_manager_find_rank_of_window_in_list(target->window_order[0], window_list, window_count);
            if ((distance < best_distance) || (distance == best_distance && rank < best_rank)) {
                best_node = target;
                best_distance = distance;
                best_rank = rank;
            }
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

struct window_node *view_remove_window_node(struct view *view, struct window *window)
{
    struct window_node *node = view_find_window_node(view, window->id);
    if (!node) return NULL;

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

        if (view->insertion_point == window->id) {
            view->insertion_point = node->window_order[0];
        }

        return NULL;
    }

    if (node == view->root) {
        view->insertion_point = 0;
        insert_feedback_destroy(node);
        memset(node, 0, sizeof(struct window_node));
        view_update(view);
        return NULL;
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
    parent->zoom      = !g_space_manager.window_zoom_persist
                      ? NULL
                      : !child->zoom
                      ? NULL
                      : child->zoom == parent
                      ? parent->parent
                      : view->root;

    if (child->insert_dir) {
        parent->feedback_window = child->feedback_window;
        parent->insert_dir      = child->insert_dir;
        parent->split           = child->split;
        parent->child           = child->child;
        insert_feedback_show(parent);
    }

    if (window_node_is_intermediate(child) && !window_node_is_leaf(child)) {
        parent->left          = child->left;
        parent->left->parent  = parent;
        parent->left->zoom    = !g_space_manager.window_zoom_persist
                              ? NULL
                              : !child->left->zoom
                              ? NULL
                              : child->left->zoom == child
                              ? parent
                              : view->root;

        parent->right         = child->right;
        parent->right->parent = parent;
        parent->right->zoom   = !g_space_manager.window_zoom_persist
                              ? NULL
                              : !child->right->zoom
                              ? NULL
                              : child->right->zoom == child
                              ? parent
                              : view->root;

        if (!g_space_manager.window_zoom_persist) {
            window_node_clear_zoom(parent);
        }

        window_node_update(view, parent->left);
        window_node_update(view, parent->right);
    }

    insert_feedback_destroy(node);

    free(child);
    free(node);

    if (g_space_manager.auto_balance) {
        window_node_equalize(view->root, SPLIT_X | SPLIT_Y);
        view_update(view);
        return view->root;
    }

    return parent;
}

void view_stack_window_node(struct view *view, struct window_node *node, struct window *window)
{
    node->window_list[node->window_count]  = window->id;
    memmove(node->window_order + 1, node->window_order, sizeof(uint32_t) * node->window_count);
    node->window_order[0] = window->id;
    ++node->window_count;
}

struct window_node *view_add_window_node_with_insertion_point(struct view *view, struct window *window, uint32_t insertion_point)
{
    if (!window_node_is_occupied(view->root) &&
        window_node_is_leaf(view->root)) {
        view->root->window_list[0] = window->id;
        view->root->window_order[0] = window->id;
        view->root->window_count = 1;
        return view->root;
    } else if (view->layout == VIEW_BSP) {
        uint32_t prev_insertion_point = 0;
        struct window_node *leaf = NULL;

        if (insertion_point) {
            prev_insertion_point = view->insertion_point;
            view->insertion_point = insertion_point;
        }

        if (view->insertion_point) {
            leaf = view_find_window_node(view, view->insertion_point);
            view->insertion_point = prev_insertion_point;

            if (leaf) {
                bool do_stack = leaf->insert_dir == STACK;

                leaf->insert_dir = 0;
                insert_feedback_destroy(leaf);

                if (do_stack) {
                    view_stack_window_node(view, leaf, window);
                    return leaf;
                }
            }
        }

        if (!leaf) leaf = view_find_window_node(view, g_window_manager.focused_window_id);
        if (!leaf) leaf = view_find_min_depth_leaf_node(view->root);

        window_node_split(view, leaf, window);

        if (g_space_manager.auto_balance) {
            window_node_equalize(view->root, SPLIT_X | SPLIT_Y);
            view_update(view);
            return view->root;
        }

        return leaf;
    } else if (view->layout == VIEW_STACK) {
        view_stack_window_node(view, view->root, window);
        return view->root;
    }

    return NULL;
}

struct window_node *view_add_window_node(struct view *view, struct window *window)
{
    return view_add_window_node_with_insertion_point(view, window, 0);
}

uint32_t *view_find_window_list(struct view *view, int *window_count)
{
    *window_count = 0;

    int capacity = 13;
    uint32_t *window_list = ts_alloc_aligned(sizeof(uint32_t), capacity);

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
    char *uuid = ts_cfstring_copy(view->suuid);
    int buffer_size = MAXLEN;
    size_t bytes_written = 0;
    char buffer[MAXLEN] = {};
    char *cursor = buffer;

    int window_count = 0;
    uint32_t *window_list = space_window_list(view->sid, &window_count, true);

    for (int i = 0; i < window_count; ++i) {
        if (i < window_count - 1) {
            bytes_written = snprintf(cursor, buffer_size, "%d, ", window_list[i]);
        } else {
            bytes_written = snprintf(cursor, buffer_size, "%d", window_list[i]);
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
            "\t\"uuid\":\"%s\",\n"
            "\t\"index\":%d,\n"
            "\t\"label\":\"%s\",\n"
            "\t\"type\":\"%s\",\n"
            "\t\"display\":%d,\n"
            "\t\"windows\":[%s],\n"
            "\t\"first-window\":%d,\n"
            "\t\"last-window\":%d,\n"
            "\t\"has-focus\":%s,\n"
            "\t\"is-visible\":%s,\n"
            "\t\"is-native-fullscreen\":%s\n"
            "}",
            view->sid,
            uuid ? uuid : "<unknown>",
            space_manager_mission_control_index(view->sid),
            space_label ? space_label->label : "",
            view_type_str[view->layout],
            display_arrangement(space_display_id(view->sid)),
            buffer,
            first_leaf ? first_leaf->window_order[0] : 0,
            last_leaf ? last_leaf->window_order[0] : 0,
            json_bool(view->sid == g_space_manager.current_space_id),
            json_bool(space_is_visible(view->sid)),
            json_bool(space_is_fullscreen(view->sid)));
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
    view->suuid = SLSSpaceCopyName(g_connection, sid);

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

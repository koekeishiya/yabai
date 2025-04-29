#include "layout.h"
#include "../window_manager.h"

static void apply_bsp_layout(struct space *space, struct window_node *node, struct window_region screen, layout_result **results, int *count)
{
    if (!node || !results || !count)
        return;

    if (node->window_id)
    {
        layout_result result = {
            .window_id = node->window_id,
            .region = screen};
        (*results)[(*count)++] = result;
    }

    if (node->split_type == SPLIT_VERTICAL)
    {
        struct window_region left = screen;
        struct window_region right = screen;

        left.w = (int)(screen.w * node->split_ratio);
        right.x += left.w;
        right.w -= left.w;

        apply_bsp_layout(space, node->left, left, results, count);
        apply_bsp_layout(space, node->right, right, results, count);
    }
    else if (node->split_type == SPLIT_HORIZONTAL)
    {
        struct window_region top = screen;
        struct window_region bottom = screen;

        top.h = (int)(screen.h * node->split_ratio);
        bottom.y += top.h;
        bottom.h -= top.h;

        apply_bsp_layout(space, node->left, top, results, count);
        apply_bsp_layout(space, node->right, bottom, results, count);
    }
}

static layout_func bsp_layout = {
    .name = "bsp",
    .apply = apply_bsp_layout};

__attribute__((constructor)) static void register_bsp_layout()
{
    layout_register("bsp", &bsp_layout);
}
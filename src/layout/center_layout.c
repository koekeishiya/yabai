#include "layout.h"
#include "../window_manager.h"

static void apply_center_layout(struct space *space, struct window_node *node, struct window_region screen, layout_result **results, int *count)
{
    if (!node || !results || !count)
        return;

    if (node->window_id)
    {
        int pad_x = 100;
        int pad_y = 80;
        struct window_region centered = {
            .x = screen.x + pad_x,
            .y = screen.y + pad_y,
            .w = screen.w - (2 * pad_x),
            .h = screen.h - (2 * pad_y)};

        layout_result result = {
            .window_id = node->window_id,
            .region = centered};
        (*results)[(*count)++] = result;
    }
}

static layout_func center_layout = {
    .name = "center",
    .apply = apply_center_layout};

__attribute__((constructor)) static void register_center_layout()
{
    layout_register("center", &center_layout);
}

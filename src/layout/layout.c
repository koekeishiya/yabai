#include "layout.h"
#include "../misc/memory_pool.h"
#include <string.h>

#define MAX_LAYOUTS 8

static layout_func layout_registry[MAX_LAYOUTS];
static int layout_count = 0;

void layout_register(const char *name, layout_func *func)
{
    if (layout_count >= MAX_LAYOUTS)
        return;

    for (int i = 0; i < layout_count; ++i)
    {
        if (strcmp(layout_registry[i].name, name) == 0)
            return;
    }

    layout_registry[layout_count++] = *func;
}

layout_func *layout_get(const char *name)
{
    for (int i = 0; i < layout_count; ++i)
    {
        if (strcmp(layout_registry[i].name, name) == 0)
            return &layout_registry[i];
    }
    return NULL;
}

void layout_apply(const char *name, struct space *space, struct window_node *node, struct window_region screen)
{
    layout_func *func = layout_get(name);
    if (!func || !func->apply)
        return;

    layout_result *results = memory_pool_push(memory_pool, sizeof(layout_result) * 128); // arbitrary max
    int result_count = 0;

    func->apply(space, node, screen, &results, &result_count);

    for (int i = 0; i < result_count; ++i)
    {
        struct window *win = window_manager_find_window_by_id(results[i].window_id);
        if (win)
            window_set_region(win, results[i].region);
    }
}
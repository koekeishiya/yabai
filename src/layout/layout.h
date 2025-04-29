// src/layout/layout.h

#pragma once
#include "../window.h"
#include "../space.h"
#include "../display.h"
#include "../types.h"

typedef struct layout_result
{
    uint32_t window_id;
    struct window_region region;
} layout_result;

typedef struct layout_func
{
    const char *name;
    void (*apply)(struct space *space, struct window_node *node, struct window_region screen, layout_result **results, int *count);
} layout_func;

void layout_register(const char *name, layout_func *func);
layout_func *layout_get(const char *name);
void layout_apply(const char *name, struct space *space, struct window_node *node, struct window_region screen);
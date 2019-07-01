#ifndef WINDOW_GROUP_H
#define WINDOW_GROUP_H

struct window_group
{
    AXUIElementRef ref;
    uint32_t id;
    uint32_t *window_list;
    uint32_t active_window_id;
};

static uint32_t g_group_id_seed;

uint32_t window_group_find_active_window(struct window_group *group);
bool window_group_remove_window(struct window_group *group, uint32_t wid);
bool window_group_find_window(struct window_group *group, uint32_t wid);
void window_group_add_window(struct window_group *group, uint32_t wid);
struct window_group *window_group_create(AXUIElementRef ref);
void window_group_destroy(struct window_group *group);

#endif

#include "window_group.h"

extern uint32_t g_group_id_seed;

uint32_t window_group_find_active_window(struct window_group *group)
{
    uint32_t window_id   = 0;
    CFTypeRef window_ref = NULL;

    AXUIElementCopyAttributeValue(group->ref, kAXWindowAttribute, &window_ref);
    if (!window_ref) goto out;

    window_id = ax_window_id(window_ref);
    CFRelease(window_ref);

out:
    return window_id;
}

bool window_group_remove_window(struct window_group *group, uint32_t wid)
{
    for (int i = 0; i < buf_len(group->window_list); ++i) {
        if (group->window_list[i] == wid) {
            buf_del(group->window_list, i);
            return true;
        }
    }

    return false;
}

bool window_group_find_window(struct window_group *group, uint32_t wid)
{
    for (int i = 0; i < buf_len(group->window_list); ++i) {
        if (group->window_list[i] == wid) {
            return true;
        }
    }

    return false;
}

void window_group_add_window(struct window_group *group, uint32_t wid)
{
    buf_push(group->window_list, wid);
}

struct window_group *window_group_create(AXUIElementRef ref)
{
    struct window_group *group = malloc(sizeof(struct window_group));
    memset(group, 0, sizeof(struct window_group));

    group->id = ++g_group_id_seed;
    group->window_list = NULL;
    group->ref = ref;

    return group;
}

void window_group_destroy(struct window_group *group)
{
    CFRelease(group->ref);
    buf_free(group->window_list);
    free(group);
}

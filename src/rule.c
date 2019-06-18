#include "rule.h"

extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;

void rule_apply(struct rule *rule)
{
    for (int window_index = 0; window_index < g_window_manager.window.capacity; ++window_index) {
        struct bucket *bucket = g_window_manager.window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct ax_window *window = bucket->value;
                window_manager_apply_rule_to_window(&g_space_manager, &g_window_manager, window, rule);
            }

            bucket = bucket->next;
        }
    }
}

bool rule_remove(char *label)
{
    for (int i = 0; i < buf_len(g_window_manager.rules); ++i) {
        if (string_equals(g_window_manager.rules[i]->label, label)) {
            rule_destroy(g_window_manager.rules[i]);
            buf_del(g_window_manager.rules, i);
            return true;
        }
    }

    return false;
}

void rule_add(struct rule *rule)
{
    if (rule->label) rule_remove(rule->label);
    buf_push(g_window_manager.rules, rule);
    rule_apply(rule);
}

bool rule_is_valid(struct rule *rule)
{
    return rule->app_regex_valid || rule->title_regex_valid;
}

struct rule *rule_create(void)
{
    struct rule *rule = malloc(sizeof(struct rule));
    memset(rule, 0, sizeof(struct rule));
    return rule;
}

void rule_destroy(struct rule *rule)
{
    if (rule->app_regex_valid)   regfree(&rule->app_regex);
    if (rule->title_regex_valid) regfree(&rule->title_regex);
    if (rule->label) free(rule->label);
    free(rule);
}

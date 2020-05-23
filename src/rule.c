#include "rule.h"

extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;

void rule_serialize(FILE *rsp, struct rule *rule, int index)
{
    fprintf(rsp,
            "{\n"
            "\t\"index\":%d,\n"
            "\t\"label\":\"%s\",\n"
            "\t\"app\":\"%s\",\n"
            "\t\"title\":\"%s\",\n"
            "\t\"display_id\":%d,\n"
            "\t\"space_id\":%lld,\n"
            "\t\"follow_space\":%d,\n"
            "\t\"opacity\":%.4f,\n"
            "\t\"manage\":%d,\n"
            "\t\"sticky\":%d,\n"
            "\t\"layer\":\"%s\",\n"
            "\t\"native-fullscreen\":%d,\n"
            "\t\"grid\":\"%d:%d:%d:%d:%d:%d\"\n"
            "}",
            index,
            rule->label ? rule->label : "",
            rule->app ? rule->app : "",
            rule->title ? rule->title : "",
            rule->did,
            rule->sid,
            rule->follow_space,
            rule->alpha,
            rule_prop[rule->manage],
            rule_prop[rule->sticky],
            layer_str[rule->layer],
            rule_prop[rule->fullscreen],
            rule->grid[0], rule->grid[1],
            rule->grid[2], rule->grid[3],
            rule->grid[4], rule->grid[5]);
}

void rule_apply(struct rule *rule)
{
    for (int window_index = 0; window_index < g_window_manager.window.capacity; ++window_index) {
        struct bucket *bucket = g_window_manager.window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                window_manager_apply_rule_to_window(&g_space_manager, &g_window_manager, window, rule);
            }

            bucket = bucket->next;
        }
    }
}

bool rule_remove_by_index(int index)
{
    for (int i = 0; i < buf_len(g_window_manager.rules); ++i) {
        if (i == index) {
            rule_destroy(&g_window_manager.rules[i]);
            buf_del(g_window_manager.rules, i);
            return true;
        }
    }

    return false;
}

bool rule_remove(char *label)
{
    for (int i = 0; i < buf_len(g_window_manager.rules); ++i) {
        if (string_equals(g_window_manager.rules[i].label, label)) {
            rule_destroy(&g_window_manager.rules[i]);
            buf_del(g_window_manager.rules, i);
            return true;
        }
    }

    return false;
}

void rule_add(struct rule *rule)
{
    if (rule->label) rule_remove(rule->label);
    buf_push(g_window_manager.rules, *rule);
    rule_apply(rule);
}

void rule_destroy(struct rule *rule)
{
    if (rule->app_regex_valid)   regfree(&rule->app_regex);
    if (rule->title_regex_valid) regfree(&rule->title_regex);
    if (rule->label) free(rule->label);
    if (rule->app)   free(rule->app);
    if (rule->title) free(rule->title);
}

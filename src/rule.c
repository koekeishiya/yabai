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
            "\t\"role\":\"%s\",\n"
            "\t\"subrole\":\"%s\",\n"
            "\t\"display\":%d,\n"
            "\t\"space\":%d,\n"
            "\t\"follow_space\":%s,\n"
            "\t\"opacity\":%.4f,\n"
            "\t\"manage\":%s,\n"
            "\t\"sticky\":%s,\n"
            "\t\"mouse_follows_focus\":%s,\n"
            "\t\"layer\":\"%s\",\n"
            "\t\"border\":%s,\n"
            "\t\"native-fullscreen\":%s,\n"
            "\t\"grid\":\"%d:%d:%d:%d:%d:%d\"\n"
            "}",
            index,
            rule->label ? rule->label : "",
            rule->app ? rule->app : "",
            rule->title ? rule->title : "",
            rule->role ? rule->role : "",
            rule->subrole ? rule->subrole : "",
            display_arrangement(rule->did),
            space_manager_mission_control_index(rule->sid),
            json_bool(rule->follow_space),
            rule->alpha,
            json_optional_bool(rule->manage),
            json_optional_bool(rule->sticky),
            json_optional_bool(rule->mff),
            layer_str[rule->layer],
            json_optional_bool(rule->border),
            json_optional_bool(rule->fullscreen),
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

void rule_merge_actions(struct rule *from, struct rule *to)
{
    /*
     * Do not merge rule filter fields:
     * app, title, role, subrole
     */
    to->follow_space = from->follow_space;
    if (from->did)
        to->did = from->did;
    if (from->sid)
        to->sid = from->sid;
    if (in_range_ei(from->alpha, 0.0f, 1.0f))
        to->alpha = from->alpha;
    if (from->manage != RULE_PROP_UD)
        to->manage = from->manage;
    if (from->sticky != RULE_PROP_UD)
        to->sticky = from->sticky;
    if (from->mff != RULE_PROP_UD)
        to->mff = from->mff;
    if (from->layer)
        to->layer = from->layer;
    if (from->border != RULE_PROP_UD)
        to->border = from->border;
    if (from->fullscreen != RULE_PROP_UD)
        to->fullscreen = from->fullscreen;
    if (from->grid[0] != 0 && from->grid[1] != 0) {
        to->grid[0] = from->grid[0];
        to->grid[1] = from->grid[1];
        to->grid[2] = from->grid[2];
        to->grid[3] = from->grid[3];
        to->grid[4] = from->grid[4];
        to->grid[5] = from->grid[5];
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

bool rule_remove_by_label(char *label)
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

void rule_add_without_apply(struct rule *rule)
{
    if (rule->label) rule_remove_by_label(rule->label);
    buf_push(g_window_manager.rules, *rule);

}

void rule_add(struct rule *rule)
{
    rule_add_without_apply(rule);
    rule_apply(rule);
}

void rule_add_multiple(struct rule *rules)
{
    for (int i = 0; i < buf_len(rules); ++i) {
        rule_add_without_apply(&rules[i]);
    }
    for (int window_index = 0; window_index < g_window_manager.window.capacity; ++window_index) {
        struct bucket *bucket = g_window_manager.window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                window_manager_apply_rules_to_window(&g_space_manager, &g_window_manager, window);
            }

            bucket = bucket->next;
        }
    }
}

void rule_destroy(struct rule *rule)
{
    if (rule->app_regex_valid)     regfree(&rule->app_regex);
    if (rule->title_regex_valid)   regfree(&rule->title_regex);
    if (rule->role_regex_valid)    regfree(&rule->role_regex);
    if (rule->subrole_regex_valid) regfree(&rule->subrole_regex);

    if (rule->label)   free(rule->label);
    if (rule->app)     free(rule->app);
    if (rule->title)   free(rule->title);
    if (rule->role)    free(rule->role);
    if (rule->subrole) free(rule->subrole);
}

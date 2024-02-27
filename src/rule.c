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
            "\t\"sub-layer\":\"%s\",\n"
            "\t\"native-fullscreen\":%s,\n"
            "\t\"grid\":\"%d:%d:%d:%d:%d:%d\",\n"
            "\t\"one-shot\":%s,\n"
            "\t\"flags\":\"0x%08x\"\n"
            "}",
            index,
            rule->label ? rule->label : "",
            rule->app ? rule->app : "",
            rule->title ? rule->title : "",
            rule->role ? rule->role : "",
            rule->subrole ? rule->subrole : "",
            display_arrangement(rule->did),
            space_manager_mission_control_index(rule->sid),
            json_bool(rule_check_flag(rule, RULE_FOLLOW_SPACE)),
            rule->opacity,
            json_optional_bool(rule->manage),
            json_optional_bool(rule->sticky),
            json_optional_bool(rule->mff),
            rule_check_flag(rule, RULE_LAYER) ? layer_str[rule->layer] : "",
            json_optional_bool(rule->fullscreen),
            rule->grid[0], rule->grid[1],
            rule->grid[2], rule->grid[3],
            rule->grid[4], rule->grid[5],
            json_bool(rule_check_flag(rule, RULE_ONE_SHOT)),
            rule->flags);
}

void rule_reapply_all(void)
{
    for (int i = 0; i < buf_len(g_window_manager.rules); ++i) {
        if (!rule_check_flag(&g_window_manager.rules[i], RULE_ONE_SHOT)) {
            rule_apply(&g_window_manager.rules[i]);
        }
    }
}

bool rule_reapply_by_index(int index)
{
    for (int i = 0; i < buf_len(g_window_manager.rules); ++i) {
        if (i == index) {
            if (!rule_check_flag(&g_window_manager.rules[i], RULE_ONE_SHOT)) {
                rule_apply(&g_window_manager.rules[i]);
            }
            return true;
        }
    }

    return false;
}

bool rule_reapply_by_label(char *label)
{
    for (int i = 0; i < buf_len(g_window_manager.rules); ++i) {
        if (string_equals(g_window_manager.rules[i].label, label)) {
            if (!rule_check_flag(&g_window_manager.rules[i], RULE_ONE_SHOT)) {
                rule_apply(&g_window_manager.rules[i]);
            }
            return true;
        }
    }

    return false;
}

void rule_apply(struct rule *rule)
{
    for (int window_index = 0; window_index < g_window_manager.window.capacity; ++window_index) {
        struct bucket *bucket = g_window_manager.window.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct window *window = bucket->value;
                if (window->is_root) {
                    char *window_title = window_title_ts(window);
                    char *window_role = window_role_ts(window);
                    char *window_subrole = window_subrole_ts(window);
                    window_manager_apply_manage_rule_to_window(&g_space_manager, &g_window_manager, window, rule, window_title, window_role, window_subrole);

                    if (window_manager_is_window_eligible(window)) {
                        window->is_eligible = true;
                        window_manager_apply_rule_to_window(&g_space_manager, &g_window_manager, window, rule, window_title, window_role, window_subrole);
                    }
                }
            }

            bucket = bucket->next;
        }
    }
}

void rule_add(struct rule *rule)
{
    if (rule->label) rule_remove_by_label(rule->label);
    buf_push(g_window_manager.rules, *rule);
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

void rule_destroy(struct rule *rule)
{
    if (rule_check_flag(rule, RULE_APP_VALID))     regfree(&rule->app_regex);
    if (rule_check_flag(rule, RULE_TITLE_VALID))   regfree(&rule->title_regex);
    if (rule_check_flag(rule, RULE_ROLE_VALID))    regfree(&rule->role_regex);
    if (rule_check_flag(rule, RULE_SUBROLE_VALID)) regfree(&rule->subrole_regex);

    if (rule->label)   free(rule->label);
    if (rule->app)     free(rule->app);
    if (rule->title)   free(rule->title);
    if (rule->role)    free(rule->role);
    if (rule->subrole) free(rule->subrole);
}

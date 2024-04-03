extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;

void rule_serialize(FILE *rsp, struct rule *rule, int index)
{
    TIME_FUNCTION;

    char *app   = rule->app;
    char *title = rule->title;
    char *role  = rule->role;
    char *srole = rule->subrole;

    char *escaped_app   = app   ? ts_string_escape(app)   : NULL;
    char *escaped_title = title ? ts_string_escape(title) : NULL;
    char *escaped_role  = role  ? ts_string_escape(role)  : NULL;
    char *escaped_srole = srole ? ts_string_escape(srole) : NULL;

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
            "\t\"scratchpad\":\"%s\",\n"
            "\t\"one-shot\":%s,\n"
            "\t\"flags\":\"0x%08x\"\n"
            "}",
            index,
            rule->label ? rule->label : "",
            escaped_app ? escaped_app : app ? app : "",
            escaped_title ? escaped_title : title ? title : "",
            escaped_role ? escaped_role : role ? role : "",
            escaped_srole ? escaped_srole : srole ? srole : "",
            rule->effects.did ? display_manager_display_id_arrangement(rule->effects.did) : 0,
            rule->effects.sid ? space_manager_mission_control_index(rule->effects.sid) : 0,
            json_bool(rule_effects_check_flag(&rule->effects, RULE_FOLLOW_SPACE)),
            rule->effects.opacity,
            json_optional_bool(rule->effects.manage),
            json_optional_bool(rule->effects.sticky),
            json_optional_bool(rule->effects.mff),
            rule_effects_check_flag(&rule->effects, RULE_LAYER) ? layer_str[rule->effects.layer] : "",
            json_optional_bool(rule->effects.fullscreen),
            rule->effects.grid[0], rule->effects.grid[1],
            rule->effects.grid[2], rule->effects.grid[3],
            rule->effects.grid[4], rule->effects.grid[5],
            rule->effects.scratchpad ? rule->effects.scratchpad : "",
            json_bool(rule_check_flag(rule, RULE_ONE_SHOT)),
            (uint32_t)(rule->effects.flags << 16) | (uint32_t)rule->flags);
}

void rule_combine_effects(struct rule_effects *effects, struct rule_effects *result)
{
    if (effects->did) {
        result->did = effects->did;
        if (rule_effects_check_flag(effects, RULE_FOLLOW_SPACE)) {
            rule_effects_set_flag(result, RULE_FOLLOW_SPACE);
        } else {
            rule_effects_clear_flag(result, RULE_FOLLOW_SPACE);
        }
    }

    if (effects->sid) {
        result->sid = effects->sid;
        if (rule_effects_check_flag(effects, RULE_FOLLOW_SPACE)) {
            rule_effects_set_flag(result, RULE_FOLLOW_SPACE);
        } else {
            rule_effects_clear_flag(result, RULE_FOLLOW_SPACE);
        }
    }

    if (rule_effects_check_flag(effects, RULE_OPACITY) && in_range_ii(effects->opacity, 0.0f, 1.0f)) {
        result->opacity = effects->opacity;
        rule_effects_set_flag(result, RULE_OPACITY);
    }

    if (rule_effects_check_flag(effects, RULE_LAYER)) {
        result->layer = effects->layer;
        rule_effects_set_flag(result, RULE_LAYER);
    }

    if (effects->scratchpad) {
        if (result->scratchpad) free(result->scratchpad);
        result->scratchpad = string_copy(effects->scratchpad);
    }

    if (effects->manage     != RULE_PROP_UD) result->manage     = effects->manage;
    if (effects->sticky     != RULE_PROP_UD) result->sticky     = effects->sticky;
    if (effects->mff        != RULE_PROP_UD) result->mff        = effects->mff;
    if (effects->fullscreen != RULE_PROP_UD) result->fullscreen = effects->fullscreen;

    if (effects->grid[0] != 0 && effects->grid[1] != 0) {
        result->grid[0] = effects->grid[0];
        result->grid[1] = effects->grid[1];
        result->grid[2] = effects->grid[2];
        result->grid[3] = effects->grid[3];
        result->grid[4] = effects->grid[4];
        result->grid[5] = effects->grid[5];
    }
}

void rule_reapply_all(void)
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

                    window_manager_apply_manage_rules_to_window(&g_space_manager, &g_window_manager, window, window_title, window_role, window_subrole, false);

                    if (window_manager_is_window_eligible(window)) {
                        window->is_eligible = true;
                        window_manager_apply_rules_to_window(&g_space_manager, &g_window_manager, window, window_title, window_role, window_subrole, false);
                    }
                }
            }

            bucket = bucket->next;
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

                    if (window_manager_rule_matches_window(rule, window, window_title, window_role, window_subrole)) {
                        window_manager_apply_manage_rule_effects_to_window(&g_space_manager, &g_window_manager, window, &rule->effects, window_title, window_role, window_subrole);

                        if (window_manager_is_window_eligible(window)) {
                            window->is_eligible = true;
                            window_manager_apply_rule_effects_to_window(&g_space_manager, &g_window_manager, window, &rule->effects, window_title, window_role, window_subrole);
                        }
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

    if (rule->effects.scratchpad) free(rule->effects.scratchpad);
}

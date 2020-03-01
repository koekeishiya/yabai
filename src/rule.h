#ifndef RULE_H
#define RULE_H

#define RULE_PROP_UD  0
#define RULE_PROP_ON  1
#define RULE_PROP_OFF 2

struct rule
{
    char *label;
    regex_t app_regex;
    regex_t title_regex;
    bool app_regex_valid;
    bool title_regex_valid;
    bool app_regex_exclude;
    bool title_regex_exclude;
    bool follow_space;
    uint32_t did;
    uint64_t sid;
    float alpha;
    int manage;
    int sticky;
    int layer;
    int border;
    int fullscreen;
    unsigned grid[6];
};

bool rule_remove(char *label);
void rule_add(struct rule *rule);
void rule_destroy(struct rule *rule);

#endif

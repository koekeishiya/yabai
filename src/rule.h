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
    bool follow_space;
    int display;
    int space;
    float alpha;
    int manage;
    int sticky;
    int topmost;
    int border;
    int fullscreen;
    unsigned grid[6];
};

bool rule_remove(char *label);
void rule_add(struct rule *rule);
bool rule_is_valid(struct rule *rule);
struct rule *rule_create(void);
void rule_destroy(struct rule *rule);

#endif

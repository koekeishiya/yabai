#ifndef RULE_H
#define RULE_H

#define RULE_PROP_UD  0
#define RULE_PROP_ON  1
#define RULE_PROP_OFF 2

enum rule_flag
{
    RULE_APP_VALID       = 0x001,
    RULE_TITLE_VALID     = 0x002,
    RULE_ROLE_VALID      = 0x004,
    RULE_SUBROLE_VALID   = 0x008,
    RULE_APP_EXCLUDE     = 0x010,
    RULE_TITLE_EXCLUDE   = 0x020,
    RULE_ROLE_EXCLUDE    = 0x040,
    RULE_SUBROLE_EXCLUDE = 0x080,
    RULE_ONE_SHOT        = 0x100,
    RULE_ONE_SHOT_REMOVE = 0x200
};

enum rule_effects_flag
{
    RULE_FOLLOW_SPACE = 0x01,
    RULE_OPACITY      = 0x02,
    RULE_LAYER        = 0x04
};

struct rule_effects
{
    uint32_t did;
    uint64_t sid;
    float opacity;
    int manage;
    int sticky;
    int mff;
    int layer;
    int fullscreen;
    unsigned grid[6];
    char *scratchpad;
    uint16_t flags;
};

struct rule
{
    char *label;
    char *app;
    char *title;
    char *role;
    char *subrole;
    regex_t app_regex;
    regex_t title_regex;
    regex_t role_regex;
    regex_t subrole_regex;
    struct rule_effects effects;
    uint16_t flags;
};

static inline bool rule_check_flag(struct rule *r, enum rule_flag x) { return r->flags & x; }
static inline void rule_clear_flag(struct rule *r, enum rule_flag x) { r->flags &= ~x; }
static inline void rule_set_flag(struct rule *r, enum rule_flag x) { r->flags |= x; }

static inline bool rule_effects_check_flag(struct rule_effects *e, enum rule_effects_flag x) { return e->flags & x; }
static inline void rule_effects_clear_flag(struct rule_effects *e, enum rule_effects_flag x) { e->flags &= ~x; }
static inline void rule_effects_set_flag(struct rule_effects *e, enum rule_effects_flag x) { e->flags |= x; }

void rule_serialize(FILE *rsp, struct rule *rule, int index);
void rule_combine_effects(struct rule_effects *rule_effects, struct rule_effects *result);
void rule_reapply_all(void);
bool rule_reapply_by_index(int index);
bool rule_reapply_by_label(char *label);
void rule_apply(struct rule *rule);
void rule_add(struct rule *rule);
bool rule_remove_by_index(int index);
bool rule_remove_by_label(char *label);
void rule_destroy(struct rule *rule);

#endif

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#define DOCK_ORIENTATION_BOTTOM 2
#define DOCK_ORIENTATION_LEFT   3
#define DOCK_ORIENTATION_RIGHT  4

enum display_arrangement_order
{
    DISPLAY_ARRANGEMENT_ORDER_DEFAULT,
    DISPLAY_ARRANGEMENT_ORDER_X,
    DISPLAY_ARRANGEMENT_ORDER_Y
};

static const char *display_arrangement_order_str[] =
{
    "default",
    "horizontal",
    "vertical"
};

enum external_bar_mode
{
    EXTERNAL_BAR_OFF,
    EXTERNAL_BAR_MAIN,
    EXTERNAL_BAR_ALL
};

static const char *external_bar_mode_str[] =
{
    "off",
    "main",
    "all"
};

struct display_label
{
    uint32_t did;
    char *label;
};

struct display_manager
{
    uint32_t current_display_id;
    uint32_t last_display_id;

    int top_padding;
    int bottom_padding;

    enum display_arrangement_order order;
    enum external_bar_mode mode;

    struct display_label *labels;
};

struct display_label *display_manager_get_label_for_display(struct display_manager *dm, uint32_t did);
struct display_label *display_manager_get_display_for_label(struct display_manager *dm, char *label);
bool display_manager_remove_label_for_display(struct display_manager *dm, uint32_t did);
void display_manager_set_label_for_display(struct display_manager *dm, uint32_t did, char *label);
bool display_manager_query_displays(FILE *rsp, uint64_t flags);
CFStringRef display_manager_main_display_uuid(void);
uint32_t display_manager_main_display_id(void);
CFStringRef display_manager_active_display_uuid(void);
uint32_t display_manager_active_display_id(void);
CFStringRef display_manager_dock_display_uuid(void);
uint32_t display_manager_dock_display_id(void);
CFStringRef display_manager_point_display_uuid(CGPoint point);
uint32_t display_manager_point_display_id(CGPoint point);
uint32_t display_manager_cursor_display_id(void);
int display_manager_display_id_arrangement(uint32_t did);
CFStringRef display_manager_arrangement_display_uuid(int arrangement);
uint32_t display_manager_arrangement_display_id(int arrangement);
uint32_t display_manager_prev_display_id(uint32_t did);
uint32_t display_manager_next_display_id(uint32_t did);
uint32_t display_manager_first_display_id(void);
uint32_t display_manager_last_display_id(void);
uint32_t display_manager_find_closest_display_in_direction(uint32_t acting_did, int direction);
bool display_manager_menu_bar_hidden(void);
CGRect display_manager_menu_bar_rect(uint32_t did);
bool display_manager_dock_hidden(void);
int display_manager_dock_orientation(void);
CGRect display_manager_dock_rect(void);
bool display_manager_active_display_is_animating(void);
bool display_manager_display_is_animating(uint32_t did);
int display_manager_active_display_count(void);
uint32_t *display_manager_active_display_list(int *count);
uint32_t display_manager_focus_display_with_point(uint32_t did, CGPoint point, bool update_cursor_position);
void display_manager_focus_display(uint32_t did, uint64_t sid);
enum space_op_error display_manager_focus_space(uint32_t did, uint64_t sid);
bool display_manager_begin(struct display_manager *dm);

#endif

#ifndef SPACE_MANAGER
#define SPACE_MANAGER

struct space_label
{
    uint64_t sid;
    char *label;
};

struct space_manager
{
    struct table view;
    uint64_t current_space_id;
    uint64_t last_space_id;
    bool did_begin;
    enum view_type layout;
    int top_padding;
    int bottom_padding;
    int left_padding;
    int right_padding;
    int window_gap;
    float split_ratio;
    enum window_node_split split_type;
    enum window_node_child window_placement;
    enum window_insertion_point window_insertion_point;
    bool window_zoom_persist;
    uint32_t auto_balance;
    struct space_label *labels;
    CFArrayRef grid_columns;
};

enum space_op_error
{
    SPACE_OP_ERROR_SUCCESS              = 0,
    SPACE_OP_ERROR_MISSING_SRC          = 1,
    SPACE_OP_ERROR_MISSING_DST          = 2,
    SPACE_OP_ERROR_INVALID_SRC          = 3,
    SPACE_OP_ERROR_INVALID_DST          = 4,
    SPACE_OP_ERROR_INVALID_TYPE         = 5,
    SPACE_OP_ERROR_SAME_SPACE           = 6,
    SPACE_OP_ERROR_SAME_DISPLAY         = 7,
    SPACE_OP_ERROR_DISPLAY_IS_ANIMATING = 8,
    SPACE_OP_ERROR_IN_MISSION_CONTROL   = 9,
    SPACE_OP_ERROR_SCRIPTING_ADDITION   = 10,
};

bool space_manager_query_space(FILE *rsp, uint64_t sid, uint64_t flags);
bool space_manager_query_spaces_for_window(FILE *rsp, struct window *window, uint64_t flags);
bool space_manager_query_spaces_for_display(FILE *rsp, uint32_t did, uint64_t flags);
bool space_manager_query_spaces_for_displays(FILE *rsp, uint64_t flags);
struct view *space_manager_query_view(struct space_manager *sm, uint64_t sid);
struct view *space_manager_find_view(struct space_manager *sm, uint64_t sid);
void space_manager_refresh_view(struct space_manager *sm, uint64_t sid);
void space_manager_mark_view_invalid(struct space_manager *sm,  uint64_t sid);
void space_manager_mark_view_dirty(struct space_manager *sm,  uint64_t sid);
void space_manager_untile_window(struct view *view, struct window *window);
struct view *space_manager_tile_window_on_space_with_insertion_point(struct space_manager *sm, struct window *window, uint64_t sid, uint32_t insertion_point);
struct view *space_manager_tile_window_on_space(struct space_manager *sm, struct window *window, uint64_t sid);
bool space_manager_equalize_space(struct space_manager *sm, uint64_t sid, uint32_t axis_flag);
bool space_manager_balance_space(struct space_manager *sm, uint64_t sid, uint32_t axis_flag);
void space_manager_toggle_window_split(struct space_manager *sm, struct window *window);
int space_manager_mission_control_index(uint64_t sid);
uint64_t space_manager_mission_control_space(int desktop_id);
uint64_t space_manager_cursor_space(void);
uint64_t space_manager_prev_space(uint64_t sid);
uint64_t space_manager_next_space(uint64_t sid);
uint64_t space_manager_grid_space(uint64_t sid, char *direction);
uint64_t space_manager_first_space(void);
uint64_t space_manager_last_space(void);
uint64_t space_manager_active_space(void);
struct space_label *space_manager_get_label_for_space(struct space_manager *sm, uint64_t sid);
struct space_label *space_manager_get_space_for_label(struct space_manager *sm, char *label);
bool space_manager_remove_label_for_space(struct space_manager *sm, uint64_t sid);
void space_manager_set_label_for_space(struct space_manager *sm, uint64_t sid, char *label);
void space_manager_set_layout_for_space(struct space_manager *sm, uint64_t sid, enum view_type type);
bool space_manager_set_gap_for_space(struct space_manager *sm, uint64_t sid, int type, int gap);
bool space_manager_toggle_gap_for_space(struct space_manager *sm, uint64_t sid);
void space_manager_toggle_mission_control(uint64_t sid);
void space_manager_toggle_show_desktop(uint64_t sid);
void space_manager_set_layout_for_all_spaces(struct space_manager *sm, enum view_type layout);
void space_manager_set_window_gap_for_all_spaces(struct space_manager *sm, int window_gap);
void space_manager_set_top_padding_for_all_spaces(struct space_manager *sm, int top_padding);
void space_manager_set_bottom_padding_for_all_spaces(struct space_manager *sm, int bottom_padding);
void space_manager_set_left_padding_for_all_spaces(struct space_manager *sm, int left_padding);
void space_manager_set_right_padding_for_all_spaces(struct space_manager *sm, int right_padding);
void space_manager_set_split_type_for_all_spaces(struct space_manager *sm, enum window_node_split split_type);
void space_manager_set_auto_balance_for_all_spaces(struct space_manager *sm, uint32_t auto_balance);
bool space_manager_set_padding_for_space(struct space_manager *sm, uint64_t sid, int type, int top, int bottom, int left, int right);
bool space_manager_toggle_padding_for_space(struct space_manager *sm, uint64_t sid);
bool space_manager_rotate_space(struct space_manager *sm, uint64_t sid, int degrees);
bool space_manager_mirror_space(struct space_manager *sm, uint64_t sid, enum window_node_split axis);
void space_manager_move_window_list_to_space(uint64_t sid, uint32_t *window_list, int window_count);
void space_manager_move_window_to_space(uint64_t sid, struct window *window);
enum space_op_error space_manager_focus_space(uint64_t sid);
enum space_op_error space_manager_switch_space(uint64_t sid);
enum space_op_error space_manager_swap_space_with_space(uint64_t acting_sid, uint64_t selector_sid);
enum space_op_error space_manager_move_space_to_space(uint64_t acting_sid, uint64_t selector_sid);
enum space_op_error space_manager_move_space_to_display(struct space_manager *sm, uint64_t sid, uint32_t did);
enum space_op_error space_manager_destroy_space(uint64_t sid);
enum space_op_error space_manager_add_space(uint64_t sid);
void space_manager_assign_process_to_space(pid_t pid, uint64_t sid);
void space_manager_assign_process_to_all_spaces(pid_t pid);
bool space_manager_is_window_on_active_space(struct window *window);
bool space_manager_is_window_on_space(uint64_t sid, struct window *window);
void space_manager_mark_spaces_invalid_for_display(struct space_manager *sm, uint32_t did);
void space_manager_mark_spaces_invalid(struct space_manager *sm);
bool space_manager_refresh_application_windows(struct space_manager *sm);
void space_manager_handle_display_add(struct space_manager *sm, uint32_t did);
void space_manager_begin(struct space_manager *sm);

#endif

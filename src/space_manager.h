#ifndef SPACE_MANAGER
#define SPACE_MANAGER

extern int SLSGetSpaceManagementMode(int cid);
extern CFArrayRef SLSCopyManagedDisplaySpaces(int cid);
extern CGError SLSGetMenuBarAutohideEnabled(int cid, int *enabled);
extern CGError SLSGetRevealedMenuBarBounds(CGRect *rect, int cid, uint64_t sid);
extern CGError SLSGetDockRectWithReason(int cid, CGRect *rect, int *reason);
extern CGError SLSProcessAssignToSpace(int cid, pid_t pid, uint64_t sid);
extern CGError SLSProcessAssignToAllSpaces(int cid, pid_t pid);
extern void CGSMoveWindowsToManagedSpace(int cid, CFArrayRef window_list, uint64_t sid);
extern void CGSRemoveWindowsFromSpaces(int cid, CFArrayRef window_list, CFArrayRef space_list);
extern void CGSAddWindowsToSpaces(int cid, CFArrayRef window_list, CFArrayRef space_list);
extern Boolean CoreDockGetAutoHideEnabled(void);
extern void CoreDockGetOrientationAndPinning(int *orientation, int *pinning);

#define DOCK_ORIENTATION_BOTTOM 2
#define DOCK_ORIENTATION_LEFT   3
#define DOCK_ORIENTATION_RIGHT  4

struct space_manager
{
    struct table view;
    uint64_t current_space_id;
    uint64_t last_space_id;
    uint32_t top_padding;
    uint32_t bottom_padding;
    uint32_t left_padding;
    uint32_t right_padding;
    uint32_t window_gap;
    float split_ratio;
    bool auto_balance;
};

bool space_manager_has_separate_spaces(void);
bool space_manager_menu_bar_hidden(void);
CGRect space_manager_menu_bar_rect(void);
bool space_manager_dock_hidden(void);
int space_manager_dock_orientation(void);
CGRect space_manager_dock_rect(void);
struct view *space_manager_find_view(struct space_manager *sm, uint64_t sid);
void space_manager_refresh_view(struct space_manager *sm, uint64_t sid);
void space_manager_mark_view_invalid(struct space_manager *sm,  uint64_t sid);
void space_manager_mark_view_dirty(struct space_manager *sm,  uint64_t sid);
void space_manager_untile_window(struct space_manager *sm, struct view *view, struct ax_window *window);
struct view *space_manager_tile_window_on_space(struct space_manager *sm, struct ax_window *window, uint64_t sid);
void space_manager_balance_space(struct space_manager *sm, uint64_t sid);
void space_manager_toggle_window_split(struct space_manager *sm, struct ax_window *window);
uint64_t space_manager_mission_control_space(int desktop_id);
uint64_t space_manager_prev_space(uint64_t sid);
uint64_t space_manager_next_space(uint64_t sid);
uint64_t space_manager_active_space(void);
void space_manager_move_window_to_space(uint64_t sid, struct ax_window *window);
void space_manager_remove_window_from_space(uint64_t sid, struct ax_window *window);
void space_manager_add_window_to_space(uint64_t sid, struct ax_window *window);
void space_manager_focus_space(uint64_t sid);
void space_manager_move_space_to_display(struct space_manager *sm, uint32_t did);
void space_manager_destroy_space(void);
void space_manager_add_space(void);
void space_manager_assign_process_to_space(pid_t pid, uint64_t sid);
void space_manager_assign_process_to_all_spaces(pid_t pid);
bool space_manager_is_window_on_active_space(struct ax_window *window);
bool space_manager_is_window_on_space(uint64_t sid, struct ax_window *window);
bool space_manager_refresh_application_windows(void);
void space_manager_begin(struct space_manager *sm);

#endif

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

extern CFStringRef SLSCopyActiveMenuBarDisplayIdentifier(int cid);
extern CFStringRef SLSCopyBestManagedDisplayForPoint(int cid, CGPoint point);
extern bool SLSManagedDisplayIsAnimating(int cid, CFStringRef uuid);
extern CGError SLSGetMenuBarAutohideEnabled(int cid, int *enabled);
extern CGError SLSGetRevealedMenuBarBounds(CGRect *rect, int cid, uint64_t sid);
extern CGError SLSGetDockRectWithReason(int cid, CGRect *rect, int *reason);
extern Boolean CoreDockGetAutoHideEnabled(void);
extern void CoreDockGetOrientationAndPinning(int *orientation, int *pinning);

#define DOCK_ORIENTATION_BOTTOM 2
#define DOCK_ORIENTATION_LEFT   3
#define DOCK_ORIENTATION_RIGHT  4

struct display_manager
{
    uint32_t current_display_id;
    uint32_t last_display_id;
};

bool display_manager_query_displays(FILE *rsp);
CFStringRef display_manager_main_display_uuid(void);
uint32_t display_manager_main_display_id(void);
CFStringRef display_manager_active_display_uuid(void);
uint32_t display_manager_active_display_id(void);
CFStringRef display_manager_dock_display_uuid(void);
uint32_t display_manager_dock_display_id(void);
CFStringRef display_manager_cursor_display_uuid(void);
uint32_t display_manager_cursor_display_id(void);
CFStringRef display_manager_arrangement_display_uuid(int arrangement);
uint32_t display_manager_arrangement_display_id(int arrangement);
uint32_t display_manager_prev_display_id(uint32_t did);
uint32_t display_manager_next_display_id(uint32_t did);
uint32_t display_manager_first_display_id(void);
uint32_t display_manager_last_display_id(void);
bool display_manager_menu_bar_hidden(void);
CGRect display_manager_menu_bar_rect(uint32_t did);
bool display_manager_dock_hidden(void);
int display_manager_dock_orientation(void);
CGRect display_manager_dock_rect(void);
bool display_manager_active_display_is_animating(void);
bool display_manager_display_is_animating(uint32_t did);
uint32_t display_manager_active_display_count(void);
uint32_t *display_manager_active_display_list(uint32_t *count);
void display_manager_focus_display(uint32_t display_id);
bool display_manager_begin(struct display_manager *dm);
bool display_manager_end(void);

#endif

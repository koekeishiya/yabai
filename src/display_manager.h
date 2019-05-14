#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

extern CFStringRef SLSCopyActiveMenuBarDisplayIdentifier(int cid);
extern bool SLSManagedDisplayIsAnimating(int cid, CFStringRef uuid);

struct display_manager
{
    uint32_t current_display_id;
    uint32_t last_display_id;
};

void display_manager_query_display(FILE *rsp, uint32_t did);
bool display_manager_query_displays(FILE *rsp);
CFStringRef display_manager_main_display_uuid(void);
uint32_t display_manager_main_display_id(void);
CFStringRef display_manager_active_display_uuid(void);
uint32_t display_manager_active_display_id(void);
CFStringRef display_manager_arrangement_display_uuid(int arrangement);
uint32_t display_manager_arrangement_display_id(int arrangement);
uint32_t display_manager_prev_display_id(uint32_t did);
uint32_t display_manager_next_display_id(uint32_t did);
uint32_t display_manager_left_display_id(void);
uint32_t display_manager_right_display_id(void);
uint32_t display_manager_bottom_display_id(void);
bool display_manager_active_display_is_animating(void);
bool display_manager_display_is_animating(uint32_t did);
uint32_t display_manager_active_display_count(void);
uint32_t *display_manager_active_display_list(uint32_t *count);
void display_manager_focus_display(uint32_t display_id);
bool display_manager_begin(struct display_manager *dm);
bool display_manager_end(void);

#endif

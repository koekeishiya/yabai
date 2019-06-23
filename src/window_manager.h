#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

extern CFTypeRef SLSWindowQueryWindows(int cid, CFArrayRef windows, int count);
extern CFTypeRef SLSWindowQueryResultCopyWindows(CFTypeRef window_query);
extern CGError SLSWindowIteratorAdvance(CFTypeRef iterator);
extern uint32_t SLSWindowIteratorGetParentID(CFTypeRef iterator);
extern uint32_t SLSWindowIteratorGetWindowID(CFTypeRef iterator);
extern OSStatus _SLPSGetFrontProcess(ProcessSerialNumber *psn);
extern CGError SLSGetWindowOwner(int cid, uint32_t wid, int *wcid);
extern CGError SLSGetConnectionPSN(int cid, ProcessSerialNumber *psn);
extern CGError SLSConnectionGetPID(int cid, pid_t *pid);
extern CGError _SLPSSetFrontProcessWithOptions(ProcessSerialNumber *psn, uint32_t wid, uint32_t mode);
extern CGError SLPSPostEventRecordTo(ProcessSerialNumber *psn, uint8_t *bytes);
extern OSStatus SLSFindWindowByGeometry(int cid, int zero, int one, int zero_again, CGPoint *screen_point, CGPoint *window_point, uint32_t *wid, int *wcid);
extern CGError SLSGetCurrentCursorLocation(int cid, CGPoint *point);

#define kCPSAllWindows    0x100
#define kCPSUserGenerated 0x200
#define kCPSNoWindows     0x400

enum purify_mode
{
    PURIFY_DISABLED,
    PURIFY_MANAGED,
    PURIFY_ALWAYS
};

static const char *purify_mode_str[] =
{
    "on",
    "float",
    "off"
};

enum ffm_mode
{
    FFM_DISABLED,
    FFM_AUTOFOCUS,
    FFM_AUTORAISE
};

static const char *ffm_mode_str[] =
{
    "disabled",
    "autofocus",
    "autoraise"
};

struct window_manager
{
    AXUIElementRef system_element;
    struct table application;
    struct table window;
    struct table managed_window;
    struct table window_lost_focused_event;
    struct table application_lost_front_switched_event;
    struct rule **rules;
    uint32_t focused_window_id;
    ProcessSerialNumber focused_window_psn;
    uint32_t last_window_id;
    bool enable_mff;
    enum ffm_mode ffm_mode;
    enum purify_mode purify_mode;
    bool enable_window_opacity;
    bool enable_window_topmost;
    bool enable_window_border;
    int window_border_width;
    uint32_t active_window_border_color;
    uint32_t normal_window_border_color;
    uint32_t insert_window_border_color;
    float active_window_opacity;
    float normal_window_opacity;
};

void window_manager_query_windows_for_space(FILE *rsp, uint64_t sid);
void window_manager_query_windows_for_display(FILE *rsp, uint32_t did);
void window_manager_query_windows_for_displays(FILE *rsp);
void window_manager_apply_rule_to_window(struct space_manager *sm, struct window_manager *wm, struct ax_window *window, struct rule *rule);
void window_manager_apply_rules_to_window(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
void window_manager_center_mouse(struct window_manager *wm, struct ax_window *window);
bool window_manager_should_manage_window(struct ax_window *window);
void window_manager_tile_window(struct window_manager *wm, struct ax_window *window);
void window_manager_move_window(struct ax_window *window, float x, float y);
void window_manager_resize_window(struct ax_window *window, float width, float height);
struct ax_window *window_manager_find_window_on_space_by_rank(struct window_manager *wm, uint64_t sid, int rank);
struct ax_window *window_manager_find_window_on_display_by_rank(struct window_manager *wm, uint32_t did, int rank);
struct ax_window *window_manager_find_window_at_point_filtering_window(struct window_manager *wm, CGPoint point, uint32_t filter_wid);
struct ax_window *window_manager_find_window_at_point(struct window_manager *wm, CGPoint point);
struct ax_window *window_manager_find_closest_managed_window_in_direction(struct window_manager *wm, struct ax_window *window, int direction);
struct ax_window *window_manager_find_closest_window_in_direction(struct window_manager *wm, struct ax_window *window, int direction);
struct ax_window *window_manager_find_prev_managed_window(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
struct ax_window *window_manager_find_next_managed_window(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
struct ax_window *window_manager_find_last_managed_window(struct space_manager *sm, struct window_manager *wm);
void window_manager_focus_window_without_raise(uint32_t window_id);
void window_manager_focus_window_with_raise(uint32_t window_id);
struct ax_window *window_manager_focused_window(struct window_manager *wm);
struct ax_application *window_manager_focused_application(struct window_manager *wm);
struct view *window_manager_find_managed_window(struct window_manager *wm, struct ax_window *window);
void window_manager_remove_managed_window(struct window_manager *wm, struct ax_window *window);
void window_manager_add_managed_window(struct window_manager *wm, struct ax_window *window, struct view *view);
bool window_manager_find_lost_front_switched_event(struct window_manager *wm, pid_t pid);
void window_manager_remove_lost_front_switched_event(struct window_manager *wm, pid_t pid);
void window_manager_add_lost_front_switched_event(struct window_manager *wm, pid_t pid);
bool window_manager_find_lost_focused_event(struct window_manager *wm, uint32_t window_id);
void window_manager_remove_lost_focused_event(struct window_manager *wm, uint32_t window_id);
void window_manager_add_lost_focused_event(struct window_manager *wm, uint32_t window_id);
struct ax_window *window_manager_find_window(struct window_manager *wm, uint32_t window_id);
void window_manager_remove_window(struct window_manager *wm, uint32_t window_id);
void window_manager_add_window(struct window_manager *wm, struct ax_window *window);
struct ax_application *window_manager_find_application(struct window_manager *wm, pid_t pid);
void window_manager_remove_application(struct window_manager *wm, pid_t pid);
void window_manager_add_application(struct window_manager *wm, struct ax_application *application);
struct ax_window **window_manager_find_application_windows(struct window_manager *wm, struct ax_application *application, int *count);
void window_manager_move_window_relative(struct window_manager *wm, struct ax_window *window, int type, float dx, float dy);
void window_manager_resize_window_relative(struct window_manager *wm, struct ax_window *window, int direction, float dx, float dy);
void window_manager_set_purify_mode(struct window_manager *wm, enum purify_mode mode);
void window_manager_set_active_window_opacity(struct window_manager *wm, float opacity);
void window_manager_set_normal_window_opacity(struct window_manager *wm, float opacity);
void window_manager_set_active_border_window_color(struct window_manager *wm, uint32_t color);
void window_manager_set_normal_border_window_color(struct window_manager *wm, uint32_t color);
void window_manager_set_window_opacity(struct window_manager *wm, struct ax_window *window, float opacity);
void window_manager_set_window_insertion(struct space_manager *sm, struct window_manager *wm, struct ax_window *window, int direction);
void window_manager_warp_window(struct space_manager *sm, struct ax_window *a, struct ax_window *b);
void window_manager_swap_window(struct space_manager *sm, struct window_manager *wm, struct ax_window *a, struct ax_window *b);
void window_manager_send_window_to_display(struct space_manager *sm, struct window_manager *wm, struct ax_window *window, uint32_t did);
void window_manager_send_window_to_space(struct space_manager *sm, struct window_manager *wm, struct ax_window *window, uint64_t sid);
void window_manager_add_application_windows(struct space_manager *sm, struct window_manager *wm, struct ax_application *application);
void window_manager_apply_grid(struct space_manager *sm, struct window_manager *wm, struct ax_window *window, unsigned r, unsigned c, unsigned x, unsigned y, unsigned w, unsigned h);
void window_manager_purify_window(struct window_manager *wm, struct ax_window *window);
void window_manager_make_children_floating(struct window_manager *wm, struct ax_window *window, bool floating);
void window_manager_make_floating(struct window_manager *wm, uint32_t wid, bool floating);
void window_manager_make_sticky(uint32_t wid, bool sticky);
void window_manager_toggle_window_float(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
void window_manager_toggle_window_sticky(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
void window_manager_toggle_window_parent(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
void window_manager_toggle_window_fullscreen(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
void window_manager_toggle_window_native_fullscreen(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
void window_manager_toggle_window_border(struct window_manager *wm, struct ax_window *window);
void window_manager_validate_windows_on_space(struct space_manager *sm, struct window_manager *wm, uint64_t sid);
void window_manager_check_for_windows_on_space(struct space_manager *sm, struct window_manager *wm, uint64_t sid);
void window_manager_handle_display_add_and_remove(struct space_manager *sm, struct window_manager *wm, uint32_t display_id, uint64_t sid);
void window_manager_begin(struct space_manager *sm, struct window_manager *window_manager);
void window_manager_init(struct window_manager *window_manager);

#endif

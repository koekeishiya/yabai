#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

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

enum ffm_mode
{
    FFM_DISABLED,
    FFM_AUTOFOCUS,
    FFM_AUTORAISE
};

struct window_manager
{
    AXUIElementRef system_element;
    struct table application;
    struct table window;
    struct table managed_window;
    struct table window_lost_focused_event;
    uint32_t focused_window_id;
    pid_t focused_window_pid;
    bool enable_mff;
    enum ffm_mode ffm_mode;
    enum purify_mode purify_mode;
    bool enable_window_border;
    int window_border_width;
    uint32_t active_window_border_color;
    uint32_t normal_window_border_color;
};

void window_manager_center_mouse(struct window_manager *wm, struct ax_window *window);
bool window_manager_should_manage_window(struct ax_window *window);
void window_manager_tile_window(struct window_manager *wm, struct ax_window *window);
void window_manager_move_window(struct ax_window *window, float x, float y);
void window_manager_resize_window(struct ax_window *window, float width, float height);
struct ax_window *window_manager_find_window_at_point(struct window_manager *wm, CGPoint point);
struct ax_window *window_manager_find_closest_window_in_direction(struct window_manager *wm, struct ax_window *window, int direction);
void window_manager_focus_window_without_raise(uint32_t window_id);
void window_manager_focus_window_with_raise(uint32_t window_id);
struct ax_window *window_manager_focused_window(struct window_manager *wm);
struct ax_application *window_manager_focused_application(struct window_manager *wm);
struct view *window_manager_find_managed_window(struct window_manager *wm, struct ax_window *window);
void window_manager_remove_managed_window(struct window_manager *wm, struct ax_window *window);
void window_manager_add_managed_window(struct window_manager *wm, struct ax_window *window, struct view *view);
bool window_manager_find_lost_focused_event(struct window_manager *wm, uint32_t window_id);
void window_manager_remove_lost_focused_event(struct window_manager *wm, uint32_t window_id);
void window_manager_add_lost_focused_event(struct window_manager *wm, uint32_t window_id, enum event_type type);
struct ax_window *window_manager_find_window(struct window_manager *wm, uint32_t window_id);
void window_manager_remove_window(struct window_manager *wm, uint32_t window_id);
void window_manager_add_window(struct window_manager *wm, struct ax_window *window);
struct ax_application *window_manager_find_application(struct window_manager *wm, pid_t pid);
void window_manager_remove_application(struct window_manager *wm, pid_t pid);
void window_manager_add_application(struct window_manager *wm, struct ax_application *application);
struct ax_window **window_manager_find_application_windows(struct window_manager *wm, struct ax_application *application, int *count);
void window_manager_send_window_to_space(struct space_manager *sm, struct window_manager *wm, struct ax_window *window, uint64_t sid);
void window_manager_add_application_windows(struct window_manager *wm, struct ax_application *application);
void window_manager_apply_grid(struct space_manager *sm, struct window_manager *wm, struct ax_window *window, unsigned r, unsigned c, unsigned x, unsigned y, unsigned w, unsigned h);
void window_manager_toggle_window_float(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
void window_manager_toggle_window_sticky(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
void window_manager_toggle_window_fullscreen(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
void window_manager_toggle_window_native_fullscreen(struct space_manager *sm, struct window_manager *wm, struct ax_window *window);
void window_manager_validate_windows_on_space(struct space_manager *sm, struct window_manager *wm, uint64_t sid);
void window_manager_check_for_windows_on_space(struct space_manager *sm, struct window_manager *wm, uint64_t sid);
void window_manager_handle_display_add_and_remove(struct space_manager *sm, struct window_manager *wm, uint32_t display_id, uint64_t sid);
void window_manager_begin(struct window_manager *window_manager);
void window_manager_init(struct window_manager *window_manager);

#endif

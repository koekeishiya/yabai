#ifndef WINDOW_H
#define WINDOW_H

extern int SLSMainConnectionID(void);
extern CGError SLSGetWindowBounds(int cid, uint32_t wid, CGRect *frame);
extern CGError SLSCopyWindowProperty(int cid, uint32_t wid, CFStringRef property, CFTypeRef *value);
extern CFStringRef SLSCopyManagedDisplayForWindow(int cid, uint32_t wid);
extern CFStringRef SLSCopyBestManagedDisplayForRect(int cid, CGRect rect);
extern CFArrayRef SLSCopySpacesForWindows(int cid, int selector, CFArrayRef window_list);

#define AX_WINDOW_MINIMIZED_INDEX      0
#define AX_WINDOW_DEMINIMIZED_INDEX    1
#define AX_WINDOW_SHEET_CREATED_INDEX  2
#define AX_WINDOW_DRAWER_CREATED_INDEX 3
#define AX_WINDOW_DESTROYED_INDEX      4

#define AX_WINDOW_DESTROYED      (1 << AX_WINDOW_DESTROYED_INDEX)
#define AX_WINDOW_MINIMIZED      (1 << AX_WINDOW_MINIMIZED_INDEX)
#define AX_WINDOW_DEMINIMIZED    (1 << AX_WINDOW_DEMINIMIZED_INDEX)
#define AX_WINDOW_SHEET_CREATED  (1 << AX_WINDOW_SHEET_CREATED_INDEX)
#define AX_WINDOW_DRAWER_CREATED (1 << AX_WINDOW_DRAWER_CREATED_INDEX)
#define AX_WINDOW_ALL            (AX_WINDOW_DESTROYED |\
                                  AX_WINDOW_MINIMIZED |\
                                  AX_WINDOW_DEMINIMIZED |\
                                  AX_WINDOW_SHEET_CREATED |\
                                  AX_WINDOW_DRAWER_CREATED)

static CFStringRef ax_window_notification[] =
{
    [AX_WINDOW_DESTROYED_INDEX]      = kAXUIElementDestroyedNotification,
    [AX_WINDOW_MINIMIZED_INDEX]      = kAXWindowMiniaturizedNotification,
    [AX_WINDOW_DEMINIMIZED_INDEX]    = kAXWindowDeminiaturizedNotification,
    [AX_WINDOW_SHEET_CREATED_INDEX]  = kAXSheetCreatedNotification,
    [AX_WINDOW_DRAWER_CREATED_INDEX] = kAXDrawerCreatedNotification
};

struct ax_window
{
    struct ax_application *application;
    AXUIElementRef ref;
    uint32_t id;
    volatile uint32_t **id_ptr;
    uint8_t notification;
    struct border border;
    bool is_minimized;
};

CFStringRef window_display_uuid(struct ax_window *window);
int window_display_id(struct ax_window *window);
uint64_t *window_space_list(struct ax_window *window, int *count);
char *window_title(struct ax_window *window);
CGRect window_ax_frame(struct ax_window *window);
CGRect window_frame(struct ax_window *window);
CFStringRef window_role(struct ax_window *window);
CFStringRef window_subrole(struct ax_window *window);
bool window_can_move(struct ax_window *window);
bool window_can_resize(struct ax_window *window);
bool window_is_minimized(struct ax_window *window);
bool window_is_fullscreen(struct ax_window *window);
bool window_is_sticky(struct ax_window *window);
bool window_is_standard(struct ax_window *window);
bool window_is_dialog(struct ax_window *window);
bool window_observe(struct ax_window *window);
void window_unobserve(struct ax_window *window);
struct ax_window *window_create(struct ax_application *application, AXUIElementRef window_ref);
void window_destroy(struct ax_window *window);

#endif

#ifndef BORDER_H
#define BORDER_H

extern CGError SLSDisableUpdate(int cid);
extern CGError SLSReenableUpdate(int cid);
extern CGError SLSNewWindow(int cid, int type, float x, float y, CFTypeRef region, uint32_t *wid);
extern CGError SLSReleaseWindow(int cid, uint32_t wid);
extern CGError SLSSetWindowTags(int cid, uint32_t wid, uint64_t *tags, int tag_size);
extern CGError SLSSetWindowShape(int cid, uint32_t wid, float x_offset, float y_offset, CFTypeRef shape);
extern CGError SLSSetWindowOpacity(int cid, uint32_t wid, bool isOpaque);
extern CGError SLSOrderWindow(int cid, uint32_t wid, int mode, uint32_t relativeToWID);
extern CGError SLSSetWindowLevel(int cid, uint32_t wid, int level);
extern CGContextRef SLWindowContextCreate(int cid, uint32_t wid, CFDictionaryRef options);
extern CGError CGSNewRegionWithRect(CGRect *rect, CFTypeRef *outRegion);

#define kCGSIgnoreForExposeTagBit (1 << 7)
#define kCGSIgnoreForEventsTagBit (1 << 9)
#define kCGSDisableShadowTagBit   (1 << 3)

struct border
{
    uint32_t id;
    CGContextRef context;
    CFTypeRef region;
    CGRect frame;
    CGMutablePathRef path;
};

struct window;
void border_redraw(struct window *window);
void border_activate(struct window *window);
void border_deactivate(struct window *window);
void border_enter_fullscreen(struct window *window);
void border_exit_fullscreen(struct window *window);
void border_create(struct window *window);
void border_resize(struct window *window);
void border_destroy(struct window *window);

#endif

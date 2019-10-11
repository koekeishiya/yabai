#ifndef BORDER_WINDOW_H
#define BORDER_WINDOW_H

extern CGError SLSDisableUpdate(int cid);
extern CGError SLSReenableUpdate(int cid);
extern CGError SLSNewWindow(int cid, int type, float x, float y, CFTypeRef region, uint32_t *wid);
extern CGError SLSReleaseWindow(int cid, uint32_t wid);
extern CGError SLSSetWindowTags(int cid, uint32_t wid, uint32_t tags[2], int tag_size);
extern CGError SLSClearWindowTags(int cid, uint32_t wid, uint32_t tags[2], int tag_size);
extern CGError SLSSetWindowShape(int cid, uint32_t wid, float x_offset, float y_offset, CFTypeRef shape);
extern CGError SLSSetWindowResolution(int cid, uint32_t wid, double res);
extern CGError SLSSetWindowOpacity(int cid, uint32_t wid, bool isOpaque);
extern CGError SLSSetMouseEventEnableFlags(int cid, uint32_t wid, bool shouldEnable);
extern CGError SLSOrderWindow(int cid, uint32_t wid, int mode, uint32_t relativeToWID);
extern CGError SLSSetWindowLevel(int cid, uint32_t wid, int level);
extern CGContextRef SLWindowContextCreate(int cid, uint32_t wid, CFDictionaryRef options);
extern CGError CGSNewRegionWithRect(CGRect *rect, CFTypeRef *outRegion);

#define kCGSModalWindowTagBit           (1 << 31)
#define kCGSDisableShadowTagBit         (1 <<  3)
#define kCGSHighQualityResamplingTagBit (1 <<  4)
#define kCGSIgnoreForExposeTagBit       (1 <<  7)
#define kCGSStickyTagBit                (1 << 11)

enum border_placement
{
    BORDER_PLACEMENT_EXTERIOR = 0,
    BORDER_PLACEMENT_INTERIOR = 1,
    BORDER_PLACEMENT_INSET    = 2,
};

static const char *border_placement_str[] =
{
    "exterior",
    "interior",
    "inset"
};

struct border
{
    CGContextRef context;
    uint32_t id;
    int width;
    float radius;
    struct rgba_color color;
    struct rgba_color insert_color;
    bool insert_active;
    int insert_dir;
    bool enabled;
};

struct window;

void border_window_refresh(struct window *window);
void border_window_activate(struct window *window);
void border_window_deactivate(struct window *window);
void border_window_show(struct window *window);
void border_window_hide(struct window *window);

#endif

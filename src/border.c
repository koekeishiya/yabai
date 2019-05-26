#include "border.h"

extern struct window_manager g_window_manager;
extern int g_connection;

static struct rgba_color
rgba_color_from_hex(uint32_t color)
{
    struct rgba_color result;
    result.r = ((color >> 16) & 0xff) / 255.0;
    result.g = ((color >> 8) & 0xff) / 255.0;
    result.b = ((color >> 0) & 0xff) / 255.0;
    result.a = ((color >> 24) & 0xff) / 255.0;
    return result;
}

static int border_detect_window_level(struct ax_window *window, bool topmost)
{
    int window_level = 0;
    SLSGetWindowLevel(g_connection, window->id, &window_level);

    int key = topmost ? kCGModalPanelWindowLevelKey : window_level == 0 ? kCGNormalWindowLevelKey : kCGModalPanelWindowLevelKey;
    return CGWindowLevelForKey(key);
}

static void border_window_ensure_same_space(struct ax_window *window)
{
    int w_space_count;
    uint64_t *w_space_list = window_space_list(window, &w_space_count);
    if (!w_space_list) return;

    uint64_t w_sid = *w_space_list;
    free(w_space_list);

    if (w_space_count > 1) {
        uint32_t tags[2] = { kCGSStickyTagBit };
        SLSSetWindowTags(g_connection, window->border.id, tags, 32);
        return;
    } else {
        uint32_t tags[2] = { kCGSStickyTagBit };
        SLSClearWindowTags(g_connection, window->border.id, tags, 32);
    }

    bool found = false;
    uint64_t b_sid = 0;

    CFNumberRef border_id_ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &window->border.id);
    CFArrayRef border_list_ref = CFArrayCreate(NULL, (void *)&border_id_ref, 1, NULL);
    CFArrayRef border_space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, border_list_ref);

    for (int i = 0; i < CFArrayGetCount(border_space_list_ref); ++i) {
        CFNumberRef id_ref = CFArrayGetValueAtIndex(border_space_list_ref, i);
        CFNumberGetValue(id_ref, CFNumberGetType(id_ref), &b_sid);
        CFRelease(id_ref);

        if (b_sid == w_sid) {
            found = true;
            break;
        }
    }

    if (!found) {
        CGSMoveWindowsToManagedSpace(g_connection, border_list_ref, w_sid);
    }

    CFRelease(border_list_ref);
    CFRelease(border_id_ref);
    CFRelease(border_space_list_ref);
}

static CGMutablePathRef border_normal_shape(CGRect frame, float radius)
{
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathAddRoundedRect(path, NULL, frame, radius, radius);
    return path;
}

static CGMutablePathRef border_insert_shape(struct border *border, CGRect frame, float radius)
{
    CGMutablePathRef insert = CGPathCreateMutable();
    CGFloat minx = CGRectGetMinX(frame), midx = CGRectGetMidX(frame), maxx = CGRectGetMaxX(frame);
    CGFloat miny = CGRectGetMinY(frame), midy = CGRectGetMidY(frame), maxy = CGRectGetMaxY(frame);

    switch (border->insert_dir) {
    case DIR_NORTH: {
        CGPathMoveToPoint(insert, NULL, maxx, midy);
        CGPathAddArcToPoint(insert, NULL, maxx, maxy, midx, maxy, radius);
        CGPathAddArcToPoint(insert, NULL, minx, maxy, minx, midy, radius);
        CGPathAddArcToPoint(insert, NULL, minx, midy, minx, miny, radius);
    } break;
    case DIR_EAST: {
        CGPathMoveToPoint(insert, NULL, midx, miny);
        CGPathAddArcToPoint(insert, NULL, maxx, miny, maxx, midy, radius);
        CGPathAddArcToPoint(insert, NULL, maxx, maxy, midx, maxy, radius);
        CGPathAddArcToPoint(insert, NULL, midx, maxy, maxx, maxy, radius);
    } break;
    case DIR_SOUTH: {
        CGPathMoveToPoint(insert, NULL, minx, midy);
        CGPathAddArcToPoint(insert, NULL, minx, miny, midx, miny, radius);
        CGPathAddArcToPoint(insert, NULL, maxx, miny, maxx, midy, radius);
        CGPathAddArcToPoint(insert, NULL, maxx, midy, maxx, maxy, radius);
    } break;
    case DIR_WEST: {
        CGPathMoveToPoint(insert, NULL, midx, miny);
        CGPathAddArcToPoint(insert, NULL, minx, miny, minx, midy, radius);
        CGPathAddArcToPoint(insert, NULL, minx, maxy, midx, maxy, radius);
        CGPathAddArcToPoint(insert, NULL, midx, maxy, midx, maxy, radius);
    } break;
    }

    return insert;
}

static float border_radius_clamp(CGRect frame, float radius)
{
    if (radius * 2 > CGRectGetWidth(frame)) {
        radius = CGRectGetWidth(frame) / 2;
    }

    if (radius * 2 > CGRectGetHeight(frame)) {
        radius = CGRectGetHeight(frame) / 2;
    }

    return radius;
}

void border_window_refresh(struct ax_window *window)
{
    if (!window->border.id) return;
    if (!window->border.enabled) return;
    struct border *border = &window->border;
    border_window_ensure_same_space(window);

    CGRect region = window_ax_frame(window);

    CGRect border_frame = CGRectInset(((CGRect) {
        { border->width, border->width },
        { region.size.width  - border->width,
          region.size.height - border->width }
    }), -0.5f * border->width, -0.5f * border->width);

    CGRect clear_region = {
        { 0, 0 },
        { region.size.width  + border->width,
          region.size.height + border->width }
    };

    region.origin.x -= border->width / 2;
    region.origin.y -= border->width / 2;
    region.size.width  += border->width;
    region.size.height += border->width;

    CFTypeRef region_ref;
    CGSNewRegionWithRect(&region, &region_ref);

    float radius = border_radius_clamp(border_frame, 1.25f * border->width);
    CGMutablePathRef path = border_normal_shape(border_frame, radius);

    SLSOrderWindow(g_connection, border->id, 0, window->id);
    SLSSetWindowShape(g_connection, border->id, 0.0f, 0.0f, region_ref);
    CGContextClearRect(border->context, clear_region);

    CGContextAddPath(border->context, path);
    CGContextStrokePath(border->context);

    if (border->insert_active) {
        CGMutablePathRef insert = border_insert_shape(border, border_frame, radius);
        CGContextSetRGBStrokeColor(border->context, border->insert_color.r, border->insert_color.g, border->insert_color.b, border->insert_color.a);
        CGContextAddPath(border->context, insert);
        CGContextStrokePath(border->context);
        CGContextSetRGBStrokeColor(border->context, border->color.r, border->color.g, border->color.b, border->color.a);
        CGPathRelease(insert);
    }

    CGContextFlush(border->context);
    SLSOrderWindow(g_connection, border->id, 1, window->id);

    CGSReleaseRegion(region_ref);
    CGPathRelease(path);
}

void border_window_activate(struct ax_window *window)
{
    if (!window->border.id) return;
    if (!window->border.enabled) return;

    struct border *border = &window->border;
    border->color = rgba_color_from_hex(g_window_manager.active_window_border_color);
    CGContextSetRGBStrokeColor(border->context, border->color.r, border->color.g, border->color.b, border->color.a);
    SLSSetWindowLevel(g_connection, window->border.id, border_detect_window_level(window, true));

    if (window_is_fullscreen(window)) {
        border_window_hide(window);
    } else {
        border_window_refresh(window);
    }
}

void border_window_deactivate(struct ax_window *window)
{
    if (!window->border.id) return;
    if (!window->border.enabled) return;

    struct border *border = &window->border;
    border->color = rgba_color_from_hex(g_window_manager.normal_window_border_color);
    CGContextSetRGBStrokeColor(border->context, border->color.r, border->color.g, border->color.b, border->color.a);
    SLSSetWindowLevel(g_connection, window->border.id, border_detect_window_level(window, false));

    if (window_is_fullscreen(window)) {
        border_window_hide(window);
    } else {
        border_window_refresh(window);
    }
}

void border_window_show(struct ax_window *window)
{
    if (!window->border.id) return;
    if (!window->border.enabled) return;
    SLSOrderWindow(g_connection, window->border.id, 1, window->id);
}

void border_window_hide(struct ax_window *window)
{
    if (!window->border.id) return;
    if (!window->border.enabled) return;
    SLSOrderWindow(g_connection, window->border.id, 0, window->id);
}

void border_window_create(struct ax_window *window)
{
    if (!g_window_manager.enable_window_border) return;

    struct border *border = &window->border;
    border->color = rgba_color_from_hex(g_window_manager.normal_window_border_color);
    border->insert_color = rgba_color_from_hex(g_window_manager.insert_window_border_color);
    border->width = g_window_manager.window_border_width;
    border->enabled = true;

    CFTypeRef frame_region;
    CGRect frame = window_frame(window);
    CGSNewRegionWithRect(&frame, &frame_region);

    uint32_t tags[2] = {
          kCGSModalWindowTagBit |
          kCGSDisableShadowTagBit |
          kCGSHighQualityResamplingTagBit |
          kCGSIgnoreForExposeTagBit
    };

    SLSNewWindow(g_connection, 2, 0.0f, 0.0f, frame_region, &border->id);
    SLSSetWindowResolution(g_connection, border->id, 2.0f);
    SLSSetWindowTags(g_connection, border->id, tags, 32);
    SLSSetWindowOpacity(g_connection, border->id, 0);
    SLSSetMouseEventEnableFlags(g_connection, border->id, false);
    SLSSetWindowLevel(g_connection, border->id, border_detect_window_level(window, false));
    border->context = SLWindowContextCreate(g_connection, border->id, 0);
    CGContextSetAllowsAntialiasing(border->context, true);
    CGContextSetShouldAntialias(border->context, true);
    CGContextSetLineWidth(border->context, border->width);
    CGContextSetRGBStrokeColor(border->context, border->color.r, border->color.g, border->color.b, border->color.a);
    CGSReleaseRegion(frame_region);
}

void border_window_destroy(struct ax_window *window)
{
    if (window->border.id) {
        CGContextRelease(window->border.context);
        SLSReleaseWindow(g_connection, window->border.id);
    }
}

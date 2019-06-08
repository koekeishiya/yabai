#include "bar.h"

static POWER_CALLBACK(power_handler)
{
    struct event *event;
    event_create(event, BAR_REFRESH, NULL);
    event_loop_post(&g_event_loop, event);
}

static TIMER_CALLBACK(timer_handler)
{
    struct event *event;
    event_create(event, BAR_REFRESH, NULL);
    event_loop_post(&g_event_loop, event);
}

static int bar_find_battery_life(bool *charging)
{
    CFTypeRef ps_info = IOPSCopyPowerSourcesInfo();
    CFTypeRef ps_list = IOPSCopyPowerSourcesList(ps_info);

    int ps_count = CFArrayGetCount(ps_list);
    if (!ps_count) return 0;

    int cur_capacity = 0;
    int max_capacity = 0;
    int percent = 0;

    for (int i = 0; i < ps_count; ++i) {
        CFDictionaryRef ps = IOPSGetPowerSourceDescription(ps_info, CFArrayGetValueAtIndex(ps_list, i));
        if (!ps) continue;

        CFTypeRef ps_type = CFDictionaryGetValue(ps, CFSTR(kIOPSTypeKey));
        if (!ps_type || !CFEqual(ps_type, CFSTR(kIOPSInternalBatteryType))) continue;

        CFTypeRef ps_cur = CFDictionaryGetValue(ps, CFSTR(kIOPSCurrentCapacityKey));
        if (!ps_cur) continue;

        CFTypeRef ps_max = CFDictionaryGetValue(ps, CFSTR(kIOPSMaxCapacityKey));
        if (!ps_max) continue;

        CFTypeRef ps_charging = CFDictionaryGetValue(ps, CFSTR(kIOPSPowerSourceStateKey));
        if (!ps_charging) continue;

        CFNumberGetValue((CFNumberRef) ps_cur, kCFNumberSInt32Type, &cur_capacity);
        CFNumberGetValue((CFNumberRef) ps_max, kCFNumberSInt32Type, &max_capacity);
        *charging = !CFEqual(ps_charging, CFSTR(kIOPSBatteryPowerValue));
        percent = (int)((double) cur_capacity / (double) max_capacity * 100);
        break;
    }

    CFRelease(ps_list);
    CFRelease(ps_info);
    return percent;
}

static CTFontRef bar_create_font(char *cstring, float size)
{
    const void *keys[] = { kCTFontFamilyNameAttribute, kCTFontSizeAttribute };
    const void *values[] = { CFStringCreateWithCString(NULL, cstring, kCFStringEncodingUTF8), CFNumberCreate(NULL, kCFNumberFloat32Type, &size) };
    CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);
    CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);
    CFRelease(attributes);
    CFRelease(descriptor);
    return font;
}

static CGPoint bar_align_line(struct bar *bar, struct bar_line line, int align_x, int align_y)
{
    float x = 0, y = 0;

    if (align_x == ALIGN_NONE) {
        x = CGContextGetTextPosition(bar->context).x;
    } else if (align_x == ALIGN_LEFT) {
        x = 20;
    } else if (align_x == ALIGN_CENTER) {
        x = (bar->frame.size.width / 2) - (line.bounds.size.width  / 2);
    } else if (align_x == ALIGN_RIGHT) {
        x = bar->frame.size.width - line.bounds.size.width - 20;
    }

    if (align_y == ALIGN_NONE) {
        y = CGContextGetTextPosition(bar->context).y;
    } else if (align_y == ALIGN_TOP) {
        y = bar->frame.size.height;
    } else if (align_y == ALIGN_CENTER) {
        y = (bar->frame.size.height / 2) - ((line.ascent - line.descent) / 2);
    } else if (align_y == ALIGN_BOTTOM) {
        y = line.descent;
    }

    return (CGPoint) { x, y };
}

static void bar_draw_line(struct bar *bar, struct bar_line line, float x, float y)
{
    CGContextSetRGBFillColor(bar->context, line.color.r, line.color.g, line.color.b, line.color.a);
    CGContextSetTextPosition(bar->context, x, y);
    CTLineDraw(line.line, bar->context);
}

static void bar_destroy_line(struct bar_line line)
{
    CFRelease(line.line);
}

static struct bar_line bar_prepare_line(CTFontRef font, char *cstring, struct rgba_color color)
{
    const void *keys[] = { kCTFontAttributeName, kCTForegroundColorFromContextAttributeName };
    const void *values[] = { font, kCFBooleanTrue };
    CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFStringRef string = CFStringCreateWithCString(NULL, cstring, kCFStringEncodingUTF8);
    CFAttributedStringRef attr_string = CFAttributedStringCreate(NULL, string, attributes);
    CTLineRef line = CTLineCreateWithAttributedString(attr_string);

    CGFloat ascent, descent;
    CTLineGetTypographicBounds(line, &ascent, &descent, NULL);
    CGRect bounds = CTLineGetBoundsWithOptions(line, kCTLineBoundsUseGlyphPathBounds);

    CFRelease(string);
    CFRelease(attributes);
    CFRelease(attr_string);

    return (struct bar_line) {
        .line = line,
        .ascent = ascent,
        .descent = descent,
        .bounds = bounds,
        .color = color
    };
}

void bar_refresh(struct bar *bar)
{
    if (!bar->enabled) return;

    SLSDisableUpdate(g_connection);
    SLSOrderWindow(g_connection, bar->id, -1, 0);
    CGContextClearRect(bar->context, bar->frame);
    CGContextSetRGBFillColor(bar->context, bar->background_color.r, bar->background_color.g, bar->background_color.b, bar->background_color.a);
    CGContextFillRect(bar->context, bar->frame);
    CGContextSetRGBStrokeColor(bar->context, bar->background_color_dim.r, bar->background_color_dim.g, bar->background_color_dim.b, bar->background_color_dim.a);
    CGContextSetLineWidth(bar->context, 2.0f);
    CGContextMoveToPoint(bar->context, 0, 1);
    CGContextAddLineToPoint(bar->context, bar->frame.size.width, 1);
    CGContextStrokePath(bar->context);

    // BAR LEFT
    int space_count = display_space_count(display_manager_main_display_id());
    int mci = space_manager_mission_control_index(display_space_id(display_manager_main_display_id()));

    for (int i = 0; i < space_count; ++i) {
        CGPoint pos = CGContextGetTextPosition(bar->context);
        struct bar_line space_line = i >= buf_len(bar->space_icon_strip)
                                   ? bar->space_icon
                                   : bar->space_icon_strip[i];
        if (i == 0) {
            pos = bar_align_line(bar, space_line, ALIGN_LEFT, ALIGN_CENTER);
        } else {
            pos.x += 25;
        }

        bar_draw_line(bar, space_line, pos.x, pos.y);

        if ((i+1) == mci) {
            CGPoint new_pos = CGContextGetTextPosition(bar->context);
            struct bar_line mark_line = bar->space_underline;
            CGPoint mark_pos = bar_align_line(bar, mark_line, 0, ALIGN_BOTTOM);
            mark_pos.x = mark_pos.x - mark_line.bounds.size.width / 2 - space_line.bounds.size.width / 2;
            bar_draw_line(bar, mark_line, mark_pos.x, mark_pos.y);
            CGContextSetTextPosition(bar->context, new_pos.x, new_pos.y);
        }
    }

    // BAR CENTER
    struct ax_window *window = window_manager_focused_window(&g_window_manager);
    char *title = window ? window_title(window) : NULL;
    if (title) {
        struct bar_line title_line = bar_prepare_line(bar->n_font, title, bar->foreground_color);
        CGPoint pos = bar_align_line(bar, title_line, ALIGN_CENTER, ALIGN_CENTER);
        bar_draw_line(bar, bar->focus_icon, pos.x - bar->focus_icon.bounds.size.width - 5, pos.y);
        bar_draw_line(bar, title_line, pos.x, pos.y);
        bar_destroy_line(title_line);
        free(title);
    }

    // BAR RIGHT
    time_t rawtime;
    time(&rawtime);
    float time_line_width = 0;
    struct tm *timeinfo = localtime(&rawtime);
    if (timeinfo) {
        char time[255];
        snprintf(time, sizeof(time), "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
        struct bar_line time_line = bar_prepare_line(bar->n_font, time, bar->foreground_color);
        CGPoint t_pos = bar_align_line(bar, time_line, ALIGN_RIGHT, ALIGN_CENTER);
        bar_draw_line(bar, time_line, t_pos.x, t_pos.y);

        CGPoint ti_pos = bar_align_line(bar, bar->clock_icon, 0, ALIGN_CENTER);
        t_pos.x = t_pos.x - bar->clock_icon.bounds.size.width - 5;

        CGPoint tu_pos = bar_align_line(bar, bar->clock_underline, 0, ALIGN_BOTTOM);
        tu_pos.x = tu_pos.x - bar->clock_underline.bounds.size.width / 2 - time_line.bounds.size.width / 2 - (bar->clock_icon.bounds.size.width + 5) / 2;

        bar_draw_line(bar, bar->clock_icon, t_pos.x, ti_pos.y);
        bar_draw_line(bar, bar->clock_underline, tu_pos.x, tu_pos.y);
        time_line_width = time_line.bounds.size.width;
        bar_destroy_line(time_line);
    }

    bool charging;
    char batt[255];
    snprintf(batt, sizeof(batt), "%' '3d%%", bar_find_battery_life(&charging));
    struct bar_line batt_line = bar_prepare_line(bar->n_font, batt, bar->foreground_color);
    CGPoint p_pos = bar_align_line(bar, batt_line, ALIGN_RIGHT, ALIGN_CENTER);
    p_pos.x = p_pos.x - time_line_width - bar->clock_underline.bounds.size.width - 20;
    bar_draw_line(bar, batt_line, p_pos.x, p_pos.y);

    struct bar_line batt_icon = charging ? bar->power_icon : bar->battr_icon;
    CGPoint pi_pos = bar_align_line(bar, batt_icon, 0, ALIGN_CENTER);
    p_pos.x = p_pos.x - batt_icon.bounds.size.width - 5;

    CGPoint pu_pos = bar_align_line(bar, bar->power_underline, 0, ALIGN_BOTTOM);
    pu_pos.x = pu_pos.x - bar->power_underline.bounds.size.width / 2 - batt_line.bounds.size.width / 2 - (batt_icon.bounds.size.width + 5) / 2;

    bar_draw_line(bar, batt_icon, p_pos.x, pi_pos.y);
    bar_draw_line(bar, bar->power_underline, pu_pos.x, pu_pos.y);
    bar_destroy_line(batt_line);

    CGContextFlush(bar->context);
    SLSOrderWindow(g_connection, bar->id, 1, 0);
    SLSReenableUpdate(g_connection);
}

void bar_resize(struct bar *bar)
{
    if (!bar->enabled) return;

    CFTypeRef frame_region;
    CGRect bounds = display_bounds(display_manager_main_display_id());
    bar->frame = (CGRect) {{0,0},{bounds.size.width, 26}};
    CGSNewRegionWithRect(&bar->frame, &frame_region);
    SLSDisableUpdate(g_connection);
    SLSOrderWindow(g_connection, bar->id, -1, 0);
    SLSSetWindowShape(g_connection, bar->id, 0.0f, 0.0f, frame_region);
    bar_refresh(bar);
    SLSOrderWindow(g_connection, bar->id, 1, 0);
    SLSReenableUpdate(g_connection);
    CFRelease(frame_region);
}

void bar_create(struct bar *bar)
{
    if (!bar->enabled) return;

    if (!bar->n_font_name) bar->n_font_name = string_copy("Helvetica Neue");
    if (!bar->i_font_name) bar->i_font_name = string_copy("FontAwesome");

    if (!bar->background_color.is_valid)     bar->background_color     = rgba_color_from_hex(0xff202020);
    if (!bar->background_color_dim.is_valid) bar->background_color_dim = rgba_color_dim(bar->background_color);
    if (!bar->foreground_color.is_valid)     bar->foreground_color     = rgba_color_from_hex(0xffa8a8a8);

    bar->refresh_frequency = 5;
    bar->n_font_size = 10.0f;
    bar->i_font_size = 10.0f;

    bar->n_font = bar_create_font(bar->n_font_name, bar->n_font_size);
    bar->i_font = bar_create_font(bar->i_font_name, bar->i_font_size);

    if (buf_len(bar->_power_icon_strip) == 2) {
        bar->battr_icon = bar_prepare_line(bar->i_font, bar->_power_icon_strip[0], rgba_color_from_hex(0xffd75f5f));
        bar->power_icon = bar_prepare_line(bar->i_font, bar->_power_icon_strip[1], rgba_color_from_hex(0xffcd950c));
    } else {
        bar->battr_icon = bar_prepare_line(bar->i_font, "", rgba_color_from_hex(0xffd75f5f));
        bar->power_icon = bar_prepare_line(bar->i_font, "", rgba_color_from_hex(0xffcd950c));
    }

    if (bar->_focus_icon) {
        bar->focus_icon = bar_prepare_line(bar->i_font, bar->_focus_icon, bar->foreground_color);
    } else {
        bar->focus_icon = bar_prepare_line(bar->i_font, "", bar->foreground_color);
    }

    if (bar->_clock_icon) {
        bar->clock_icon = bar_prepare_line(bar->i_font, bar->_clock_icon, bar->foreground_color);
    } else {
        bar->clock_icon = bar_prepare_line(bar->i_font, "", bar->foreground_color);
    }

    if (bar->_space_icon) {
        bar->space_icon = bar_prepare_line(bar->i_font, bar->_space_icon, bar->foreground_color);
    } else {
        bar->space_icon = bar_prepare_line(bar->i_font, "*", bar->foreground_color);
    }

    bar->space_underline = bar_prepare_line(bar->n_font, "______", rgba_color_from_hex(0xffd4d232));
    bar->power_underline = bar_prepare_line(bar->n_font, "__________", rgba_color_from_hex(0xffd75f5f));
    bar->clock_underline = bar_prepare_line(bar->n_font, "__________", rgba_color_from_hex(0xff458588));

    for (int i = 0; i < buf_len(bar->_space_icon_strip); ++i) {
        struct bar_line space_line = bar_prepare_line(bar->i_font, bar->_space_icon_strip[i], bar->foreground_color);
        buf_push(bar->space_icon_strip, space_line);
    }

    uint32_t tags[2] = {
        kCGSStickyTagBit |
        kCGSModalWindowTagBit |
        kCGSDisableShadowTagBit |
        kCGSHighQualityResamplingTagBit |
        kCGSIgnoreForExposeTagBit
    };

    CFTypeRef frame_region;
    CGRect bounds = display_bounds(display_manager_main_display_id());
    bar->frame = (CGRect) {{0,0},{bounds.size.width, 26}};

    CGSNewRegionWithRect(&bar->frame, &frame_region);
    SLSNewWindow(g_connection, 2, 0.0f, 0.0f, frame_region, &bar->id);
    CFRelease(frame_region);

    SLSSetWindowResolution(g_connection, bar->id, 2.0f);
    SLSSetWindowTags(g_connection, bar->id, tags, 32);
    SLSSetWindowOpacity(g_connection, bar->id, 0);
    SLSSetMouseEventEnableFlags(g_connection, bar->id, false);
    SLSSetWindowLevel(g_connection, bar->id, CGWindowLevelForKey(5));
    bar->context = SLWindowContextCreate(g_connection, bar->id, 0);

    CFRunLoopAddSource(CFRunLoopGetMain(), IOPSNotificationCreateRunLoopSource(power_handler, NULL), kCFRunLoopCommonModes);
    CFRunLoopTimerRef refresh_timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + bar->refresh_frequency, bar->refresh_frequency, 0, 0, timer_handler, NULL);
    CFRunLoopAddTimer(CFRunLoopGetMain(), refresh_timer, kCFRunLoopCommonModes);
    bar_refresh(bar);
}

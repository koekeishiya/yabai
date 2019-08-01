#include "bar.h"

extern struct space_manager g_space_manager;

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

static int bar_find_battery_life(bool *has_battery, bool *charging)
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
        *has_battery = true;
        percent = (int)((double) cur_capacity / (double) max_capacity * 100);
        break;
    }

    CFRelease(ps_list);
    CFRelease(ps_info);
    return percent;
}

static CTFontRef bar_create_font(char *cstring)
{
    float size = 10.0f;
    char font_properties[2][255] = { {}, {} };
    sscanf(cstring, "%254[^:]:%254[^:]:%f", font_properties[0], font_properties[1], &size);
    CFStringRef font_family_name = CFStringCreateWithCString(NULL, font_properties[0], kCFStringEncodingUTF8);
    CFStringRef font_style_name = CFStringCreateWithCString(NULL, font_properties[1], kCFStringEncodingUTF8);
    CFNumberRef font_size = CFNumberCreate(NULL, kCFNumberFloat32Type, &size);

    const void *keys[] = { kCTFontFamilyNameAttribute, kCTFontStyleNameAttribute, kCTFontSizeAttribute };
    const void *values[] = { font_family_name, font_style_name, font_size };
    CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);
    CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);

    CFRelease(descriptor);
    CFRelease(attributes);
    CFRelease(font_size);
    CFRelease(font_style_name);
    CFRelease(font_family_name);

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
    int space_count;
    uint64_t *space_list = display_space_list(bar->did, &space_count);
    if (space_list) {
        uint64_t sid = display_space_id(bar->did);

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

            if (sid == space_list[i]) {
                CGPoint new_pos = CGContextGetTextPosition(bar->context);
                struct bar_line mark_line = bar->space_underline;
                CGPoint mark_pos = bar_align_line(bar, mark_line, 0, ALIGN_BOTTOM);
                mark_pos.x = mark_pos.x - mark_line.bounds.size.width / 2 - space_line.bounds.size.width / 2;
                bar_draw_line(bar, mark_line, mark_pos.x, mark_pos.y);
                CGContextSetTextPosition(bar->context, new_pos.x, new_pos.y);
            }
        }

        free(space_list);
    }

    // BAR CENTER
    struct window *window = window_manager_focused_window(&g_window_manager);
    char *title = window ? window_title(window) : NULL;
    if (title) {
        struct bar_line title_line = bar_prepare_line(bar->t_font, title, bar->foreground_color);
        CGPoint pos = bar_align_line(bar, title_line, ALIGN_CENTER, ALIGN_CENTER);
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
        struct bar_line time_line = bar_prepare_line(bar->t_font, time, bar->foreground_color);
        CGPoint t_pos = bar_align_line(bar, time_line, ALIGN_RIGHT, ALIGN_CENTER);
        bar_draw_line(bar, time_line, t_pos.x, t_pos.y);

        CGPoint ti_pos = bar_align_line(bar, bar->clock_icon, 0, ALIGN_CENTER);
        ti_pos.x = t_pos.x - bar->clock_icon.bounds.size.width - 5;

        CGPoint tu_pos = bar_align_line(bar, bar->clock_underline, 0, ALIGN_BOTTOM);
        tu_pos.x = tu_pos.x - bar->clock_underline.bounds.size.width / 2 - time_line.bounds.size.width / 2 - (bar->clock_icon.bounds.size.width + 5) / 2;

        bar_draw_line(bar, bar->clock_icon, ti_pos.x, ti_pos.y);
        bar_draw_line(bar, bar->clock_underline, tu_pos.x, tu_pos.y);
        bar_destroy_line(time_line);

        time_line_width = time_line.bounds.size.width;
    }

    bool has_batt = false;
    bool charging = false;
    int percent = bar_find_battery_life(&has_batt, &charging);
    if (has_batt) {
        char batt[255];
        snprintf(batt, sizeof(batt), "%' '3d%%", percent);

        struct bar_line batt_line = bar_prepare_line(bar->t_font, batt, bar->foreground_color);
        CGPoint p_pos = bar_align_line(bar, batt_line, ALIGN_RIGHT, ALIGN_CENTER);
        p_pos.x = p_pos.x - time_line_width - bar->clock_underline.bounds.size.width - 20;
        bar_draw_line(bar, batt_line, p_pos.x, p_pos.y);

        struct bar_line batt_icon = charging ? bar->power_icon : bar->battr_icon;
        CGPoint pi_pos = bar_align_line(bar, batt_icon, 0, ALIGN_CENTER);
        pi_pos.x = p_pos.x - batt_icon.bounds.size.width - 5;

        CGPoint pu_pos = bar_align_line(bar, bar->power_underline, 0, ALIGN_BOTTOM);
        pu_pos.x = pu_pos.x - bar->power_underline.bounds.size.width / 2 - batt_line.bounds.size.width / 2 - (batt_icon.bounds.size.width + 5) / 2;

        bar_draw_line(bar, batt_icon, pi_pos.x, pi_pos.y);
        bar_draw_line(bar, bar->power_underline, pu_pos.x, pu_pos.y);
        bar_destroy_line(batt_line);
    }

    CGContextFlush(bar->context);
    SLSOrderWindow(g_connection, bar->id, 1, bar->id);
    SLSReenableUpdate(g_connection);
}

void bar_resize(struct bar *bar)
{
    if (!bar->enabled) return;

    bar->did = display_manager_main_display_id();
    CGRect bounds = display_bounds(bar->did);

    CFTypeRef frame_region;
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

void bar_set_foreground_color(struct bar *bar, uint32_t color)
{
    bar->foreground_color = rgba_color_from_hex(color);
    if (bar->_space_icon_strip) bar_set_space_strip(bar, bar->_space_icon_strip);
    if (bar->_power_icon_strip) bar_set_power_strip(bar, bar->_power_icon_strip);
    if (bar->_clock_icon) bar_set_clock_icon(bar, bar->_clock_icon);
    if (bar->_space_icon) bar_set_space_icon(bar, bar->_space_icon);
    bar_refresh(bar);
}

void bar_set_background_color(struct bar *bar, uint32_t color)
{
    bar->background_color = rgba_color_from_hex(color);
    bar->background_color_dim = rgba_color_dim(bar->background_color);
    bar_refresh(bar);
}

void bar_set_text_font(struct bar *bar, char *font_string)
{
    if (bar->t_font) {
        CFRelease(bar->t_font);
    }

    if (bar->space_underline.line) {
        bar_destroy_line(bar->space_underline);
    }

    if (bar->power_underline.line) {
        bar_destroy_line(bar->power_underline);
    }

    if (bar->clock_underline.line) {
        bar_destroy_line(bar->clock_underline);
    }

    if (font_string != bar->t_font_prop) {
        if (bar->t_font_prop) {
            free(bar->t_font_prop);
        }

        bar->t_font_prop = font_string;
    }

    bar->t_font = bar_create_font(bar->t_font_prop);
    bar->space_underline = bar_prepare_line(bar->t_font, "______", rgba_color_from_hex(0xffd4d232));
    bar->power_underline = bar_prepare_line(bar->t_font, "__________", rgba_color_from_hex(0xffd75f5f));
    bar->clock_underline = bar_prepare_line(bar->t_font, "__________", rgba_color_from_hex(0xff458588));
    bar_refresh(bar);
}

void bar_set_icon_font(struct bar *bar, char *font_string)
{
    if (bar->i_font) {
        CFRelease(bar->i_font);
    }

    if (font_string != bar->i_font_prop) {
        if (bar->i_font_prop) {
            free(bar->i_font_prop);
        }

        bar->i_font_prop = font_string;
    }

    bar->i_font = bar_create_font(bar->i_font_prop);
    if (bar->_space_icon_strip) bar_set_space_strip(bar, bar->_space_icon_strip);
    if (bar->_power_icon_strip) bar_set_power_strip(bar, bar->_power_icon_strip);
    if (bar->_clock_icon) bar_set_clock_icon(bar, bar->_clock_icon);
    if (bar->_space_icon) bar_set_space_icon(bar, bar->_space_icon);
    bar_refresh(bar);
}

void bar_set_space_strip(struct bar *bar, char **icon_strip)
{
    for (int i = 0; i < buf_len(bar->space_icon_strip); ++i) {
        bar_destroy_line(bar->space_icon_strip[i]);
    }

    buf_free(bar->space_icon_strip);
    bar->space_icon_strip = NULL;

    if (icon_strip != bar->_space_icon_strip) {
        for (int i = 0; i < buf_len(bar->_space_icon_strip); ++i) {
            free(bar->_space_icon_strip[i]);
        }

        buf_free(bar->_space_icon_strip);
        bar->_space_icon_strip = icon_strip;
    }

    for (int i = 0; i < buf_len(bar->_space_icon_strip); ++i) {
        struct bar_line space_line = bar_prepare_line(bar->i_font, bar->_space_icon_strip[i], bar->foreground_color);
        buf_push(bar->space_icon_strip, space_line);
    }

    bar_refresh(bar);
}

void bar_set_power_strip(struct bar *bar, char **icon_strip)
{
    if (bar->battr_icon.line) {
        bar_destroy_line(bar->battr_icon);
    }

    if (bar->power_icon.line) {
        bar_destroy_line(bar->power_icon);
    }

    if (icon_strip != bar->_power_icon_strip) {
        for (int i = 0; i < buf_len(bar->_power_icon_strip); ++i) {
            free(bar->_power_icon_strip[i]);
        }

        buf_free(bar->_power_icon_strip);
        bar->_power_icon_strip = icon_strip;
    }

    if (buf_len(bar->_power_icon_strip) == 2) {
        bar->battr_icon = bar_prepare_line(bar->i_font, bar->_power_icon_strip[0], rgba_color_from_hex(0xffd75f5f));
        bar->power_icon = bar_prepare_line(bar->i_font, bar->_power_icon_strip[1], rgba_color_from_hex(0xffcd950c));
    } else {
        bar->battr_icon = bar_prepare_line(bar->i_font, "", rgba_color_from_hex(0xffd75f5f));
        bar->power_icon = bar_prepare_line(bar->i_font, "", rgba_color_from_hex(0xffcd950c));
    }

    bar_refresh(bar);
}

void bar_set_clock_icon(struct bar *bar, char *icon)
{
    if (bar->clock_icon.line) {
        bar_destroy_line(bar->clock_icon);
    }

    if (icon != bar->_clock_icon) {
        if (bar->_clock_icon) {
            free(bar->_clock_icon);
        }

        bar->_clock_icon = icon;
    }

    bar->clock_icon = bar_prepare_line(bar->i_font, bar->_clock_icon, bar->foreground_color);

    bar_refresh(bar);
}

void bar_set_space_icon(struct bar *bar, char *icon)
{
    if (bar->space_icon.line) {
        bar_destroy_line(bar->space_icon);
    }

    if (icon != bar->_space_icon) {
        if (bar->_space_icon) {
            free(bar->_space_icon);
        }

        bar->_space_icon = icon;
    }

    bar->space_icon = bar_prepare_line(bar->i_font, bar->_space_icon, bar->foreground_color);

    bar_refresh(bar);
}

void bar_create(struct bar *bar)
{
    if (bar->enabled) return;

    if (!bar->t_font_prop) bar_set_text_font(bar, string_copy("Helvetica Neue:Regular:10.0"));
    if (!bar->i_font_prop) bar_set_icon_font(bar, string_copy("FontAwesome:Regular:10.0"));
    if (!bar->background_color.is_valid) bar_set_background_color(bar, 0xff202020);
    if (!bar->foreground_color.is_valid) bar_set_foreground_color(bar, 0xffa8a8a8);
    if (!bar->_clock_icon) bar_set_clock_icon(bar, string_copy(" "));
    if (!bar->_space_icon) bar_set_space_icon(bar, string_copy("*"));
    if (!bar->_power_icon_strip) bar_set_power_strip(bar, NULL);

    uint32_t set_tags[2] = {
        kCGSStickyTagBit |
        kCGSModalWindowTagBit |
        kCGSDisableShadowTagBit |
        kCGSHighQualityResamplingTagBit |
        kCGSIgnoreForExposeTagBit
    };

    uint32_t clear_tags[2] = { 0, 0 };
    *((int8_t *)(clear_tags) + 0x5) = 0x20;

    bar->did = display_manager_main_display_id();
    CGRect bounds = display_bounds(bar->did);

    CFTypeRef frame_region;
    bar->frame = (CGRect) {{0,0},{bounds.size.width, 26}};

    CGSNewRegionWithRect(&bar->frame, &frame_region);
    SLSNewWindow(g_connection, 2, 0.0f, 0.0f, frame_region, &bar->id);
    CFRelease(frame_region);

    SLSSetWindowResolution(g_connection, bar->id, 2.0f);
    SLSSetWindowTags(g_connection, bar->id, set_tags, 64);
    SLSClearWindowTags(g_connection, bar->id, clear_tags, 64);
    SLSSetWindowOpacity(g_connection, bar->id, 0);
    SLSSetMouseEventEnableFlags(g_connection, bar->id, false);
    SLSSetWindowLevel(g_connection, bar->id, CGWindowLevelForKey(4));
    bar->context = SLWindowContextCreate(g_connection, bar->id, 0);

    int refresh_frequency = 5;
    bar->power_source = IOPSNotificationCreateRunLoopSource(power_handler, NULL);
    bar->refresh_timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + refresh_frequency, refresh_frequency, 0, 0, timer_handler, NULL);

    bar->enabled = true;

    CFRunLoopAddSource(CFRunLoopGetMain(), bar->power_source, kCFRunLoopCommonModes);
    CFRunLoopAddTimer(CFRunLoopGetMain(), bar->refresh_timer, kCFRunLoopCommonModes);

    space_manager_mark_spaces_invalid_for_display(&g_space_manager, bar->did);
    bar_refresh(bar);
}

void bar_destroy(struct bar *bar)
{
    if (bar->enabled) {
        CFRunLoopRemoveSource(CFRunLoopGetMain(), bar->power_source, kCFRunLoopCommonModes);
        CFRunLoopSourceInvalidate(bar->power_source);

        CFRunLoopRemoveTimer(CFRunLoopGetMain(), bar->refresh_timer, kCFRunLoopCommonModes);
        CFRunLoopTimerInvalidate(bar->refresh_timer);

        CGContextRelease(bar->context);
        SLSReleaseWindow(g_connection, bar->id);

        bar->enabled = false;

        space_manager_mark_spaces_invalid_for_display(&g_space_manager, bar->did);
    }
}

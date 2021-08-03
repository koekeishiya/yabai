#include "border.h"

extern int g_connection;
extern struct window_manager g_window_manager;

static void border_order_in(struct window *window)
{
    SLSOrderWindow(g_connection, window->border.id, 1, window->id);
}

static void border_order_out(struct window *window)
{
    SLSOrderWindow(g_connection, window->border.id, 0, window->id);
}

void border_redraw(struct window *window)
{
    SLSDisableUpdate(g_connection);
    CGContextClearRect(window->border.context, window->border.frame);
    CGContextAddPath(window->border.context, window->border.path);
    CGContextStrokePath(window->border.context);
    CGContextFlush(window->border.context);
    SLSReenableUpdate(g_connection);
}

void border_activate(struct window *window)
{
    if (!window->border.id) return;

    CGContextSetRGBStrokeColor(window->border.context,
                               g_window_manager.active_border_color.r,
                               g_window_manager.active_border_color.g,
                               g_window_manager.active_border_color.b,
                               g_window_manager.active_border_color.a);
    border_redraw(window);
}

void border_deactivate(struct window *window)
{
    if (!window->border.id) return;

    CGContextSetRGBStrokeColor(window->border.context,
                               g_window_manager.normal_border_color.r,
                               g_window_manager.normal_border_color.g,
                               g_window_manager.normal_border_color.b,
                               g_window_manager.normal_border_color.a);
    border_redraw(window);
}

void border_enter_fullscreen(struct window *window)
{
    if (!window->border.id) return;

    scripting_addition_remove_from_window_group(window->border.id, window->id);
    border_order_out(window);
}

void border_exit_fullscreen(struct window *window)
{
    if (!window->border.id) return;

    border_order_in(window);
    scripting_addition_add_to_window_group(window->border.id, window->id);
}

void border_create(struct window *window)
{
    if (window->border.id) return;

    if ((!window_is_standard(window)) && (!window_is_dialog(window))) return;

    CGRect frame = window_ax_frame(window);
    CGSNewRegionWithRect(&frame, &window->border.region);
    window->border.frame.size = frame.size;

    window->border.path = CGPathCreateMutable();
    CGPathAddRoundedRect(window->border.path, NULL, window->border.frame, 0, 0);

    uint64_t tags = kCGSIgnoreForEventsTagBit | kCGSDisableShadowTagBit;
    SLSNewWindow(g_connection, 2, 0, 0, window->border.region, &window->border.id);
    SLSSetWindowTags(g_connection, window->border.id, &tags, 64);
    SLSSetWindowResolution(g_connection, window->border.id, 1.0f);
    SLSSetWindowOpacity(g_connection, window->border.id, 0);
    SLSSetWindowLevel(g_connection, window->border.id, window_level(window));
    window->border.context = SLWindowContextCreate(g_connection, window->border.id, 0);
    CGContextSetLineWidth(window->border.context, g_window_manager.border_width);
    CGContextSetRGBStrokeColor(window->border.context,
                               g_window_manager.normal_border_color.r,
                               g_window_manager.normal_border_color.g,
                               g_window_manager.normal_border_color.b,
                               g_window_manager.normal_border_color.a);
    scripting_addition_add_to_window_group(window->border.id, window->id);

    border_redraw(window);

    if ((!window->application->is_hidden) &&
        (!window->is_minimized) &&
        (!window->is_fullscreen)) {
        border_order_in(window);
    }
}

void border_resize(struct window *window)
{
    if (!window->border.id) return;

    CGRect frame = window_ax_frame(window);
    if ((frame.origin.x  == window->border.frame.origin.x) &&
        (frame.origin.y == window->border.frame.origin.y) &&
        (frame.size.width  == window->border.frame.size.width) &&
        (frame.size.height == window->border.frame.size.height)) return;

    if (window->border.region) CFRelease(window->border.region);
    CGSNewRegionWithRect(&frame, &window->border.region);
    window->border.frame.size = frame.size;

    if (window->border.path) CGPathRelease(window->border.path);
    window->border.path = CGPathCreateMutable();
    CGPathAddRoundedRect(window->border.path, NULL, window->border.frame, 0, 0);

    SLSDisableUpdate(g_connection);
    SLSSetWindowShape(g_connection, window->border.id, 0.0f, 0.0f, window->border.region);
    CGContextClearRect(window->border.context, window->border.frame);
    CGContextAddPath(window->border.context, window->border.path);
    CGContextStrokePath(window->border.context);
    CGContextFlush(window->border.context);
    SLSReenableUpdate(g_connection);
}

void border_destroy(struct window *window)
{
    if (!window->border.id) return;

    if (window->border.region) CFRelease(window->border.region);
    if (window->border.path) CGPathRelease(window->border.path);
    CGContextRelease(window->border.context);
    SLSReleaseWindow(g_connection, window->border.id);
    memset(&window->border, 0, sizeof(struct border));
}

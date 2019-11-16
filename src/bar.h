#ifndef BAR_H
#define BAR_H

#include "cpu.h"

#define POWER_CALLBACK(name) void name(void *context)
typedef POWER_CALLBACK(power_callback);

#define TIMER_CALLBACK(name) void name(CFRunLoopTimerRef timer, void *context)
typedef TIMER_CALLBACK(timer_callback);

#define ALIGN_NONE   0
#define ALIGN_LEFT   1
#define ALIGN_RIGHT  2
#define ALIGN_TOP    3
#define ALIGN_BOTTOM 4
#define ALIGN_CENTER 5

struct bar_line
{
    CTLineRef line;
    CGFloat ascent;
    CGFloat descent;
    CGRect bounds;
    struct rgba_color color;
};

struct bar
{
    bool enabled;
    uint32_t id;
    uint32_t did;
    CGContextRef context;
    CFRunLoopSourceRef power_source;
    CFRunLoopTimerRef refresh_timer;
    CGRect frame;
    char *t_font_prop;
    char *i_font_prop;
    CTFontRef t_font;
    CTFontRef i_font;
    char **_space_icon_strip;
    char **_power_icon_strip;
    char *_clock_icon;
    char *_space_icon;
    struct rgba_color foreground_color;
    struct rgba_color background_color;
    struct rgba_color background_color_dim;
    struct bar_line *space_icon_strip;
    struct bar_line space_icon;
    struct bar_line clock_icon;
    struct bar_line battr_icon;
    struct bar_line power_icon;
    struct bar_line space_underline;
    struct bar_line power_underline;
    struct bar_line clock_underline;
    bool enable_cpu_meter;
    struct cpu_info cpu_info;
    struct rgba_color cpu_user_color;
    struct rgba_color cpu_sys_color;
};

void bar_set_foreground_color(struct bar *bar, uint32_t color);
void bar_set_background_color(struct bar *bar, uint32_t color);
void bar_set_text_font(struct bar *bar, char *font_string);
void bar_set_icon_font(struct bar *bar, char *font_string);
void bar_set_space_strip(struct bar *bar, char **icon_strip);
void bar_set_power_strip(struct bar *bar, char **icon_strip);
void bar_set_clock_icon(struct bar *bar, char *icon);
void bar_set_space_icon(struct bar *bar, char *icon);
void bar_set_cpu_user_color(struct bar *bar, uint32_t color);
void bar_set_cpu_sys_color(struct bar *bar, uint32_t color);

void bar_refresh(struct bar *bar);
void bar_resize(struct bar *bar);
void bar_create(struct bar *bar);
void bar_destroy(struct bar *bar);

#endif

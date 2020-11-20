#ifndef BORDER_H
#define BORDER_H

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

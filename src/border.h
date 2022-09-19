#ifndef BORDER_H
#define BORDER_H

struct border
{
    uint32_t id;
    CFArrayRef id_ref;
    CGContextRef context;
    CFTypeRef region;
    CGRect frame;
    CGMutablePathRef path;
};

struct window;
bool border_should_order_in(struct window *window);
void border_show_all(void);
void border_hide_all(void);
void border_hide(struct window *window);
void border_show(struct window *window);
void border_ensure_same_space(struct window *window);
void border_redraw(struct window *window);
void border_resize(struct window *window);
void border_activate(struct window *window);
void border_deactivate(struct window *window);
void border_create(struct window *window);
void border_destroy(struct window *window);

#endif

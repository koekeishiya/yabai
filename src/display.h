#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_EVENT_HANDLER(name) void name(uint32_t did, CGDisplayChangeSummaryFlags flags, void *context)
typedef DISPLAY_EVENT_HANDLER(display_callback);

#define DISPLAY_PROPERTY_LIST \
    DISPLAY_PROPERTY_ENTRY("id",        DISPLAY_PROPERTY_ID,        0x01) \
    DISPLAY_PROPERTY_ENTRY("uuid",      DISPLAY_PROPERTY_UUID,      0x02) \
    DISPLAY_PROPERTY_ENTRY("index",     DISPLAY_PROPERTY_INDEX,     0x04) \
    DISPLAY_PROPERTY_ENTRY("label",     DISPLAY_PROPERTY_LABEL,     0x08) \
    DISPLAY_PROPERTY_ENTRY("frame",     DISPLAY_PROPERTY_FRAME,     0x10) \
    DISPLAY_PROPERTY_ENTRY("spaces",    DISPLAY_PROPERTY_SPACES,    0x20) \
    DISPLAY_PROPERTY_ENTRY("has-focus", DISPLAY_PROPERTY_HAS_FOCUS, 0x40)

enum display_property
{
#define DISPLAY_PROPERTY_ENTRY(n, p, v) p = v,
    DISPLAY_PROPERTY_LIST
#undef DISPLAY_PROPERTY_ENTRY
};

static uint64_t display_property_val[] =
{
#define DISPLAY_PROPERTY_ENTRY(n, p, v) p,
    DISPLAY_PROPERTY_LIST
#undef DISPLAY_PROPERTY_ENTRY
};

static char *display_property_str[] =
{
#define DISPLAY_PROPERTY_ENTRY(n, p, v) n,
    DISPLAY_PROPERTY_LIST
#undef DISPLAY_PROPERTY_ENTRY
};

void display_serialize(FILE *rsp, uint32_t did, uint64_t flags);
CFStringRef display_uuid(uint32_t did);
uint32_t display_id(CFStringRef uuid);
CGRect display_bounds_constrained(uint32_t did, bool ignore_external_bar);
CGPoint display_center(uint32_t did);
uint64_t display_space_id(uint32_t did);
int display_space_count(uint32_t did);
uint64_t *display_space_list(uint32_t did, int *count);

#endif

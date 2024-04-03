#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_EVENT_HANDLER(name) void name(uint32_t did, CGDisplayChangeSummaryFlags flags, void *context)
typedef DISPLAY_EVENT_HANDLER(display_callback);

enum display_property
{
    DISPLAY_PROPERTY_ID        = 0x01,
    DISPLAY_PROPERTY_UUID      = 0x02,
    DISPLAY_PROPERTY_INDEX     = 0x04,
    DISPLAY_PROPERTY_LABEL     = 0x08,
    DISPLAY_PROPERTY_FRAME     = 0x10,
    DISPLAY_PROPERTY_SPACES    = 0x20,
    DISPLAY_PROPERTY_HAS_FOCUS = 0x40
};

static uint64_t display_property_val[] =
{
    [0] = DISPLAY_PROPERTY_ID,
    [1] = DISPLAY_PROPERTY_UUID,
    [2] = DISPLAY_PROPERTY_INDEX,
    [3] = DISPLAY_PROPERTY_LABEL,
    [4] = DISPLAY_PROPERTY_FRAME,
    [5] = DISPLAY_PROPERTY_SPACES,
    [6] = DISPLAY_PROPERTY_HAS_FOCUS
};

static char *display_property_str[] =
{
    "id",
    "uuid",
    "index",
    "label",
    "frame",
    "spaces",
    "has-focus"
};

void display_serialize(FILE *rsp, uint32_t did, uint64_t flags);
CFStringRef display_uuid(uint32_t did);
uint32_t display_id(CFStringRef uuid);
CGRect display_bounds_constrained(uint32_t did);
CGPoint display_center(uint32_t did);
uint64_t display_space_id(uint32_t did);
int display_space_count(uint32_t did);
uint64_t *display_space_list(uint32_t did, int *count);

#endif

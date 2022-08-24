#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_EVENT_HANDLER(name) void name(uint32_t did, CGDisplayChangeSummaryFlags flags, void *context)
typedef DISPLAY_EVENT_HANDLER(display_callback);

void display_serialize(FILE *rsp, uint32_t did);
CFStringRef display_uuid(uint32_t did);
uint32_t display_id(CFStringRef uuid);
CGRect display_bounds_constrained(uint32_t did);
CGPoint display_center(uint32_t did);
uint64_t display_space_id(uint32_t did);
int display_space_count(uint32_t did);
uint64_t *display_space_list(uint32_t did, int *count);
int display_arrangement(uint32_t did);

#endif

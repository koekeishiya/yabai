#ifndef SPACE_H
#define SPACE_H

extern CFStringRef SLSCopyManagedDisplayForSpace(int cid, uint64_t sid);
extern int SLSSpaceGetType(int cid, uint64_t sid);
extern CFStringRef SLSSpaceCopyName(int cid, uint64_t sid);
extern CFArrayRef SLSCopyWindowsWithOptionsAndTags(int cid, uint32_t owner, CFArrayRef spaces, uint32_t options, uint64_t *set_tags, uint64_t *clear_tags);

CFStringRef space_display_uuid(uint64_t sid);
uint32_t space_display_id(uint64_t sid);
uint32_t *space_window_list_for_connection(uint64_t sid, int cid, int *count, bool include_minimized);
uint32_t *space_window_list(uint64_t sid, int *count, bool include_minimized);
CFStringRef space_uuid(uint64_t sid);
int space_type(uint64_t sid);
bool space_is_user(uint64_t sid);
bool space_is_system(uint64_t sid);
bool space_is_fullscreen(uint64_t sid);
bool space_is_visible(uint64_t sid);

#endif

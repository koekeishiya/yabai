extern int g_connection;

CFStringRef space_display_uuid(uint64_t sid)
{
    return SLSCopyManagedDisplayForSpace(g_connection, sid);
}

uint32_t space_display_id(uint64_t sid)
{
    CFStringRef uuid_string = space_display_uuid(sid);
    if (!uuid_string) return 0;

    CFUUIDRef uuid = CFUUIDCreateFromString(NULL, uuid_string);
    uint32_t id = CGDisplayGetDisplayIDFromUUID(uuid);

    CFRelease(uuid);
    CFRelease(uuid_string);

    return id;
}

uint32_t *space_window_list_for_connection(uint64_t *space_list, int space_count, int cid, int *count, bool include_minimized)
{
    uint32_t *window_list = NULL;
    uint64_t set_tags = 0;
    uint64_t clear_tags = 0;
    uint32_t options = include_minimized ? 0x7 : 0x2;

    CFArrayRef space_list_ref = cfarray_of_cfnumbers(space_list, sizeof(uint64_t), space_count, kCFNumberSInt64Type);
    CFArrayRef window_list_ref = SLSCopyWindowsWithOptionsAndTags(g_connection, cid, space_list_ref, options, &set_tags, &clear_tags);
    if (!window_list_ref) goto err;

    *count = CFArrayGetCount(window_list_ref);
    if (!*count) goto out;

    CFTypeRef query = SLSWindowQueryWindows(g_connection, window_list_ref, *count);
    CFTypeRef iterator = SLSWindowQueryResultCopyWindows(query);

    int window_count = 0;
    window_list = ts_alloc_aligned(sizeof(uint32_t), *count);

    while (SLSWindowIteratorAdvance(iterator)) {
        uint64_t tags = SLSWindowIteratorGetTags(iterator);
        uint64_t attributes = SLSWindowIteratorGetAttributes(iterator);
        uint32_t parent_wid = SLSWindowIteratorGetParentID(iterator);

        if ((parent_wid == 0) && (attributes == 0x2) && (((tags & 0x1)) || ((tags & 0x2) && (tags & 0x80000000)))) {
            window_list[window_count++] = SLSWindowIteratorGetWindowID(iterator);
        }
    }

    *count = window_count;
    CFRelease(query);
    CFRelease(iterator);
out:
    CFRelease(window_list_ref);
err:
    CFRelease(space_list_ref);
    return window_list;
}

uint32_t *space_window_list(uint64_t sid, int *count, bool include_minimized)
{
    return space_window_list_for_connection(&sid, 1, 0, count, include_minimized);
}

CFStringRef space_uuid(uint64_t sid)
{
    return SLSSpaceCopyName(g_connection, sid);
}

int space_type(uint64_t sid)
{
    return SLSSpaceGetType(g_connection, sid);
}

bool space_is_user(uint64_t sid)
{
    return space_type(sid) == 0;
}

bool space_is_fullscreen(uint64_t sid)
{
    return space_type(sid) == 4;
}

bool space_is_system(uint64_t sid)
{
    return space_type(sid) == 2;
}

bool space_is_visible(uint64_t sid)
{
    return sid == display_space_id(space_display_id(sid));
}

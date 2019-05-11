#ifndef HELPERS_H
#define HELPERS_H

extern AXError _AXUIElementGetWindow(AXUIElementRef ref, uint32_t *wid);

static inline bool is_root(void)
{
    return getuid() == 0 || geteuid() == 0;
}

static bool fork_exec_wait(char *command)
{
    static const char *shell = "/bin/bash";
    static const char *arg   = "-c";

    int pid = fork();
    if (pid == -1) {
        return false;
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        char *exec[] = { (char *) shell, (char *) arg, command, NULL};
        exit(execvp(exec[0], exec));
    }

    return true;
}

static bool ax_privilege(void)
{
    const void *keys[] = { kAXTrustedCheckOptionPrompt };
    const void *values[] = { kCFBooleanTrue };
    CFDictionaryRef options = CFDictionaryCreate(NULL, keys, values, array_count(keys),
                                                 &kCFCopyStringDictionaryKeyCallBacks,
                                                 &kCFTypeDictionaryValueCallBacks);
    bool result = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);
    return result;
}

static inline uint32_t ax_window_id(AXUIElementRef ref)
{
    uint32_t wid = 0;
    _AXUIElementGetWindow(ref, &wid);
    return wid;
}

static inline pid_t ax_window_pid(AXUIElementRef ref)
{
    return *(pid_t *)((void *) ref + 0x10);
}

static bool rect_is_in_direction(CGRect r1, CGRect r2, int direction)
{
    CGPoint r1_max = { CGRectGetMaxX(r1), CGRectGetMaxY(r1) };
    CGPoint r2_max = { CGRectGetMaxX(r2), CGRectGetMaxY(r2) };

    switch (direction) {
    case DIR_NORTH: if (r2.origin.y > r1_max.y) return false; break;
    case DIR_WEST:  if (r2.origin.x > r1_max.x) return false; break;
    case DIR_SOUTH: if (r2_max.y < r1.origin.y) return false; break;
    case DIR_EAST:  if (r2_max.x < r1.origin.x) return false; break;
    }

    switch (direction) {
    case DIR_NORTH:
    case DIR_SOUTH:
        return (r2.origin.x >= r1.origin.x && r2.origin.x <= r1_max.x) ||
               (r2_max.x >= r1.origin.x && r2_max.x <= r1_max.x) ||
               (r1.origin.x > r2.origin.x && r1.origin.x < r2_max.x);
    case DIR_WEST:
    case DIR_EAST:
        return (r2.origin.y >= r1.origin.y && r2.origin.y <= r1_max.y) ||
               (r2_max.y >= r1.origin.y && r2_max.y <= r1_max.y) ||
               (r1.origin.y > r2.origin.y && r1_max.y < r2_max.y);
    }

    return false;
}

static uint32_t rect_distance(CGRect r1, CGRect r2, int direction)
{
    CGPoint r1_max = { CGRectGetMaxX(r1), CGRectGetMaxY(r1) };
    CGPoint r2_max = { CGRectGetMaxX(r2), CGRectGetMaxY(r2) };

    switch (direction) {
    case DIR_NORTH: return r2_max.y > r1.origin.y ? r2_max.y - r1.origin.y : r1.origin.y - r2_max.y;
    case DIR_WEST:  return r2_max.x > r1.origin.x ? r2_max.x - r1.origin.x : r1.origin.x - r2_max.x;
    case DIR_SOUTH: return r2.origin.y < r1_max.y ? r1_max.y - r2.origin.y : r2.origin.y - r1_max.y;
    case DIR_EAST:  return r2.origin.x < r1_max.x ? r1_max.x - r2.origin.x : r2.origin.x - r1_max.x;
    }

    return UINT32_MAX;
}

#endif

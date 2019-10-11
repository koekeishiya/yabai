#ifndef HELPERS_H
#define HELPERS_H

extern AXError _AXUIElementGetWindow(AXUIElementRef ref, uint32_t *wid);

static const char *bool_str[] = { "off", "on" };

struct signal_args
{
    char name[2][255];
    char value[2][255];
    void *entity;
};

struct rgba_color
{
    bool is_valid;
    uint32_t p;
    float r;
    float g;
    float b;
    float a;
};

static struct rgba_color
rgba_color_from_hex(uint32_t color)
{
    struct rgba_color result;
    result.is_valid = true;
    result.p = color;
    result.r = ((color >> 16) & 0xff) / 255.0;
    result.g = ((color >> 8) & 0xff) / 255.0;
    result.b = ((color >> 0) & 0xff) / 255.0;
    result.a = ((color >> 24) & 0xff) / 255.0;
    return result;
}

static struct rgba_color
rgba_color_dim(struct rgba_color color)
{
    struct rgba_color result;
    result.is_valid = true;
    result.p = color.p;
    result.r = 0.25f*color.r;
    result.g = 0.25f*color.g;
    result.b = 0.25f*color.b;
    result.a = 0.25f*color.a;
    return result;
}

static inline bool is_root(void)
{
    return getuid() == 0 || geteuid() == 0;
}

static inline bool string_equals(const char *a, const char *b)
{
    return a && b && strcmp(a, b) == 0;
}

static inline char *string_escape_quote(char *s)
{
    if (!s) return NULL;

    bool found_quote = false;
    for (char *cursor = s; *cursor; ++cursor) {
        if (*cursor == '"') {
            found_quote = true;
            break;
        }
    }

    if (!found_quote) return NULL;

    int size = sizeof(char) * (2*strlen(s));
    char *result = malloc(size);
    char *dst = result;
    memset(result, 0, size);

    for (char *cursor = s; *cursor; ++cursor) {
        if (*cursor == '"') *dst++ = '\\';
        *dst++ = *cursor;
    }

    return result;
}

static CFArrayRef cfarray_of_cfnumbers(void *values, size_t size, int count, CFNumberType type)
{
    CFNumberRef *temp = malloc(sizeof(CFNumberRef) * count);

    for (int i = 0; i < count; ++i) {
        temp[i] = CFNumberCreate(NULL, type, ((char *)values) + (size * i));
    }

    CFArrayRef result = CFArrayCreate(NULL, (const void **)temp, count, &kCFTypeArrayCallBacks);

    for (int i = 0; i < count; ++i) {
        CFRelease(temp[i]);
    }

    free(temp);
    return result;
}

static inline char *cfstring_copy(CFStringRef string)
{
    CFIndex num_bytes = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string), kCFStringEncodingUTF8);
    char *result = malloc(num_bytes + 1);
    if (!result) return NULL;

    if (!CFStringGetCString(string, result, num_bytes + 1, kCFStringEncodingUTF8)) {
        free(result);
        result = NULL;
    }

    return result;
}

static inline char *string_copy(char *s)
{
    int length = strlen(s);
    char *result = malloc(length + 1);
    if (!result) return NULL;

    memcpy(result, s, length);
    result[length] = '\0';
    return result;
}

static inline bool file_exists(char *filename)
{
    struct stat buffer;

    if (stat(filename, &buffer) != 0) {
        return false;
    }

    if (buffer.st_mode & S_IFDIR) {
        return false;
    }

    return true;
}

static inline bool ensure_executable_permission(char *filename)
{
    struct stat buffer;

    if (stat(filename, &buffer) != 0) {
        return false;
    }

    bool is_executable = buffer.st_mode & S_IXUSR;
    if (!is_executable && chmod(filename, S_IXUSR | buffer.st_mode) != 0) {
        return false;
    }

    return true;
}

static bool fork_exec(char *command, struct signal_args *args)
{
    int pid = fork();
    if (pid == -1) return false;
    if (pid !=  0) return true;

    if (args) {
        if (*args->name[0]) setenv(args->name[0], args->value[0], 1);
        if (*args->name[1]) setenv(args->name[1], args->value[1], 1);
    }

    char *exec[] = { "/usr/bin/env", "sh", "-c", command, NULL};
    exit(execvp(exec[0], exec));
}

static bool ax_privilege(void)
{
    const void *keys[] = { kAXTrustedCheckOptionPrompt };
    const void *values[] = { kCFBooleanTrue };
    CFDictionaryRef options = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline bool psn_equals(ProcessSerialNumber *a, ProcessSerialNumber *b)
{
    Boolean result;
    SameProcess(a, b, &result);
    return result == 1;
}
#pragma clang diagnostic pop

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

static bool triangle_contains_point(CGPoint t[3], CGPoint p)
{
    float l1 = (p.x - t[0].x) * (t[2].y - t[0].y) - (t[2].x - t[0].x) * (p.y - t[0].y);
    float l2 = (p.x - t[1].x) * (t[0].y - t[1].y) - (t[0].x - t[1].x) * (p.y - t[1].y);
    float l3 = (p.x - t[2].x) * (t[1].y - t[2].y) - (t[1].x - t[2].x) * (p.y - t[2].y);

    return (l1 > 0.0f && l2 > 0.0f && l3 > 0.0f) || (l1 < 0.0f && l2 < 0.0f && l3 < 0.0f);
}

static int regex_match(bool valid, regex_t *regex, const char *match)
{
    if (!valid || !match) return REGEX_MATCH_UD;
    int result = regexec(regex, match, 0, NULL, 0);
    return result == 0 ? REGEX_MATCH_YES : REGEX_MATCH_NO;
}

#endif

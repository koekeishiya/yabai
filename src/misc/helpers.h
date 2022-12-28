#ifndef HELPERS_H
#define HELPERS_H

struct rgba_color
{
    uint32_t p;
    float r;
    float g;
    float b;
    float a;
};

static const CFStringRef kAXEnhancedUserInterface = CFSTR("AXEnhancedUserInterface");

static const char *bool_str[] = { "off", "on" };

static const char *layer_str[] =
{
    [0] = "",
    [LAYER_BELOW] = "below",
    [LAYER_NORMAL] = "normal",
    [LAYER_ABOVE] = "above"
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline uint64_t get_wall_clock(void)
{
    uint64_t absolute = mach_absolute_time();
    Nanoseconds result = AbsoluteToNanoseconds(*(AbsoluteTime *) &absolute);
    return *(uint64_t *) &result;
}
#pragma clang diagnostic pop

static inline float get_seconds_elapsed(uint64_t start, uint64_t end)
{
    float result = ((float)(end - start) / 1000.0f) / 1000000.0f;
    return result;
}

static inline float ease_out_cubic(float t)
{
    return 1.0f - powf(1.0f - t, 3.0f);
}

#define ANIMATE_DELAY(current_frame_duration)                                                    \
    while (frame_elapsed < current_frame_duration) {                                             \
        uint32_t sleep_ms = (uint32_t)(1000.0f * (current_frame_duration - frame_elapsed));      \
        usleep(sleep_ms * 700.0f);                                                               \
        frame_elapsed = get_seconds_elapsed(last_counter, get_wall_clock());                     \
    }

#define ANIMATE(connection, frame_rate, duration, easing_function, code_block)                   \
{                                                                                                \
    float frame_duration = 1.0f / (float)frame_rate;                                             \
    int frame_count = (int)((duration / frame_duration) + 1.0f);                                 \
    uint64_t last_counter = get_wall_clock();                                                    \
                                                                                                 \
    for (int frame_index = 1; frame_index <= frame_count; ++frame_index) {                       \
        float t = (float) frame_index / (float) frame_count;                                     \
        if (t < 0.0f) t = 0.0f;                                                                  \
        if (t > 1.0f) t = 1.0f;                                                                  \
                                                                                                 \
        float mt = easing_function(t);                                                           \
        CFTypeRef transaction = SLSTransactionCreate(connection);                                \
                                                                                                 \
        code_block                                                                               \
                                                                                                 \
        SLSTransactionCommit(transaction, 0);                                                    \
        CFRelease(transaction);                                                                  \
                                                                                                 \
        float frame_elapsed = get_seconds_elapsed(last_counter, get_wall_clock());               \
        if (frame_elapsed < frame_duration) {                                                    \
            ANIMATE_DELAY(frame_duration);                                                       \
        } else {                                                                                 \
            int frame_skip = (int)((frame_elapsed / frame_duration) + 0.5f);                     \
            frame_index += frame_skip;                                                           \
            ANIMATE_DELAY(frame_duration * frame_skip);                                          \
        }                                                                                        \
                                                                                                 \
        last_counter = get_wall_clock();                                                         \
    }                                                                                            \
}

static inline bool socket_open(int *sockfd)
{
    *sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    return *sockfd != -1;
}

static inline bool socket_connect(int sockfd, char *socket_path)
{
    struct sockaddr_un socket_address;
    socket_address.sun_family = AF_UNIX;

    snprintf(socket_address.sun_path, sizeof(socket_address.sun_path), "%s", socket_path);
    return connect(sockfd, (struct sockaddr *) &socket_address, sizeof(socket_address)) != -1;
}

static inline void socket_close(int sockfd)
{
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

static inline char *json_optional_bool(int value)
{
    if (value == 0) return "null";
    if (value == 1) return "true";

    return "false";
}

static inline char *json_bool(bool value)
{
    return value ? "true" : "false";
}

static inline struct rgba_color rgba_color_from_hex(uint32_t color)
{
    struct rgba_color result;
    result.p = color;
    result.r = ((color >> 0x10) & 0xff) / 255.0f;
    result.g = ((color >> 0x08) & 0xff) / 255.0f;
    result.b = ((color >> 0x00) & 0xff) / 255.0f;
    result.a = ((color >> 0x18) & 0xff) / 255.0f;
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

static inline char *_string_escape_pre(char *s, int *size_in_bytes)
{
    char *cursor = s;
    int num_replacements = 0;

    while (*cursor) {
        if ((*cursor == '"') ||
            (*cursor == '\\') ||
            (*cursor == '\b') ||
            (*cursor == '\f') ||
            (*cursor == '\n') ||
            (*cursor == '\r') ||
            (*cursor == '\t')) {
            ++num_replacements;
        }

        ++cursor;
    }

    *size_in_bytes = (int)(cursor - s) + num_replacements;
    return num_replacements > 0 ? cursor : NULL;
}

static inline void _string_escape_post(char *s, char *cursor, char *result)
{
    for (char *dst = result, *cursor = s; *cursor; ++cursor) {
        if (*cursor == '"') {
            *dst++ = '\\';
            *dst++ = *cursor;
        } else if (*cursor == '\\') {
            *dst++ = '\\';
            *dst++ = '\\';
        } else if (*cursor == '\b') {
            *dst++ = '\\';
            *dst++ = 'b';
        } else if (*cursor == '\f') {
            *dst++ = '\\';
            *dst++ = 'f';
        } else if (*cursor == '\n') {
            *dst++ = '\\';
            *dst++ = 'n';
        } else if (*cursor == '\r') {
            *dst++ = '\\';
            *dst++ = 'r';
        } else if (*cursor == '\t') {
            *dst++ = '\\';
            *dst++ = 't';
        } else {
            *dst++ = *cursor;
        }
    }
}

static inline char *ts_string_escape(char *s)
{
    int size_in_bytes;
    char *cursor = _string_escape_pre(s, &size_in_bytes);
    if (!cursor) return NULL;

    char *result = ts_alloc_unaligned(sizeof(char) * (size_in_bytes+1));
    result[size_in_bytes] = '\0';
    _string_escape_post(s, cursor, result);

    return result;
}

static inline CFNumberRef CFNUM32(int32_t num)
{
    return CFNumberCreate(NULL, kCFNumberSInt32Type, &num);
}

static inline void sls_window_disable_shadow(uint32_t id)
{
    CFNumberRef density = CFNUM32(0);
    const void *keys[1] = { CFSTR("com.apple.WindowShadowDensity") };
    const void *values[1] = { density };
    CFDictionaryRef options = CFDictionaryCreate(NULL, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    SLSWindowSetShadowProperties(id, options);
    CFRelease(density);
    CFRelease(options);
}

static inline CFArrayRef cfarray_of_cfnumbers(void *values, size_t size, int count, CFNumberType type)
{
    CFNumberRef temp[count];

    for (int i = 0; i < count; ++i) {
        temp[i] = CFNumberCreate(NULL, type, ((char *)values) + (size * i));
    }

    CFArrayRef result = CFArrayCreate(NULL, (const void **)temp, count, &kCFTypeArrayCallBacks);

    for (int i = 0; i < count; ++i) {
        CFRelease(temp[i]);
    }

    return result;
}

static inline char *ts_cfstring_copy(CFStringRef string)
{
    CFIndex num_bytes = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string), kCFStringEncodingUTF8);
    char *result = ts_alloc_unaligned(num_bytes + 1);

    if (!CFStringGetCString(string, result, num_bytes + 1, kCFStringEncodingUTF8)) {
        result = NULL;
    }

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

static inline char *ts_string_copy(char *s)
{
    int length = strlen(s);
    char *result = ts_alloc_unaligned(length + 1);

    memcpy(result, s, length);
    result[length] = '\0';
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

static inline bool ax_privilege(void)
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

static inline bool ax_enhanced_userinterface(AXUIElementRef ref)
{
    Boolean result = 0;
    CFTypeRef value;

    if (AXUIElementCopyAttributeValue(ref, kAXEnhancedUserInterface, &value) == kAXErrorSuccess) {
        result = CFBooleanGetValue(value);
        CFRelease(value);
    }

    return result;
}

#define AX_ENHANCED_UI_WORKAROUND(r, c) \
{\
    bool eui = ax_enhanced_userinterface(r); \
    if (eui) AXUIElementSetAttributeValue(r, kAXEnhancedUserInterface, kCFBooleanFalse); \
    c \
    if (eui) AXUIElementSetAttributeValue(r, kAXEnhancedUserInterface, kCFBooleanTrue); \
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

static inline float cgrect_clamp_x_radius(CGRect frame, float radius)
{
    if (radius * 2 > CGRectGetWidth(frame)) {
        radius = CGRectGetWidth(frame) / 2;
    }
    return radius;
}

static inline float cgrect_clamp_y_radius(CGRect frame, float radius)
{
    if (radius * 2 > CGRectGetHeight(frame)) {
        radius = CGRectGetHeight(frame) / 2;
    }
    return radius;
}

static inline bool cgrect_contains_point(CGRect r, CGPoint p)
{
    return p.x >= r.origin.x && p.x <= r.origin.x + r.size.width &&
           p.y >= r.origin.y && p.y <= r.origin.y + r.size.height;
}

static inline bool triangle_contains_point(CGPoint t[3], CGPoint p)
{
    float l1 = (p.x - t[0].x) * (t[2].y - t[0].y) - (t[2].x - t[0].x) * (p.y - t[0].y);
    float l2 = (p.x - t[1].x) * (t[0].y - t[1].y) - (t[0].x - t[1].x) * (p.y - t[1].y);
    float l3 = (p.x - t[2].x) * (t[1].y - t[2].y) - (t[1].x - t[2].x) * (p.y - t[2].y);

    return (l1 > 0.0f && l2 > 0.0f && l3 > 0.0f) || (l1 < 0.0f && l2 < 0.0f && l3 < 0.0f);
}

static inline int regex_match(bool valid, regex_t *regex, const char *match)
{
    if (!valid) return REGEX_MATCH_UD;

    int result = regexec(regex, match, 0, NULL, 0);
    return result == 0 ? REGEX_MATCH_YES : REGEX_MATCH_NO;
}

static inline float clampf_range(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

#endif

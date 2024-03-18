#ifndef HELPERS_H
#define HELPERS_H

#define ANIMATION_EASING_TYPE_LIST \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_sine) \
    ANIMATION_EASING_TYPE_ENTRY(ease_out_sine) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_out_sine) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_quad) \
    ANIMATION_EASING_TYPE_ENTRY(ease_out_quad) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_out_quad) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_cubic) \
    ANIMATION_EASING_TYPE_ENTRY(ease_out_cubic) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_out_cubic) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_quart) \
    ANIMATION_EASING_TYPE_ENTRY(ease_out_quart) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_out_quart) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_quint) \
    ANIMATION_EASING_TYPE_ENTRY(ease_out_quint) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_out_quint) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_expo) \
    ANIMATION_EASING_TYPE_ENTRY(ease_out_expo) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_out_expo) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_circ) \
    ANIMATION_EASING_TYPE_ENTRY(ease_out_circ) \
    ANIMATION_EASING_TYPE_ENTRY(ease_in_out_circ)

enum animation_easing_type
{
#define ANIMATION_EASING_TYPE_ENTRY(value) value##_type,
    ANIMATION_EASING_TYPE_LIST
#undef ANIMATION_EASING_TYPE_ENTRY
    EASING_TYPE_COUNT
};

static char *animation_easing_type_str[] =
{
#define ANIMATION_EASING_TYPE_ENTRY(value) [value##_type] = #value,
    ANIMATION_EASING_TYPE_LIST
#undef ANIMATION_EASING_TYPE_ENTRY
};

static inline float ease_in_sine(float t)
{
    return 1.0f - cosf((t * M_PI) / 2.0f);
}

static inline float ease_out_sine(float t)
{
    return sinf((t * M_PI) / 2.0f);
}

static inline float ease_in_out_sine(float t)
{
    return -(cosf(M_PI * t) - 1.0f) / 2.0f;
}

static inline float ease_in_quad(float t)
{
    return t * t;
}

static inline float ease_out_quad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

static inline float ease_in_out_quad(float t)
{
    return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

static inline float ease_in_cubic(float t)
{
    return t * t * t;
}

static inline float ease_out_cubic(float t)
{
    return 1.0f - powf(1.0f - t, 3);
}

static inline float ease_in_out_cubic(float t)
{
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

static inline float ease_in_quart(float t)
{
    return t * t * t * t;
}

static inline float ease_out_quart(float t)
{
    return 1.0f - powf(1.0f - t, 4);
}

static inline float ease_in_out_quart(float t)
{
    return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 4.0f) / 2.0f;
}

static inline float ease_in_quint(float t)
{
    return t * t * t * t * t;
}

static inline float ease_out_quint(float t)
{
    return 1.0f - powf(1.0f - t, 5);
}

static inline float ease_in_out_quint(float t)
{
    return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 5.0f) / 2.0f;
}

static inline float ease_in_expo(float t)
{
    return t == 0.0f ? 0.0f : powf(2.0f, 10.0f * t - 10.0f);
}

static inline float ease_out_expo(float t)
{
    return t == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
}

static inline float ease_in_out_expo(float t)
{
    return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : t < 0.5f ? powf(2.0f, 20.0f * t - 10.0f) / 2.0f : (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
}

static inline float ease_in_circ(float t)
{
    return 1.0f - sqrtf(1.0f - powf(t, 2.0f));
}

static inline float ease_out_circ(float t)
{
    return sqrtf(1.0f - powf(t - 1.0f, 2.0f));
}

static inline float ease_in_out_circ(float t)
{
    return t < 0.5f ? (1.0f - sqrtf(1.0f - powf(2.0f * t, 2.0f))) / 2.0f : (sqrtf(1.0f - powf(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline uint64_t read_os_timer(void)
{
    uint64_t result = mach_absolute_time();
    Nanoseconds nano = AbsoluteToNanoseconds(*(AbsoluteTime *) &result);
    return *(uint64_t *) &nano;
}
#pragma clang diagnostic pop

static inline uint64_t read_os_freq(void)
{
    return 1000000000;
}

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
    [LAYER_AUTO]   = "auto",
    [LAYER_BELOW]  = "below",
    [LAYER_NORMAL] = "normal",
    [LAYER_ABOVE]  = "above"
};

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

static inline void mach_send(mach_port_t port, void *data, uint32_t size)
{
    struct {
        mach_msg_header_t header;
        mach_msg_size_t descriptor_count;
        mach_msg_ool_descriptor_t descriptor;
    } msg = {0};

    msg.header.msgh_bits        = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND & MACH_MSGH_BITS_REMOTE_MASK, 0, 0, MACH_MSGH_BITS_COMPLEX);
    msg.header.msgh_size        = sizeof(msg);
    msg.header.msgh_remote_port = port;
    msg.descriptor_count        = 1;
    msg.descriptor.address      = data;
    msg.descriptor.size         = size;
    msg.descriptor.copy         = MACH_MSG_VIRTUAL_COPY;
    msg.descriptor.deallocate   = false;
    msg.descriptor.type         = MACH_MSG_OOL_DESCRIPTOR;

    mach_msg(&msg.header, MACH_SEND_MSG, sizeof(msg), 0, 0, 0, 0);
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

static inline CFStringRef CFSTRINGNUM32(int32_t num)
{
    char num_str[255] = {0};
    snprintf(num_str, sizeof(num_str), "%d", num);
    return CFStringCreateWithCString(NULL, num_str, kCFStringEncodingMacRoman);
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

static inline bool directory_exists(char *filename)
{
    struct stat buffer;

    if (stat(filename, &buffer) != 0) {
        return false;
    }

    return S_ISDIR(buffer.st_mode);
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

static inline bool file_can_execute(char *filename)
{
    struct stat buffer;

    if (stat(filename, &buffer) != 0) {
        return false;
    }

    return (buffer.st_mode & S_IXUSR);
}

static bool get_config_file(char *restrict filename, char *restrict buffer, int buffer_size)
{
    char *xdg_home = getenv("XDG_CONFIG_HOME");
    if (xdg_home && *xdg_home) {
        snprintf(buffer, buffer_size, "%s/yabai/%s", xdg_home, filename);
        if (file_exists(buffer)) return true;
    }

    char *home = getenv("HOME");
    if (!home) return false;

    snprintf(buffer, buffer_size, "%s/.config/yabai/%s", home, filename);
    if (file_exists(buffer)) return true;

    snprintf(buffer, buffer_size, "%s/.%s", home, filename);
    return file_exists(buffer);
}

static void exec_config_file(char *config_file, int config_file_size)
{
    if (config_file[0] == '\0' && !get_config_file("yabairc", config_file, config_file_size)) {
        warn("yabai: could not locate config file..\n");
        notify("configuration", "could not locate config file..");
        return;
    }

    if (!file_exists(config_file)) {
        warn("yabai: configuration file '%s' does not exist..\n", config_file);
        notify("configuration", "file '%s' does not exist..", config_file);
        return;
    }

    int pid = fork();
    if (pid == 0) {
        char **exec = file_can_execute(config_file)
                    ? (char*[]){ "/usr/bin/env", "sh", "-c", config_file, NULL}
                    : (char*[]){ "/usr/bin/env", "sh", config_file, NULL};
        exit(execvp(exec[0], exec));
    } else if (pid == -1) {
        warn("yabai: failed to load config file '%s'\n", config_file);
        notify("configuration", "failed to load file '%s'", config_file);
    }
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

static CGImageRef cgimage_restore_alpha(CGImageRef image)
{
    int width     = CGImageGetWidth(image);
    int height    = CGImageGetHeight(image);
    int pitch     = width * 4;
    uint8_t *data = (uint8_t *) calloc(height * pitch, 1);

    CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(data, width, height, 8, pitch, color_space, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(color_space);
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);

#ifdef __x86_64__
    __m128      inv255 = _mm_set1_ps(1.0f / 255.0f);
    __m128      one255 = _mm_set1_ps(255.0f);
    __m128        zero = _mm_set1_ps(0.0f);
    __m128i    mask_ff = _mm_set1_epi32(0xff);
#elif __arm64__
    float32x4_t inv255 = vdupq_n_f32(1.0f / 255.0f);
    float32x4_t one255 = vdupq_n_f32(255.0f);
    float32x4_t   zero = vdupq_n_f32(0.0f);
    int32x4_t  mask_ff = vdupq_n_s32(0xff);
#endif

    uint32_t *pixel = (uint32_t *) data;
    for (int i = 0; i < height*width; i += 4) {
#ifdef __x86_64__
        __m128i source = _mm_loadu_si128((__m128i *) pixel);
        __m128 r = _mm_cvtepi32_ps(_mm_and_si128(source, mask_ff));
        __m128 g = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(source,  8), mask_ff));
        __m128 b = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(source, 16), mask_ff));
        __m128 a = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(source, 24), mask_ff));
        __m128i mask = _mm_castps_si128(_mm_cmpgt_ps(a, zero));

        r = _mm_mul_ps(one255, _mm_div_ps(r, a));
        g = _mm_mul_ps(one255, _mm_div_ps(g, a));
        b = _mm_mul_ps(one255, _mm_div_ps(b, a));

        a = one255;

        r = _mm_mul_ps(inv255, _mm_mul_ps(r, a));
        g = _mm_mul_ps(inv255, _mm_mul_ps(g, a));
        b = _mm_mul_ps(inv255, _mm_mul_ps(b, a));

        __m128i sr = _mm_cvtps_epi32(r);
        __m128i sg = _mm_slli_epi32(_mm_cvtps_epi32(g),  8);
        __m128i sb = _mm_slli_epi32(_mm_cvtps_epi32(b), 16);
        __m128i sa = _mm_slli_epi32(_mm_cvtps_epi32(a), 24);

        __m128i color = _mm_or_si128(_mm_or_si128(_mm_or_si128(sr, sg), sb), sa);
        __m128i masked_color = _mm_or_si128(_mm_and_si128(mask, color), _mm_andnot_si128(mask, source));
        _mm_storeu_si128((__m128i *) pixel, masked_color);
#elif __arm64__
        int32x4_t source = vld1q_s32((int32_t *) pixel);
        float32x4_t r = vcvtq_f32_s32(vandq_s32(source, mask_ff));
        float32x4_t g = vcvtq_f32_s32(vandq_s32(vshlq_u32(source, vdupq_n_s32(-8)),  mask_ff));
        float32x4_t b = vcvtq_f32_s32(vandq_s32(vshlq_u32(source, vdupq_n_s32(-16)), mask_ff));
        float32x4_t a = vcvtq_f32_s32(vandq_s32(vshlq_u32(source, vdupq_n_s32(-24)), mask_ff));
        int32x4_t mask = vreinterpretq_s32_f32(vcgtq_f32(a, zero));

        r = vmulq_f32(one255, vdivq_f32(r, a));
        g = vmulq_f32(one255, vdivq_f32(g, a));
        b = vmulq_f32(one255, vdivq_f32(b, a));

        a = one255;

        r = vmulq_f32(inv255, vmulq_f32(r, a));
        g = vmulq_f32(inv255, vmulq_f32(g, a));
        b = vmulq_f32(inv255, vmulq_f32(b, a));

        int32x4_t sr = vcvtnq_s32_f32(r);
        int32x4_t sg = vshlq_s32(vcvtnq_s32_f32(g), vdupq_n_s32(8));
        int32x4_t sb = vshlq_s32(vcvtnq_s32_f32(b), vdupq_n_s32(16));
        int32x4_t sa = vshlq_s32(vcvtnq_s32_f32(a), vdupq_n_s32(24));

        int32x4_t color = vorrq_s32(vorrq_s32(vorrq_s32(sr, sg), sb), sa);
        int32x4_t masked_color = vorrq_s32(vandq_s32(color, mask), vbicq_s32(source, mask));
        vst1q_s32((int32_t *) pixel, masked_color);
#endif
        pixel += 4;
    }

    CGImageRef result = CGBitmapContextCreateImage(context);
    CGContextRelease(context);

    free(data);
    return result;
}

#endif

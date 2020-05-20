#ifndef LOG_H
#define LOG_H

extern bool g_verbose;

static inline void
debug(const char *format, ...)
{
    if (!g_verbose) return;

    uint32_t thread_id;
    __asm__ __volatile__ ("mov %%gs:0x00, %0" : "=r"(thread_id));
    fprintf(stdout, "thread: %d | ", thread_id);

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

static inline void
warn(const char *format, ...)
{
    uint32_t thread_id;
    __asm__ __volatile__ ("mov %%gs:0x00, %0" : "=r"(thread_id));
    fprintf(stderr, "thread: %d | ", thread_id);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

static inline void
error(const char *format, ...)
{
    uint32_t thread_id;
    __asm__ __volatile__ ("mov %%gs:0x00, %0" : "=r"(thread_id));
    fprintf(stderr, "thread: %d | ", thread_id);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

static inline void
debug_message(const char *prefix, char *message)
{
    if (!g_verbose) return;

    uint32_t thread_id;
    __asm__ __volatile__ ("mov %%gs:0x00, %0" : "=r"(thread_id));
    fprintf(stdout, "thread: %d | %s:", thread_id, prefix);

    for (;*message;) {
        message += fprintf(stdout, " %s", message);
    }

    putc('\n', stdout);
    fflush(stdout);
}

#endif

#ifndef LOG_H
#define LOG_H

extern bool g_verbose;

static inline void
debug(const char *format, ...)
{
    if (!g_verbose) return;

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

static inline void
warn(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

static inline void
error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

static inline void
require(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(EXIT_SUCCESS);
}

static inline void
debug_message(const char *prefix, char *message)
{
    if (!g_verbose) return;

    fprintf(stdout, "%s:", prefix);

    for (;*message;) {
        message += fprintf(stdout, " %s", message);
    }

    putc('\n', stdout);
    fflush(stdout);
}

#endif

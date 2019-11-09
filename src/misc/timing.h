#include <CoreServices/CoreServices.h>
#include <mach/mach_time.h>

static inline uint64_t time_clock(void)
{
    return mach_absolute_time();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline uint64_t time_elapsed_ns(uint64_t begin, uint64_t end)
{
    uint64_t elapsed = end - begin;
    Nanoseconds nano = AbsoluteToNanoseconds(*(AbsoluteTime *) &elapsed);
    return *(uint64_t *) &nano;
}
#pragma clang diagnostic pop

static inline double time_elapsed_ms(uint64_t begin, uint64_t end)
{
    uint64_t ns = time_elapsed_ns(begin, end);
    return (double)(ns / 1000000.0);
}

static inline double time_elapsed_s(uint64_t begin, uint64_t end)
{
    uint64_t ns = time_elapsed_ns(begin, end);
    return (double)(ns / 1000000000.0);
}

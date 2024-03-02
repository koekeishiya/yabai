#ifndef TIMER_H
#define TIMER_H

#ifdef PROFILE
#include <mach/mach_time.h>
#include <stdint.h>
#include <stdio.h>

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

static inline uint64_t read_cpu_timer(void)
{
#ifdef __x86_64__
    return __rdtsc();
#elif __arm64__
    uint64_t value;
    __asm__ __volatile__ ("mrs %0, cntvct_el0" : "=r" (value));
    return value;
#endif
}

static inline uint64_t read_cpu_freq(void)
{
#ifdef __x86_64__
    uint64_t ms_to_wait   = 100;
    uint64_t os_freq      = read_os_freq();
    uint64_t cpu_start    = read_cpu_timer();
    uint64_t os_start     = read_os_timer();
    uint64_t os_end       = 0;
    uint64_t os_elapsed   = 0;
    uint64_t os_wait_time = os_freq * ms_to_wait / 1000;

    while (os_elapsed < os_wait_time) {
        os_end     = read_os_timer();
        os_elapsed = os_end - os_start;
    }

    uint64_t cpu_end     = read_cpu_timer();
    uint64_t cpu_elapsed = cpu_end - cpu_start;

    return os_freq * cpu_elapsed / os_elapsed;
#elif __arm64__
    uint64_t value;
    __asm__ __volatile__ ("mrs %0, cntfrq_el0" : "=r" (value));
    return value;
#endif
}

struct time_block
{
    const char *label;
    uint64_t begin_tb;
};

void END_TIME_BLOCK(void *context)
{
    struct time_block *tb = context;
    uint64_t dt_tb = read_cpu_timer() - tb->begin_tb;\
    printf("%s: %0.4fms\n", tb->label, 1000.0 * (double)dt_tb / (double)read_cpu_freq());\
}

#define TIME_FUNCTION \
    __attribute((cleanup(END_TIME_BLOCK))) struct time_block tb_##__FUNCTION__;\
    tb_##__FUNCTION__ = (struct time_block) {__FUNCTION__, read_cpu_timer()}

#define TIME_BLOCK(label, c) \
do {\
    __attribute((cleanup(END_TIME_BLOCK))) struct time_block tb_##label;\
    tb_##label = (struct time_block) {#label, read_cpu_timer()};\
    c \
} while (0)
#else
#define TIME_FUNCTION
#define TIME_BLOCK(label, c) c
#endif

#endif

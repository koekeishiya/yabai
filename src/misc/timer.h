#ifndef TIMER_H
#define TIMER_H

#if PROFILE >= 1
#include <stdint.h>
#include <stdio.h>

struct profile_anchor
{
    uint64_t tsc_elapsed_exclusive;
    uint64_t tsc_elapsed_inclusive;
    uint64_t hit_count;
    char const *label;
};

static struct
{
    uint64_t begin_tsc;
    uint64_t end_tsc;
    struct profile_anchor anchors[4096];
    uint32_t parent;
} g_profiler;

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
    static uint64_t cpu_freq;
    if (cpu_freq == 0) {
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

        cpu_freq = os_freq * cpu_elapsed / os_elapsed;
    }
    return cpu_freq;
#elif __arm64__
    uint64_t value;
    __asm__ __volatile__ ("mrs %0, cntfrq_el0" : "=r" (value));
    return value;
#endif
}

static void profile_begin(void)
{
    memset(&g_profiler, 0, sizeof(g_profiler));
    g_profiler.begin_tsc = read_cpu_timer();
}

static void profile_end_and_print(void)
{
    g_profiler.end_tsc = read_cpu_timer();
    uint64_t timer_freq = read_cpu_freq();

    uint64_t total_tsc_elapsed = g_profiler.end_tsc - g_profiler.begin_tsc;
    printf("Total time: %0.4fms (timer freq %llu)\n", 1000.0 * (double)total_tsc_elapsed / (double)timer_freq, timer_freq);

    for (int i = 0; i < array_count(g_profiler.anchors); ++i) {
        struct profile_anchor *anchor = g_profiler.anchors + i;
        if (anchor->tsc_elapsed_inclusive) {
            double percent = 100.0 * ((double)anchor->tsc_elapsed_exclusive / (double)total_tsc_elapsed);
            printf("    %s[%llu]: %llu (%.2f%%", anchor->label, anchor->hit_count, anchor->tsc_elapsed_exclusive, percent);
            if (anchor->tsc_elapsed_inclusive != anchor->tsc_elapsed_exclusive) {
                double percent_with_children = 100.0 * ((double)anchor->tsc_elapsed_inclusive / (double)total_tsc_elapsed);
                printf(", %.2f%% w/children", percent_with_children);
            }
            printf(")\n");
        }
    }
}

#if PROFILE >= 2
struct time_block
{
    char const *label;
    uint64_t old_tsc_elapsed_inclusive;
    uint64_t begin_tsc;
    uint32_t parent_index;
    uint32_t anchor_index;
};

static void BEGIN_TIME_BLOCK(struct time_block *tb, const char *label, uint32_t anchor_index)
{
    tb->parent_index = g_profiler.parent;

    tb->anchor_index = anchor_index;
    tb->label = label;

    struct profile_anchor *anchor = g_profiler.anchors + anchor_index;
    tb->old_tsc_elapsed_inclusive = anchor->tsc_elapsed_inclusive;

    g_profiler.parent = anchor_index;
    tb->begin_tsc = read_cpu_timer();
}

static void END_TIME_BLOCK(void *context)
{
    struct time_block *tb = context;

    uint64_t elapsed = read_cpu_timer() - tb->begin_tsc;
    g_profiler.parent = tb->parent_index;

    struct profile_anchor *parent = g_profiler.anchors + tb->parent_index;
    struct profile_anchor *anchor = g_profiler.anchors + tb->anchor_index;

    parent->tsc_elapsed_exclusive -= elapsed;
    anchor->tsc_elapsed_exclusive += elapsed;
    anchor->tsc_elapsed_inclusive = tb->old_tsc_elapsed_inclusive + elapsed;
    ++anchor->hit_count;

    anchor->label = tb->label;
}

#define TIME_FUNCTION \
    __attribute((cleanup(END_TIME_BLOCK))) struct time_block tb_##__FUNCTION__;\
    BEGIN_TIME_BLOCK(&tb_##__FUNCTION__, __FUNCTION__, __COUNTER__ + 1)

#define TIME_BLOCK(label) \
    __attribute((cleanup(END_TIME_BLOCK))) struct time_block tb_##label;\
    BEGIN_TIME_BLOCK(&tb_##label, #label, __COUNTER__ + 1)

#define TIME_BODY(label, c) \
do {\
    TIME_BLOCK(label);\
    c \
} while (0)
#define PROFILER_END_TRANSLATION_UNIT _Static_assert(__COUNTER__ < array_count(g_profiler.anchors), "Number of profile points exceeds size of profiler::anchors array!")
#else
#define TIME_FUNCTION
#define TIME_BLOCK(label)
#define TIME_BODY(label, c) c
#define PROFILER_END_TRANSLATION_UNIT
#endif
#else
#define profile_begin();
#define profile_end_and_print();
#define TIME_FUNCTION
#define TIME_BLOCK(label)
#define TIME_BODY(label, c) c
#define PROFILER_END_TRANSLATION_UNIT
#endif

#endif

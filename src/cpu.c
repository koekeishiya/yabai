#include "cpu.h"
#include <assert.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <mach/mach_host.h>

// forward declare notify func for errors
extern void notify(char *message, char *subtitle);

static void cpu_refresh_handler(CFRunLoopTimerRef timer, void *ctx)
{
    assert(ctx);
    if (ctx) {
        cpu_update((struct cpu_info *) ctx);
    }
}

void cpu_start_update(struct cpu_info* cpui)
{
    assert("cpui is not NULL" && cpui);
    // setup timer to update values
    CFRunLoopTimerContext ctx = {
        .version = 0,
        .info = cpui,
        .copyDescription = NULL,
        .retain = NULL,
        .release = NULL,
    };
    cpui->update_freq = cpui->update_freq > 0.0 ? cpui->update_freq : 1.0;
    cpui->refresh_timer = CFRunLoopTimerCreate(
            NULL,
            CFAbsoluteTimeGetCurrent() + cpui->update_freq,
            cpui->update_freq,
            0,
            0,
            cpu_refresh_handler,
            &ctx
    );
    CFRunLoopAddTimer(CFRunLoopGetMain(), cpui->refresh_timer, kCFRunLoopCommonModes);
    cpui->is_running = true;
}

void cpu_stop_update(struct cpu_info* cpui)
{
    assert("cpui is not NULL" && cpui);
    CFRunLoopRemoveTimer(CFRunLoopGetMain(), cpui->refresh_timer, kCFRunLoopCommonModes);
    CFRunLoopTimerInvalidate(cpui->refresh_timer);
}

void cpu_set_update_frequency(struct cpu_info* cpui, float seconds)
{
    assert("cpui is not NULL" && cpui);
    bool was_running = cpui->is_running;
    cpui->update_freq = seconds;
    if (was_running) {
        cpu_stop_update(cpui);
        cpu_start_update(cpui);
    }
}

void cpu_create(struct cpu_info* cpui)
{
    assert("cpui is not NULL" && cpui);
    char tmp[255];
    size_t len = sizeof(cpui->nphys_cpu);
    if (sysctlbyname("hw.physicalcpu", &cpui->nphys_cpu, &len, NULL, 0)) {
        snprintf(tmp, sizeof(tmp), "error! could not retrieve hw.physicalcpu: %s", strerror(errno));
        notify(tmp, NULL);
    }
    cpu_start_update(cpui);
}

void cpu_update(struct cpu_info* cpui)
{
    assert("cpui is not NULL" && cpui);
    mach_msg_type_number_t info_size = sizeof(processor_cpu_load_info_t);
    if (cpui->prev_load) {
        munmap(cpui->prev_load, vm_page_size);
    }
    cpui->prev_load = cpui->curr_load;
    if (host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &cpui->nlog_cpu,
                (processor_info_array_t *)&cpui->curr_load, &info_size)) {
        char tmp[255];
        snprintf(tmp, sizeof(tmp), "error! could not get processor load info");
        notify(tmp, NULL);
    }

    for (size_t cpu = 0; cpu < cpui->nlog_cpu; ++cpu) {
        memmove(cpui->load_avg[cpu], &cpui->load_avg[cpu][1], sizeof(*cpui->load_avg[cpu]) * (CPU_WINDOW_SZ - 1));
        memmove(cpui->sys_avg[cpu], &cpui->sys_avg[cpu][1], sizeof(*cpui->sys_avg[cpu]) * (CPU_WINDOW_SZ - 1));
        memmove(cpui->user_avg[cpu], &cpui->user_avg[cpu][1], sizeof(*cpui->user_avg[cpu]) * (CPU_WINDOW_SZ - 1));
    }
    if (cpui->prev_load) {
        for (size_t cpu = 0; cpu < cpui->nlog_cpu; ++cpu) {
            double total_ticks = 0.0;
            for (size_t s = 0; s < CPU_STATE_MAX; ++s) {
                total_ticks += cpui->curr_load[cpu].cpu_ticks[s] - cpui->prev_load[cpu].cpu_ticks[s];
            }
            cpui->user_avg[cpu][CPU_WINDOW_SZ - 1] =
                (cpui->curr_load[cpu].cpu_ticks[CPU_STATE_USER] -
                 cpui->prev_load[cpu].cpu_ticks[CPU_STATE_USER]) / total_ticks;
            cpui->sys_avg[cpu][CPU_WINDOW_SZ - 1] =
                (cpui->curr_load[cpu].cpu_ticks[CPU_STATE_SYSTEM] -
                 cpui->prev_load[cpu].cpu_ticks[CPU_STATE_SYSTEM]) / total_ticks;
            cpui->load_avg[cpu][CPU_WINDOW_SZ - 1] =
                cpui->user_avg[cpu][CPU_WINDOW_SZ - 1] +
                cpui->sys_avg[cpu][CPU_WINDOW_SZ - 1];
        }
    }
}

void cpu_destroy(struct cpu_info* cpui)
{
    assert("cpui is not NULL" && cpui);
    cpu_stop_update(cpui);
}

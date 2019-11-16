#include "cpu.h"
#include <assert.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <mach/mach_host.h>

extern void notify(char *message, char *subtitle);

static void cpu_refresh_handler(CFRunLoopTimerRef timer, void *ctx) {
    assert(ctx);
    if (ctx) {
        cpu_update((struct cpu_info *) ctx);
    }
}

void cpu_create(struct cpu_info* cpui) {
    assert("cpui is not NULL" && cpui);
    char tmp[255];
    size_t len = sizeof(cpui->nphys_cpu);
    if (sysctlbyname("hw.physicalcpu", &cpui->nphys_cpu, &len, NULL, 0)) {
        snprintf(tmp, sizeof(tmp), "could not retrieve hw.physicalcpu: %s", strerror(errno));
        notify(tmp, "error!");
    }

    // setup timer to update values
    CFRunLoopTimerContext ctx = {
        .version = 0,
        .info = cpui,
        .copyDescription = NULL,
        .retain = NULL,
        .release = NULL,
    };
    cpui->refresh_timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + 1, 1, 0, 0, cpu_refresh_handler, &ctx);
    CFRunLoopAddTimer(CFRunLoopGetMain(), cpui->refresh_timer, kCFRunLoopCommonModes);
}


void cpu_update(struct cpu_info* cpui) {
    assert("cpui is not NULL" && cpui);
    mach_msg_type_number_t info_size = sizeof(processor_cpu_load_info_t);
    if (cpui->prev_load) {
        // munmap(cpui->prev_load, vm_page_size);
    }
    cpui->prev_load = cpui->curr_load;
    if (host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &cpui->nlog_cpu,
                (processor_info_array_t *)&cpui->curr_load, &info_size)) {
        char tmp[255];
        snprintf(tmp, sizeof(tmp), "could not get processor load info");
        notify(tmp, "error!");
    }

    for (size_t cpu = 0; cpu < cpui->nlog_cpu; ++cpu) {
        memmove(cpui->load_avg[cpu], &cpui->load_avg[cpu][1], sizeof(*cpui->load_avg[cpu]) * (CPU_WINDOW_SZ - 1));
    }
    if (cpui->prev_load) {
        for (size_t cpu = 0; cpu < cpui->nlog_cpu; ++cpu) {
            double total_ticks = 0.0;
            for (size_t s = 0; s < CPU_STATE_MAX; ++s) {
                total_ticks += cpui->curr_load[cpu].cpu_ticks[s] - cpui->prev_load[cpu].cpu_ticks[s];
            }
            cpui->load_avg[cpu][CPU_WINDOW_SZ - 1] =
                (cpui->curr_load[cpu].cpu_ticks[CPU_STATE_USER] -
                 cpui->prev_load[cpu].cpu_ticks[CPU_STATE_USER]) / total_ticks;
        }
    }
}

void cpu_destroy(struct cpu_info* cpui) {
    assert("cpui is not NULL" && cpui);
    CFRunLoopRemoveTimer(CFRunLoopGetMain(), cpui->refresh_timer, kCFRunLoopCommonModes);
    CFRunLoopTimerInvalidate(cpui->refresh_timer);
}

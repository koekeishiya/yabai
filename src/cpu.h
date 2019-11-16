#ifndef CPU_H
#define CPU_H

#define CPU_WINDOW_SZ                       20

struct cpu_info
{
    int32_t nphys_cpu;
    unsigned int nlog_cpu;
    float load_avg[20][CPU_WINDOW_SZ];
    CFRunLoopTimerRef refresh_timer;
    processor_cpu_load_info_t prev_load;
    processor_cpu_load_info_t curr_load;
};

void cpu_create(struct cpu_info* cpui);
void cpu_update(struct cpu_info* cpui);
void cpu_destroy(struct cpu_info* cpui);

#endif

#ifndef CPU_H
#define CPU_H

#define CPU_MAX_NUM_CPUS		    32
#define CPU_WINDOW_SZ                       20

struct cpu_info
{
    int32_t nphys_cpu;
    unsigned int nlog_cpu;
    float update_freq;
    bool is_running;
    float user_avg[CPU_MAX_NUM_CPUS][CPU_WINDOW_SZ];
    float sys_avg[CPU_MAX_NUM_CPUS][CPU_WINDOW_SZ];
    float load_avg[CPU_MAX_NUM_CPUS][CPU_WINDOW_SZ];
    CFRunLoopTimerRef refresh_timer;
    processor_cpu_load_info_t prev_load;
    processor_cpu_load_info_t curr_load;
};

void cpu_start_update(struct cpu_info* cpui);
void cpu_stop_update(struct cpu_info* cpui);
void cpu_set_update_frequency(struct cpu_info* cpui, float seconds);

void cpu_create(struct cpu_info* cpui);
void cpu_update(struct cpu_info* cpui);
void cpu_destroy(struct cpu_info* cpui);

#endif

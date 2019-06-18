#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#if 0
extern CFArrayRef _LSCopyApplicationArrayInFrontToBackOrder(int negative_one, int one);
extern void _LSASNExtractHighAndLowParts(const void *asn, uint32_t *high, uint32_t *low);
extern CFTypeID _LSASNGetTypeID(void);
#endif

struct process_manager
{
    struct table process;
    EventTargetRef target;
    EventHandlerUPP handler;
    EventTypeSpec type[3];
    EventHandlerRef ref;
    pid_t front_pid;
    pid_t last_front_pid;
    ProcessSerialNumber finder_psn;
};

struct process *process_manager_find_process(struct process_manager *pm, ProcessSerialNumber *psn);
void process_manager_remove_process(struct process_manager *pm, ProcessSerialNumber *psn);
void process_manager_add_process(struct process_manager *pm, struct process *process);
// bool process_manager_next_process(ProcessSerialNumber *next_psn);
void process_manager_init(struct process_manager *pm);
bool process_manager_begin(struct process_manager *pm);
bool process_manager_end(struct process_manager *pm);

#endif

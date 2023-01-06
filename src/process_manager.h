#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#define PROCESS_EVENT_HANDLER(name) OSStatus name(EventHandlerCallRef ref, EventRef event, void *user_data)
typedef PROCESS_EVENT_HANDLER(process_event_handler);

struct process
{
    ProcessSerialNumber psn;
    pid_t pid;
    char *name;
    bool volatile terminated;
    void *ns_application;
};

struct process_manager
{
    struct table process;
    EventTargetRef target;
    EventHandlerUPP handler;
    EventTypeSpec type[3];
    EventHandlerRef ref;
    pid_t front_pid;
    pid_t last_front_pid;
    EventTime switch_event_time;
    ProcessSerialNumber finder_psn;
};

void process_destroy(struct process *process);
struct process *process_create(ProcessSerialNumber psn);
struct process *process_manager_find_process(struct process_manager *pm, ProcessSerialNumber *psn);
void process_manager_remove_process(struct process_manager *pm, ProcessSerialNumber *psn);
void process_manager_add_process(struct process_manager *pm, struct process *process);
void process_manager_init(struct process_manager *pm);
bool process_manager_begin(struct process_manager *pm);
bool process_manager_end(struct process_manager *pm);

#endif

#ifndef PROCESS_H
#define PROCESS_H

#include <Carbon/Carbon.h>
#include <stdbool.h>

#define PROCESS_EVENT_HANDLER(name) OSStatus name(EventHandlerCallRef ref, EventRef event, void *user_data)
typedef PROCESS_EVENT_HANDLER(process_event_handler);

struct process
{
    ProcessSerialNumber psn;
    pid_t pid;
    char *name;
    bool background;
    bool lsuielement;
    bool xpc;
    bool volatile terminated;
};

void process_destroy(struct process *process);
struct process *process_create(ProcessSerialNumber psn);

#endif

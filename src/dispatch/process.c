#include "process.h"

extern struct eventloop g_eventloop;

static PROCESS_EVENT_HANDLER(process_handler)
{
    struct process_manager *pm = (struct process_manager *) user_data;

    ProcessSerialNumber psn;
    if (GetEventParameter(event, kEventParamProcessID, typeProcessSerialNumber, NULL, sizeof(psn), NULL, &psn) != noErr) {
        return -1;
    }

    switch (GetEventKind(event)) {
    case kEventAppLaunched: {
        struct process *process = process_create(psn);
        if (!process) return noErr;

        if (!process->background && !process->lsuielement && !process->xpc) {
            process_manager_add_process(pm, process);

            struct event *event;
            event_create(event, APPLICATION_LAUNCHED, process);
            eventloop_post(&g_eventloop, event);
        } else {
            process_destroy(process);
        }
    } break;
    case kEventAppTerminated: {
        struct process *process = process_manager_find_process(pm, &psn);
        if (!process) return noErr;

        process->terminated = true;
        process_manager_remove_process(pm, &psn);

        struct event *event;
        event_create(event, APPLICATION_TERMINATED, process);
        eventloop_post(&g_eventloop, event);
    } break;
    }

    return noErr;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
struct process *process_create(ProcessSerialNumber psn)
{
    struct process *process = malloc(sizeof(struct process));
    memset(process, 0, sizeof(struct process));

    CFStringRef process_name_ref;
    if (CopyProcessName(&psn, &process_name_ref) == noErr) {
        process->name = cfstring_copy(process_name_ref);
        CFRelease(process_name_ref);
    }

    ProcessInfoRec process_info = {};
    process_info.processInfoLength = sizeof(ProcessInfoRec);
    GetProcessInformation(&psn, &process_info);

    process->psn = psn;
    GetProcessPID(&process->psn, &process->pid);
    process->background = (process_info.processMode & modeOnlyBackground) != 0;
    process->xpc = process_info.processType == 'XPC!';

    CFDictionaryRef process_dict = ProcessInformationCopyDictionary(&psn, kProcessDictionaryIncludeAllInformationMask);
    CFBooleanRef process_lsuielement = CFDictionaryGetValue(process_dict, CFSTR("LSUIElement"));
    if (process_lsuielement) {
        process->lsuielement = CFBooleanGetValue(process_lsuielement);
        CFRelease(process_lsuielement);
    }
    CFRelease(process_dict);

    return process;
}
#pragma clang diagnostic pop

void process_destroy(struct process *process)
{
    if (process->name) free(process->name);
    free(process);
}

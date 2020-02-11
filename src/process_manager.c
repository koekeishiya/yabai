#include "process_manager.h"

extern struct event_loop g_event_loop;

static TABLE_HASH_FUNC(hash_psn)
{
    unsigned long result = ((ProcessSerialNumber*) key)->lowLongOfPSN;
    result = (result + 0x7ed55d16) + (result << 12);
    result = (result ^ 0xc761c23c) ^ (result >> 19);
    result = (result + 0x165667b1) + (result << 5);
    result = (result + 0xd3a2646c) ^ (result << 9);
    result = (result + 0xfd7046c5) + (result << 3);
    result = (result ^ 0xb55a4f09) ^ (result >> 16);
    return result;
}

static TABLE_COMPARE_FUNC(compare_psn)
{
    return psn_equals(key_a, key_b);
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
    } else {
        process->name = string_copy("<unknown>");
    }

    ProcessInfoRec process_info = {};
    process_info.processInfoLength = sizeof(ProcessInfoRec);
    GetProcessInformation(&psn, &process_info);

    process->psn = psn;
    GetProcessPID(&process->psn, &process->pid);
    process->background = (process_info.processMode & modeOnlyBackground) != 0;
    process->xpc = process_info.processType == 'XPC!';

    CFDictionaryRef process_dict = ProcessInformationCopyDictionary(&psn, kProcessDictionaryIncludeAllInformationMask);
    if (process_dict) {
        CFBooleanRef process_lsuielement = CFDictionaryGetValue(process_dict, CFSTR("LSUIElement"));
        if (process_lsuielement) process->lsuielement = CFBooleanGetValue(process_lsuielement);
        CFBooleanRef process_lsbackground = CFDictionaryGetValue(process_dict, CFSTR("LSBackgroundOnly"));
        if (process_lsbackground) process->lsbackground = CFBooleanGetValue(process_lsbackground);
        CFRelease(process_dict);
    }

    return process;
}

void process_destroy(struct process *process)
{
    free(process->name);
    free(process);
}

static bool process_is_observable(struct process *process)
{
    if (process->lsbackground) {
        debug("%s: %s was marked as background only! ignoring..\n", __FUNCTION__, process->name);
        return false;
    }

    if (process->lsuielement) {
        debug("%s: %s was marked as agent! ignoring..\n", __FUNCTION__, process->name);
        return false;
    }

    if (process->background) {
        debug("%s: %s was marked as daemon! ignoring..\n", __FUNCTION__, process->name);
        return false;
    }

    if (process->xpc) {
        debug("%s: %s was marked as xpc service! ignoring..\n", __FUNCTION__, process->name);
        return false;
    }

    return true;
}

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

        if (process_is_observable(process)) {
            struct event *event = event_create(&g_event_loop, APPLICATION_LAUNCHED, process);
            event_loop_post(&g_event_loop, event);
            process_manager_add_process(pm, process);
        } else {
            process_destroy(process);
        }
    } break;
    case kEventAppTerminated: {
        struct process *process = process_manager_find_process(pm, &psn);
        if (!process) return noErr;

        process->terminated = true;
        process_manager_remove_process(pm, &psn);

        struct event *event = event_create(&g_event_loop, APPLICATION_TERMINATED, process);
        event_loop_post(&g_event_loop, event);
    } break;
    case kEventAppFrontSwitched: {
        struct process *process = process_manager_find_process(pm, &psn);
        if (!process) return noErr;

        struct event *event = event_create(&g_event_loop, APPLICATION_FRONT_SWITCHED, process);
        event_loop_post(&g_event_loop, event);
    } break;
    }

    return noErr;
}

static void
process_manager_add_running_processes(struct process_manager *pm)
{
    ProcessSerialNumber psn = { kNoProcess, kNoProcess };
    while (GetNextProcess(&psn) == noErr) {
        struct process *process = process_create(psn);
        if (!process) continue;

        if (process_is_observable(process)) {
            if (string_equals(process->name, "Finder")) {
                debug("%s: %s was found! caching psn..\n", __FUNCTION__, process->name);
                pm->finder_psn = psn;
            }

            process_manager_add_process(pm, process);
        } else {
            process_destroy(process);
        }
    }
}
#pragma clang diagnostic pop

struct process *process_manager_find_process(struct process_manager *pm, ProcessSerialNumber *psn)
{
    return table_find(&pm->process, psn);
}

void process_manager_remove_process(struct process_manager *pm, ProcessSerialNumber *psn)
{
    table_remove(&pm->process, psn);
}

void process_manager_add_process(struct process_manager *pm, struct process *process)
{
    table_add(&pm->process, &process->psn, process);
}

#if 0
bool process_manager_next_process(ProcessSerialNumber *next_psn)
{
    CFArrayRef applications =_LSCopyApplicationArrayInFrontToBackOrder(0xFFFFFFFE, 1);
    if (!applications) return false;

    bool found_front_psn = false;
    ProcessSerialNumber front_psn;
    _SLPSGetFrontProcess(&front_psn);

    for (int i = 0; i < CFArrayGetCount(applications); ++i) {
        CFTypeRef asn = CFArrayGetValueAtIndex(applications, i);
        assert(CFGetTypeID(asn) == _LSASNGetTypeID());
        _LSASNExtractHighAndLowParts(asn, &next_psn->highLongOfPSN, &next_psn->lowLongOfPSN);
        if (found_front_psn) break;
        found_front_psn = psn_equals(&front_psn, next_psn);
    }

    CFRelease(applications);
    return true;
}
#endif

void process_manager_init(struct process_manager *pm)
{
    pm->target = GetApplicationEventTarget();
    pm->handler = NewEventHandlerUPP(process_handler);
    pm->type[0].eventClass = kEventClassApplication;
    pm->type[0].eventKind  = kEventAppLaunched;
    pm->type[1].eventClass = kEventClassApplication;
    pm->type[1].eventKind  = kEventAppTerminated;
    pm->type[2].eventClass = kEventClassApplication;
    pm->type[2].eventKind  = kEventAppFrontSwitched;
    table_init(&pm->process, 125, hash_psn, compare_psn);
    process_manager_add_running_processes(pm);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
bool process_manager_begin(struct process_manager *pm)
{
    ProcessSerialNumber front_psn;
    _SLPSGetFrontProcess(&front_psn);
    GetProcessPID(&front_psn, &g_process_manager.front_pid);
    g_process_manager.last_front_pid = g_process_manager.front_pid;
    return InstallEventHandler(pm->target, pm->handler, 3, pm->type, pm, &pm->ref) == noErr;
}
#pragma clang diagnostic pop

bool process_manager_end(struct process_manager *pm)
{
    return RemoveEventHandler(pm->ref) == noErr;
}

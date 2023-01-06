extern struct event_loop g_event_loop;
extern void *g_workspace_context;

static TABLE_HASH_FUNC(hash_psn)
{
    return ((ProcessSerialNumber *) key)->lowLongOfPSN;
}

static TABLE_COMPARE_FUNC(compare_psn)
{
    return psn_equals(key_a, key_b);
}

static const char *process_name_blacklist[] =
{
    "loginwindow",
    "ScreenSaverEngine",
    "callservicesd",
    "UIKitSystem",
    "imklaunchagent",
    "LinkedNotesUIService",
    "Universal Control",
    "Dock",
    "WindowManager",
    "photolibraryd",
    "siriactionsd"
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
struct process *process_create(ProcessSerialNumber psn)
{
    ProcessInfoRec process_info = { .processInfoLength = sizeof(ProcessInfoRec) };
    GetProcessInformation(&psn, &process_info);

    if (process_info.processType == 'XPC!') {
        debug("%s: xpc service detected! ignoring..\n", __FUNCTION__);
        return NULL;
    }

    CFStringRef process_name_ref = NULL;
    CopyProcessName(&psn, &process_name_ref);

    if (!process_name_ref) {
        debug("%s: could not retrieve process name! ignoring..\n", __FUNCTION__);
        return NULL;
    }

    char *process_name = cfstring_copy(process_name_ref);
    CFRelease(process_name_ref);

    for (int i = 0; i < array_count(process_name_blacklist); ++i) {
        if (string_equals(process_name, process_name_blacklist[i])) {
            debug("%s: %s is blacklisted! ignoring..\n", __FUNCTION__, process_name);
            free(process_name);
            return NULL;
        }
    }

    struct process *process = malloc(sizeof(struct process));
    process->psn = psn;
    GetProcessPID(&process->psn, &process->pid);
    process->name = process_name;
    process->terminated = false;
    process->ns_application = workspace_application_create_running_ns_application(process);
    return process;
}

void process_destroy(struct process *process)
{
    workspace_application_destroy_running_ns_application(g_workspace_context, process);
    free(process->name);
    free(process);
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
        if (process_manager_find_process(pm, &psn)) {

            //
            // NOTE(koekeishiya): Some garbage applications (e.g Steam) are reported twice with the same PID and PSN for some hecking reason.
            // It is by definition NOT possible for two processes to exist at the same time with the same PID and PSN.
            // If we detect such a scenario we simply discard the dupe notification..
            //

            return noErr;
        }

        struct process *process = process_create(psn);
        if (!process) return noErr;

        process_manager_add_process(pm, process);
        event_loop_post(&g_event_loop, APPLICATION_LAUNCHED, process, 0, NULL);
    } break;
    case kEventAppTerminated: {
        struct process *process = process_manager_find_process(pm, &psn);
        if (!process) return noErr;

        process->terminated = true;
        process_manager_remove_process(pm, &psn);
        __asm__ __volatile__ ("" ::: "memory");

        event_loop_post(&g_event_loop, APPLICATION_TERMINATED, process, 0, NULL);
    } break;
    case kEventAppFrontSwitched: {
        struct process *process = process_manager_find_process(pm, &psn);
        if (!process) return noErr;

        event_loop_post(&g_event_loop, APPLICATION_FRONT_SWITCHED, process, 0, NULL);
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

        if (string_equals(process->name, "Finder")) {
            debug("%s: %s (%d) was found! caching psn..\n", __FUNCTION__, process->name, process->pid);
            pm->finder_psn = psn;
        }

        process_manager_add_process(pm, process);
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

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    process_manager_add_running_processes(pm);
    [pool drain];
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
bool process_manager_begin(struct process_manager *pm)
{
    ProcessSerialNumber front_psn;
    _SLPSGetFrontProcess(&front_psn);
    GetProcessPID(&front_psn, &g_process_manager.front_pid);
    g_process_manager.last_front_pid = g_process_manager.front_pid;
    g_process_manager.switch_event_time = GetCurrentEventTime();
    return InstallEventHandler(pm->target, pm->handler, 3, pm->type, pm, &pm->ref) == noErr;
}
#pragma clang diagnostic pop

bool process_manager_end(struct process_manager *pm)
{
    return RemoveEventHandler(pm->ref) == noErr;
}

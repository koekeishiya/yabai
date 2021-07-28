#include "workspace.h"

extern struct event_loop g_event_loop;

void workspace_event_handler_init(void **context)
{
    workspace_context *ws_context = [workspace_context alloc];
    *context = ws_context;
}

void workspace_event_handler_begin(void **context)
{
    workspace_context *ws_context = *context;
    [ws_context init];
}

void workspace_event_handler_end(void *context)
{
    workspace_context *ws_context = (workspace_context *) context;
    [ws_context dealloc];
}

void *workspace_application_create_running_ns_application(struct process *process)
{
    return [[NSRunningApplication runningApplicationWithProcessIdentifier:process->pid] retain];
}

void workspace_application_destroy_running_ns_application(void *ws_context, struct process *process)
{
    NSRunningApplication *application = process->ns_application;

    if (application) {
        if ([application observationInfo]) {

            //
            // :WorstApiEverMade
            //
            // NOTE(koekeishiya): Because the developers of this API did such an amazing job
            // there is no way for us to actually just friggin loop through the currently
            // registered observations and then call removeObservation on them..
            //
            // Instead we just try to force remove the observations that **could** be present
            // at this point in time, because it will complain if we try to actually release
            // the object when it has observers present.
            //
            // We can't actually correctly track whether it did actually get unobserved previously,
            // because even when our notification callback is triggered it will claim that we try
            // to remove a non-existing observation when it just called us back.
            //

            @try {
                [application removeObserver:ws_context forKeyPath:@"activationPolicy" context:process];
            } @catch (NSException * __unused exception) {}

            @try {
                [application removeObserver:ws_context forKeyPath:@"finishedLaunching" context:process];
            } @catch (NSException * __unused exception) {}
        }

        [application release];
    }
}

void workspace_application_observe_finished_launching(void *context, struct process *process)
{
    NSRunningApplication *application = process->ns_application;
    if (application) {
        [application addObserver:context forKeyPath:@"finishedLaunching" options:NSKeyValueObservingOptionInitial|NSKeyValueObservingOptionNew context:process];
    } else {
        debug("%s: could not subscribe to activation policy changes for %s (%d)\n", __FUNCTION__, process->name, process->pid);
    }
}

void workspace_application_observe_activation_policy(void *context, struct process *process)
{
    NSRunningApplication *application = process->ns_application;
    if (application) {
        [application addObserver:context forKeyPath:@"activationPolicy" options:NSKeyValueObservingOptionInitial|NSKeyValueObservingOptionNew context:process];
    } else {
        debug("%s: could not subscribe to finished launching changes for %s (%d)\n", __FUNCTION__, process->name, process->pid);
    }
}

bool workspace_application_is_observable(struct process *process)
{
    NSRunningApplication *application = process->ns_application;
    if (application) {
        return [application activationPolicy] != NSApplicationActivationPolicyProhibited;
    } else {
        return false;
    }
}

bool workspace_application_is_finished_launching(struct process *process)
{
    NSRunningApplication *application = process->ns_application;
    if (application) {
        return [application isFinishedLaunching] == YES;
    } else {
        return false;
    }
}

bool workspace_is_macos_bigsur(void)
{
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    return (version.majorVersion == 11) || (version.majorVersion == 10 && version.minorVersion == 16);
}

bool workspace_is_macos_catalina(void)
{
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    return version.majorVersion == 10 && version.minorVersion == 15;
}

bool workspace_is_macos_mojave(void)
{
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    return version.majorVersion == 10 && version.minorVersion == 14;
}

bool workspace_is_macos_highsierra(void)
{
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    return version.majorVersion == 10 && version.minorVersion == 13;
}

@implementation workspace_context
- (id)init
{
    if ((self = [super init])) {
       [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(activeDisplayDidChange:)
                name:@"NSWorkspaceActiveDisplayDidChangeNotification"
                object:nil];

       [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(activeSpaceDidChange:)
                name:NSWorkspaceActiveSpaceDidChangeNotification
                object:nil];

       [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(didHideApplication:)
                name:NSWorkspaceDidHideApplicationNotification
                object:nil];

       [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(didUnhideApplication:)
                name:NSWorkspaceDidUnhideApplicationNotification
                object:nil];

       [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(didWake:)
                name:NSWorkspaceDidWakeNotification
                object:nil];

       [[NSNotificationCenter defaultCenter] addObserver:self
                selector:@selector(didRestartDock:)
                name:@"NSApplicationDockDidRestartNotification"
                object:nil];

       [[NSDistributedNotificationCenter defaultCenter] addObserver:self
                selector:@selector(didChangeMenuBarHiding:)
                name:@"AppleInterfaceMenuBarHidingChangedNotification"
                object:nil];

       [[NSDistributedNotificationCenter defaultCenter] addObserver:self
                selector:@selector(didChangeDockPref:)
                name:@"com.apple.dock.prefchanged"
                object:nil];
    }

    return self;
}

- (void)dealloc
{
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"activationPolicy"]) {
        id result = [change objectForKey:NSKeyValueChangeNewKey];
        if ([result intValue] != NSApplicationActivationPolicyProhibited) {
            struct process *process = context;
            assert(!process->terminated);

            debug("%s: activation policy changed for %s (%d)\n", __FUNCTION__, process->name, process->pid);
            event_loop_post(&g_event_loop, APPLICATION_LAUNCHED, process, 0, NULL);

            //
            // :WorstApiEverMade
            //
            // NOTE(koekeishiya): For some stupid reason it is possible to get notified by the system
            // about a change, and NOT being able to remove ourselves from observation because
            // it claims that we are not observing the key-path, but we clearly are, as we would
            // otherwise not be here in the first place..
            //

            @try {
                [object removeObserver:self forKeyPath:@"activationPolicy" context:process];
            } @catch (NSException * __unused exception) {}
        }
    }

    if ([keyPath isEqualToString:@"finishedLaunching"]) {
        id result = [change objectForKey:NSKeyValueChangeNewKey];
        if ([result intValue] == 1) {
            struct process *process = context;
            assert(!process->terminated);

            debug("%s: %s (%d) finished launching\n", __FUNCTION__, process->name, process->pid);
            event_loop_post(&g_event_loop, APPLICATION_LAUNCHED, process, 0, NULL);

            //
            // :WorstApiEverMade
            //
            // NOTE(koekeishiya): For some stupid reason it is possible to get notified by the system
            // about a change, and NOT being able to remove ourselves from observation because
            // it claims that we are not observing the key-path, but we clearly are, as we would
            // otherwise not be here in the first place..
            //

            @try {
                [object removeObserver:self forKeyPath:@"finishedLaunching" context:process];
            } @catch (NSException * __unused exception) {}
        }
    }
}

- (void)didWake:(NSNotification *)notification
{
    event_loop_post(&g_event_loop, SYSTEM_WOKE, NULL, 0, NULL);
}

- (void)didRestartDock:(NSNotification *)notification
{
    event_loop_post(&g_event_loop, DOCK_DID_RESTART, NULL, 0, NULL);
}

- (void)didChangeMenuBarHiding:(NSNotification *)notification
{
    event_loop_post(&g_event_loop, MENU_BAR_HIDDEN_CHANGED, NULL, 0, NULL);
}

- (void)didChangeDockPref:(NSNotification *)notification
{
    event_loop_post(&g_event_loop, DOCK_DID_CHANGE_PREF, NULL, 0, NULL);
}

- (void)activeDisplayDidChange:(NSNotification *)notification
{
    event_loop_post(&g_event_loop, DISPLAY_CHANGED, NULL, 0, NULL);
}

- (void)activeSpaceDidChange:(NSNotification *)notification
{
    event_loop_post(&g_event_loop, SPACE_CHANGED, NULL, 0, NULL);
}

- (void)didHideApplication:(NSNotification *)notification
{
    pid_t pid = [[notification.userInfo objectForKey:NSWorkspaceApplicationKey] processIdentifier];
    event_loop_post(&g_event_loop, APPLICATION_HIDDEN, (void *)(intptr_t) pid, 0, NULL);
}

- (void)didUnhideApplication:(NSNotification *)notification
{
    pid_t pid = [[notification.userInfo objectForKey:NSWorkspaceApplicationKey] processIdentifier];
    event_loop_post(&g_event_loop, APPLICATION_VISIBLE, (void *)(intptr_t) pid, 0, NULL);
}

@end

#include "workspace.h"

extern struct eventloop g_eventloop;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static struct workspace_process *
workspace_process_create(NSNotification *notification)
{
    struct workspace_process *process = malloc(sizeof(struct workspace_process));
    memset(process, 0, sizeof(struct workspace_process));

    process->pid = [[notification.userInfo objectForKey:NSWorkspaceApplicationKey] processIdentifier];
    GetProcessForPID(process->pid, &process->psn);

    CFStringRef process_name_ref;
    if (CopyProcessName(&process->psn, &process_name_ref) == noErr) {
        process->name = cfstring_copy(process_name_ref);
        CFRelease(process_name_ref);
    }

    return process;
}
#pragma clang diagnostic pop

void workspace_process_destroy(struct workspace_process *process)
{
    if (process->name) free(process->name);
    free(process);
}

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
                selector:@selector(didActivateApplication:)
                name:NSWorkspaceDidActivateApplicationNotification
                object:nil];

       [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(didDeactivateApplication:)
                name:NSWorkspaceDidDeactivateApplicationNotification
                object:nil];

       [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(didHideApplication:)
                name:NSWorkspaceDidHideApplicationNotification
                object:nil];

       [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(didUnhideApplication:)
                name:NSWorkspaceDidUnhideApplicationNotification
                object:nil];
    }

    return self;
}

- (void)dealloc
{
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
    [super dealloc];
}

- (void)activeDisplayDidChange:(NSNotification *)notification
{
    struct event *event;
    event_create(event, DISPLAY_CHANGED, NULL);
    eventloop_post(&g_eventloop, event);
}

- (void)activeSpaceDidChange:(NSNotification *)notification
{
    struct event *event;
    event_create(event, SPACE_CHANGED, NULL);
    eventloop_post(&g_eventloop, event);
}

- (void)didActivateApplication:(NSNotification *)notification
{
    struct workspace_process *process = workspace_process_create(notification);
    if (process) {
        struct event *event;
        event_create(event, APPLICATION_ACTIVATED, process);
        eventloop_post(&g_eventloop, event);
    }
}

- (void)didDeactivateApplication:(NSNotification *)notification
{
    struct workspace_process *process = workspace_process_create(notification);
    if (process) {
        struct event *event;
        event_create(event, APPLICATION_DEACTIVATED, process);
        eventloop_post(&g_eventloop, event);
    }
}

- (void)didHideApplication:(NSNotification *)notification
{
    struct workspace_process *process = workspace_process_create(notification);
    if (process) {
        struct event *event;
        event_create(event, APPLICATION_HIDDEN, process);
        eventloop_post(&g_eventloop, event);
    }
}

- (void)didUnhideApplication:(NSNotification *)notification
{
    struct workspace_process *process = workspace_process_create(notification);
    if (process) {
        struct event *event;
        event_create(event, APPLICATION_VISIBLE, process);
        eventloop_post(&g_eventloop, event);
    }
}

@end

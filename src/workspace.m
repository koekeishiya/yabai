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

- (void)didWake:(NSNotification *)notification
{
    struct event *event;
    event_create(event, SYSTEM_WOKE, NULL);
    event_loop_post(&g_event_loop, event);
}

- (void)didRestartDock:(NSNotification *)notification
{
    struct event *event;
    event_create(event, DOCK_DID_RESTART, NULL);
    event_loop_post(&g_event_loop, event);
}

- (void)didChangeMenuBarHiding:(NSNotification *)notification
{
    struct event *event;
    event_create(event, MENU_BAR_HIDDEN_CHANGED, NULL);
    event_loop_post(&g_event_loop, event);
}

- (void)didChangeDockPref:(NSNotification *)notification
{
    struct event *event;
    event_create(event, DOCK_DID_CHANGE_PREF, NULL);
    event_loop_post(&g_event_loop, event);
}

- (void)activeDisplayDidChange:(NSNotification *)notification
{
    struct event *event;
    event_create(event, DISPLAY_CHANGED, NULL);
    event_loop_post(&g_event_loop, event);
}

- (void)activeSpaceDidChange:(NSNotification *)notification
{
    struct event *event;
    event_create(event, SPACE_CHANGED, NULL);
    event_loop_post(&g_event_loop, event);
}

- (void)didHideApplication:(NSNotification *)notification
{
    pid_t pid = [[notification.userInfo objectForKey:NSWorkspaceApplicationKey] processIdentifier];

    struct event *event;
    event_create(event, APPLICATION_HIDDEN, (void *)(intptr_t) pid);
    event_loop_post(&g_event_loop, event);
}

- (void)didUnhideApplication:(NSNotification *)notification
{
    pid_t pid = [[notification.userInfo objectForKey:NSWorkspaceApplicationKey] processIdentifier];

    struct event *event;
    event_create(event, APPLICATION_VISIBLE, (void *)(intptr_t) pid);
    event_loop_post(&g_event_loop, event);
}

@end

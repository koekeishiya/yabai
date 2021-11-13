#ifndef WORKSPACE_H
#define WORKSPACE_H

@interface workspace_context : NSObject {
}
- (id)init;
@end

void workspace_event_handler_init(void **context);
void workspace_event_handler_begin(void **context);
void workspace_event_handler_end(void *context);

struct process;
void *workspace_application_create_running_ns_application(struct process *process);
void workspace_application_destroy_running_ns_application(void *context, struct process *process);
bool workspace_application_is_observable(struct process *process);
bool workspace_application_is_finished_launching(struct process *process);
void workspace_application_observe_finished_launching(void *context, struct process *process);
void workspace_application_observe_activation_policy(void *context, struct process *process);
bool workspace_is_macos_monterey(void);
bool workspace_is_macos_bigsur(void);
bool workspace_is_macos_catalina(void);
bool workspace_is_macos_mojave(void);
bool workspace_is_macos_highsierra(void);

#endif

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
bool workspace_application_is_observable(struct process *process);
bool workspace_application_is_finished_launching(struct process *process);
void workspace_application_observe_finished_launching(void *context, struct process *process);
void workspace_application_observe_activation_policy(void *context, struct process *process);

#endif

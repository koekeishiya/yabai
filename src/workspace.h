#ifndef WORKSPACE_H
#define WORKSPACE_H

@interface workspace_context : NSObject {
}
- (id)init;
@end

void workspace_event_handler_init(void **context);
void workspace_event_handler_begin(void **context);
void workspace_event_handler_end(void *context);
bool workspace_application_is_observable(pid_t pid);
bool workspace_application_is_finished_launching(pid_t pid);

#endif

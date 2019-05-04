#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <Cocoa/Cocoa.h>

struct workspace_process
{
    ProcessSerialNumber psn;
    pid_t pid;
    char *name;
};

@interface workspace_context : NSObject {
}
- (id)init;
@end

void workspace_process_destroy(struct workspace_process *process);
void workspace_event_handler_init(void **context);
void workspace_event_handler_begin(void **context);
void workspace_event_handler_end(void *context);

#endif

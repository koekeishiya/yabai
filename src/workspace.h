#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <Cocoa/Cocoa.h>

@interface workspace_context : NSObject {
}
- (id)init;
@end

void workspace_event_handler_init(void **context);
void workspace_event_handler_begin(void **context);
void workspace_event_handler_end(void *context);

#endif

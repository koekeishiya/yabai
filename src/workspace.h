#ifndef WORKSPACE_H
#define WORKSPACE_H

#define SUPPORTED_MACOS_VERSION_LIST    \
    SUPPORT_MACOS_VERSION(sonoma,   14) \
    SUPPORT_MACOS_VERSION(ventura,  13) \
    SUPPORT_MACOS_VERSION(monterey, 12) \
    SUPPORT_MACOS_VERSION(bigsur,   11)

#define SUPPORT_MACOS_VERSION(name, major_version) \
static bool _workspace_is_macos_version_##name; \
static inline bool workspace_is_macos_##name(void) \
{ \
    return _workspace_is_macos_version_##name; \
}
    SUPPORTED_MACOS_VERSION_LIST
#undef SUPPORT_MACOS_VERSION

@interface workspace_context : NSObject {
}
- (id)init;
@end

struct process;
void *workspace_application_create_running_ns_application(struct process *process);
void workspace_application_destroy_running_ns_application(void *context, struct process *process);
bool workspace_application_is_observable(struct process *process);
bool workspace_application_is_finished_launching(struct process *process);
void workspace_application_observe_finished_launching(void *context, struct process *process);
void workspace_application_observe_activation_policy(void *context, struct process *process);
int workspace_display_notch_height(uint32_t did);
pid_t workspace_get_dock_pid(void);
bool workspace_event_handler_begin(void **context);
bool workspace_is_macos_sonoma14_5_or_newer(void);

#endif

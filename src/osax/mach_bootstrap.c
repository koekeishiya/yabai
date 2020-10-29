#include <mach/mach.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>

extern void _pthread_set_self(char *);

static void drop_privileges(void)
{
    if (geteuid() == 0) {
        setgid(getgid());
        setuid(getuid());
    }
}

static void *mach_load_payload(void *context)
{
    drop_privileges();

    void *handle = dlopen("/Library/ScriptingAdditions/yabai.osax/Contents/Resources/payload.bundle/Contents/MacOS/payload", RTLD_NOW);
    if (!handle) dlerror();

    return NULL;
}

void mach_bootstrap_entry_point(char *param)
{
    int policy;
    pthread_t thread;
    pthread_attr_t attr;
    struct sched_param sched;

    _pthread_set_self(param);
    pthread_attr_init(&attr);
    pthread_attr_getschedpolicy(&attr, &policy);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    sched.sched_priority = sched_get_priority_max(policy);
    pthread_attr_setschedparam(&attr, &sched);
    pthread_create(&thread, &attr, &mach_load_payload, NULL);
    pthread_attr_destroy(&attr);

    thread_suspend(mach_thread_self());
}

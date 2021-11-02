#include <mach/mach.h>
#include <pthread.h>
#include <pthread_spis.h>
#include <unistd.h>
#include <dlfcn.h>

extern void _pthread_set_self(pthread_t *);

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

void mach_bootstrap_entry_point(void)
{
    pthread_t thread;
    _pthread_set_self(&thread);
    pthread_create_from_mach_thread(&thread, NULL, &mach_load_payload, NULL);
#ifdef _x86_64_
    for (;;) __asm__ __volatile__ ("movq %0, %%rax;" ::"r"(0x7961626169) : "%rax");
#elif _arm64_
    for (;;) __asm__ __volatile__ ("mov %0, %%x15;" ::"r"(0x7961626169) : "%x15");
#endif
}

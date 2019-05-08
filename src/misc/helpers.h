#ifndef HELPERS_H
#define HELPERS_H

extern AXError _AXUIElementGetWindow(AXUIElementRef ref, uint32_t *wid);

static inline bool is_root(void)
{
    return getuid() == 0 || geteuid() == 0;
}

static bool fork_exec_wait(char *command)
{
    static const char *shell = "/bin/bash";
    static const char *arg   = "-c";

    int pid = fork();
    if (pid == -1) {
        return false;
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        char *exec[] = { (char *) shell, (char *) arg, command, NULL};
        exit(execvp(exec[0], exec));
    }

    return true;
}

static bool ax_privilege(void)
{
    const void *keys[] = { kAXTrustedCheckOptionPrompt };
    const void *values[] = { kCFBooleanTrue };
    CFDictionaryRef options = CFDictionaryCreate(NULL, keys, values, array_count(keys),
                                                 &kCFCopyStringDictionaryKeyCallBacks,
                                                 &kCFTypeDictionaryValueCallBacks);
    bool result = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);
    return result;
}

static inline uint32_t ax_window_id(AXUIElementRef ref)
{
    uint32_t wid = 0;
    _AXUIElementGetWindow(ref, &wid);
    return wid;
}

static inline pid_t ax_window_pid(AXUIElementRef ref)
{
    return *(pid_t *)((void *) ref + 0x10);
}

#endif

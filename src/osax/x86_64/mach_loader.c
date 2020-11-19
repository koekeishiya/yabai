#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <mach/mach.h>
#include <dlfcn.h>

static bool mach_loader_lookup_image(void *ptr, void **image, uint64_t *image_size)
{
    uint64_t addr = (uint64_t) ptr;
    int count = _dyld_image_count();

    for (int i = 0; i < count; ++i) {
        const struct mach_header_64 *header = (const struct mach_header_64 *) _dyld_get_image_header(i);
        const struct section_64 *section = getsectbynamefromheader_64(header, SEG_TEXT, SECT_TEXT);
        if (!section) continue;

        uint64_t start = section->addr + _dyld_get_image_vmaddr_slide(i);
        uint64_t stop = start + section->size;

        if (addr >= start && addr <= stop) {
            const char *name = _dyld_get_image_name(i);

            struct stat sb;
            if (stat(name, &sb)) return false;

            *image = (void *) header;
            *image_size = sb.st_size;
            return true;
        }
    }

    return false;
}

bool mach_loader_inject_payload(pid_t pid)
{
    mach_port_t task = 0;
    thread_act_t thread = 0;
    x86_thread_state64_t thread_state = {};
    void *image = 0;
    uint64_t image_size = 0;
    vm_address_t code = 0;
    vm_address_t stack = 0;
    vm_size_t stack_size = 16 * 1024;
    uint64_t stack_contents = 0x00000000CAFEBABE;

    void *bootstrap_handle = dlopen("/Library/ScriptingAdditions/yabai.osax/Contents/MacOS/mach_bootstrap", RTLD_NOW);
    if (!bootstrap_handle) {
        fprintf(stderr, "could not load bootstrap object file\n");
        return false;
    }

    void *bootstrap_entry = dlsym(bootstrap_handle, "mach_bootstrap_entry_point");
    if (!bootstrap_entry) {
        fprintf(stderr, "could not load bootstrap entry point\n");
        return false;
    }

    if (task_for_pid(mach_task_self(), pid, &task) != KERN_SUCCESS) {
        fprintf(stderr, "could not retrieve task port for pid: %d\n", pid);
        return false;
    }

    if (vm_allocate(task, &stack, stack_size, 1) != KERN_SUCCESS) {
        fprintf(stderr, "could not allocate stack segment\n");
        return false;
    }

    if (!mach_loader_lookup_image(bootstrap_entry, &image, &image_size)) {
        fprintf(stderr, "could not locate bootstrap image\n");
        return false;
    }

    if (vm_allocate(task, &code, image_size, 1) != KERN_SUCCESS) {
        fprintf(stderr, "could not allocate code segment\n");
        return false;
    }

    if (vm_protect(task, code, image_size, 0, VM_PROT_EXECUTE | VM_PROT_WRITE | VM_PROT_READ) != KERN_SUCCESS) {
        fprintf(stderr, "could not change protection for code segment\n");
        return false;
    }

    if (vm_write(task, code, (pointer_t)image, image_size) != KERN_SUCCESS) {
        fprintf(stderr, "could not copy image into code segment\n");
        return false;
    }

    if (vm_write(task, stack, (pointer_t) &stack_contents, sizeof(uint64_t)) != KERN_SUCCESS) {
        fprintf(stderr, "could not copy dummy return address into stack segment\n");
        return false;
    }

    thread_state.__rdi = (uint64_t) stack;
    thread_state.__rip = (uint64_t) code + (uint64_t)(((void *) bootstrap_entry) - image);
    thread_state.__rsp = (uint64_t) (stack + (stack_size / 2) - 8);

    if (thread_create_running(task, x86_THREAD_STATE64, (thread_state_t)&thread_state, x86_THREAD_STATE64_COUNT, &thread) != KERN_SUCCESS) {
        fprintf(stderr, "could not spawn remote thread\n");
        return false;
    }

    return true;
}

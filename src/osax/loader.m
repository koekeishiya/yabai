#include <Cocoa/Cocoa.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __arm64__
#include <ptrauth.h>
kern_return_t (*_thread_convert_thread_state)(thread_act_t thread, int direction, thread_state_flavor_t flavor, thread_state_t in_state, mach_msg_type_number_t in_stateCnt, thread_state_t out_state, mach_msg_type_number_t *out_stateCnt);
#endif

static char *payload_path = "/Library/ScriptingAdditions/yabai.osax/Contents/Resources/payload.bundle/Contents/MacOS/payload";

static char shell_code[] =
#ifdef __x86_64__
"\x55"                             // push       rbp
"\x48\x89\xE5"                     // mov        rbp, rsp
"\x48\x83\xEC\x10"                 // sub        rsp, 0x10
"\x48\x8D\x7D\xF8"                 // lea        rdi, qword [rbp+var_8]
"\x31\xC0"                         // xor        eax, eax
"\x89\xC1"                         // mov        ecx, eax
"\x48\x8D\x15\x1E\x00\x00\x00"     // lea        rdx, qword ptr [rip+0x1E]
"\x48\x89\xCE"                     // mov        rsi, rcx
"\x48\xB8"                         // movabs     rax, pthread_create_from_mach_thread
"\x00\x00\x00\x00\x00\x00\x00\x00" //
"\xFF\xD0"                         // call       rax
"\x48\x83\xC4\x10"                 // add        rsp, 0x10
"\x5D"                             // pop        rbp
"\x48\xC7\xC0\x65\x62\x61\x79"     // mov        rax, 0x79616265
"\xEB\xFE"                         // jmp        0x0
"\xC3"                             // ret
"\x55"                             // push       rbp
"\x48\x89\xE5"                     // mov        rbp, rsp
"\xBE\x01\x00\x00\x00"             // mov        esi, 0x1
"\x48\x8D\x3D\x16\x00\x00\x00"     // lea        rdi, qword ptr [rip+0x16]
"\x48\xB8"                         // movabs     rax, dlopen
"\x00\x00\x00\x00\x00\x00\x00\x00" //
"\xFF\xD0"                         // call       rax
"\x31\xF6"                         // xor        esi, esi
"\x89\xF7"                         // mov        edi, esi
"\x48\x89\xF8"                     // mov        rax, rdi
"\x5D"                             // pop        rbp
"\xC3"                             // ret
#elif __arm64__
"\xFF\xC3\x00\xD1"                 // sub        sp, sp, #0x30
"\xFD\x7B\x02\xA9"                 // stp        x29, x30, [sp, #0x20]
"\xFD\x83\x00\x91"                 // add        x29, sp, #0x20
"\xA0\xC3\x1F\xB8"                 // stur       w0, [x29, #-0x4]
"\xE1\x0B\x00\xF9"                 // str        x1, [sp, #0x10]
"\xE0\x23\x00\x91"                 // add        x0, sp, #0x8
"\x08\x00\x80\xD2"                 // mov        x8, #0
"\xE8\x07\x00\xF9"                 // str        x8, [sp, #0x8]
"\xE1\x03\x08\xAA"                 // mov        x1, x8
"\xE2\x01\x00\x10"                 // adr        x2, #0x3C
"\xE2\x23\xC1\xDA"                 // paciza     x2
"\xE3\x03\x08\xAA"                 // mov        x3, x8
"\x49\x01\x00\x10"                 // adr        x9, #0x28 ; pthread_create_from_mach_thread
"\x29\x01\x40\xF9"                 // ldr        x9, [x9]
"\x20\x01\x3F\xD6"                 // blr        x9
"\xA0\x4C\x8C\xD2"                 // movz       x0, #0x6265
"\x20\x2C\xAF\xF2"                 // movk       x0, #0x7961, lsl #16
"\x09\x00\x00\x10"                 // adr        x9, #0
"\x20\x01\x1F\xD6"                 // br         x9
"\xFD\x7B\x42\xA9"                 // ldp        x29, x30, [sp, #0x20]
"\xFF\xC3\x00\x91"                 // add        sp, sp, #0x30
"\xC0\x03\x5F\xD6"                 // ret
"\x00\x00\x00\x00\x00\x00\x00\x00" //
"\x7F\x23\x03\xD5"                 // pacibsp
"\xFF\xC3\x00\xD1"                 // sub        sp, sp, #0x30
"\xFD\x7B\x02\xA9"                 // stp        x29, x30, [sp, #0x20]
"\xFD\x83\x00\x91"                 // add        x29, sp, #0x20
"\xA0\xC3\x1F\xB8"                 // stur       w0, [x29, #-0x4]
"\xE1\x0B\x00\xF9"                 // str        x1, [sp, #0x10]
"\x21\x00\x80\xD2"                 // mov        x1, #1
"\x60\x01\x00\x10"                 // adr        x0, #0x2c ; payload_path
"\x09\x01\x00\x10"                 // adr        x9, #0x20 ; dlopen
"\x29\x01\x40\xF9"                 // ldr        x9, [x9]
"\x20\x01\x3F\xD6"                 // blr        x9
"\x09\x00\x80\x52"                 // mov        w9, #0
"\xE0\x03\x09\xAA"                 // mov        x0, x9
"\xFD\x7B\x42\xA9"                 // ldp        x29, x30, [sp, #0x20]
"\xFF\xC3\x00\x91"                 // add        sp, sp, #0x30
"\xFF\x0F\x5F\xD6"                 // retab
"\x00\x00\x00\x00\x00\x00\x00\x00" //
#endif
"\x00\x00\x00\x00\x00\x00\x00\x00" // empty space for payload_path
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00";

static pid_t get_dock_pid(void)
{
    NSArray *list = [NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.apple.dock"];

    if (list.count == 1) {
        NSRunningApplication *dock = list[0];
        if ([dock isFinishedLaunching] == YES) {
            return [dock processIdentifier];
        }
    }

    return 0;
}
int main(int argc, char **argv)
{
    int result = 0;
    mach_port_t task = 0;
    thread_act_t thread = 0;
    mach_vm_address_t code = 0;
    mach_vm_address_t stack = 0;
    vm_size_t stack_size = 16 * 1024;
    uint64_t stack_contents = 0x00000000CAFEBABE;
    pid_t pid = get_dock_pid();

    if (!pid) {
        fprintf(stderr, "could not locate Dock.app pid\n");
        return 1;
    }

    if (task_for_pid(mach_task_self(), pid, &task) != KERN_SUCCESS) {
        fprintf(stderr, "could not retrieve task port for pid: %d\n", pid);
        return 1;
    }

    if (mach_vm_allocate(task, &stack, stack_size, VM_FLAGS_ANYWHERE) != KERN_SUCCESS) {
        fprintf(stderr, "could not allocate stack segment\n");
        return 1;
    }

    if (mach_vm_write(task, stack, (vm_address_t) &stack_contents, sizeof(uint64_t)) != KERN_SUCCESS) {
        fprintf(stderr, "could not copy dummy return address into stack segment\n");
        return 1;
    }

    if (vm_protect(task, stack, stack_size, 1, VM_PROT_READ | VM_PROT_WRITE) != KERN_SUCCESS) {
        fprintf(stderr, "could not change protection for stack segment\n");
        return 1;
    }

    if (mach_vm_allocate(task, &code, sizeof(shell_code), VM_FLAGS_ANYWHERE) != KERN_SUCCESS) {
        fprintf(stderr, "could not allocate code segment\n");
        return 1;
    }

#ifdef __x86_64__
    uint64_t pcfmt_address = (uint64_t) dlsym(RTLD_DEFAULT, "pthread_create_from_mach_thread");
    uint64_t dlopen_address = (uint64_t) dlsym(RTLD_DEFAULT, "dlopen");

    memcpy(shell_code + 28, &pcfmt_address, sizeof(uint64_t));
    memcpy(shell_code + 71, &dlopen_address, sizeof(uint64_t));
    memcpy(shell_code + 90, payload_path, strlen(payload_path));
#elif __arm64__
    uint64_t pcfmt_address = (uint64_t) ptrauth_strip(dlsym(RTLD_DEFAULT, "pthread_create_from_mach_thread"), ptrauth_key_function_pointer);
    uint64_t dlopen_address = (uint64_t) ptrauth_strip(dlsym(RTLD_DEFAULT, "dlopen"), ptrauth_key_function_pointer);

    memcpy(shell_code + 88, &pcfmt_address, sizeof(uint64_t));
    memcpy(shell_code + 160, &dlopen_address, sizeof(uint64_t));
    memcpy(shell_code + 168, payload_path, strlen(payload_path));
#endif

    if (mach_vm_write(task, code, (vm_address_t) shell_code, sizeof(shell_code)) != KERN_SUCCESS) {
        fprintf(stderr, "could not copy shellcode into code segment\n");
        return 1;
    }

    if (vm_protect(task, code, sizeof(shell_code), 0, VM_PROT_EXECUTE | VM_PROT_READ) != KERN_SUCCESS) {
        fprintf(stderr, "could not change protection for code segment\n");
        return 1;
    }

#ifdef __x86_64__
    x86_thread_state64_t thread_state = {};
    thread_state_flavor_t thread_flavor = x86_THREAD_STATE64;
    mach_msg_type_number_t thread_flavor_count = x86_THREAD_STATE64_COUNT;

    thread_state.__rip = (uint64_t) code;
    thread_state.__rsp = (uint64_t) stack + (stack_size / 2);

    kern_return_t error = thread_create_running(task, thread_flavor, (thread_state_t)&thread_state, thread_flavor_count, &thread);
    if (error != KERN_SUCCESS) {
        fprintf(stderr, "could not spawn remote thread: %s\n", mach_error_string(error));
        return 1;
    }
#elif __arm64__
    void *handle = dlopen("/usr/lib/system/libsystem_kernel.dylib", RTLD_GLOBAL | RTLD_LAZY);
    if (handle) {
        _thread_convert_thread_state = dlsym(handle, "thread_convert_thread_state");
        dlclose(handle);
    }

    if (!_thread_convert_thread_state) {
        fprintf(stderr, "could not load symbol: thread_convert_thread_state\n");
        return 1;
    }

    struct arm_unified_thread_state thread_state = {};
    struct arm_unified_thread_state machine_thread_state = {};

    thread_state_flavor_t thread_flavor = ARM_UNIFIED_THREAD_STATE;
    mach_msg_type_number_t thread_flavor_count = ARM_UNIFIED_THREAD_STATE_COUNT;
    mach_msg_type_number_t machine_thread_flavor_count = ARM_UNIFIED_THREAD_STATE_COUNT;

    thread_state.ash.flavor = ARM_THREAD_STATE64;
    thread_state.ash.count = ARM_THREAD_STATE64_COUNT;

    __darwin_arm_thread_state64_set_pc_fptr(thread_state.ts_64, ptrauth_sign_unauthenticated((void *) code, ptrauth_key_asia, 0));
    __darwin_arm_thread_state64_set_sp(thread_state.ts_64, stack + (stack_size / 2));

    kern_return_t error = thread_create(task, &thread);
    if (error != KERN_SUCCESS) {
        fprintf(stderr, "could not create remote thread: %s\n", mach_error_string(error));
        return 1;
    }

    error = _thread_convert_thread_state(thread, 2, thread_flavor, (thread_state_t) &thread_state, thread_flavor_count, (thread_state_t) &machine_thread_state, &machine_thread_flavor_count);
    if (error != KERN_SUCCESS) {
        fprintf(stderr, "could not convert thread state: %s\n", mach_error_string(error));
        return 1;
    }

    error = thread_set_state(thread, thread_flavor, (thread_state_t)&machine_thread_state, machine_thread_flavor_count);
    if (error != KERN_SUCCESS) {
        fprintf(stderr, "could not set thread state: %s\n", mach_error_string(error));
        return 1;
    }

    error = thread_resume(thread);
    if (error != KERN_SUCCESS) {
        fprintf(stderr, "could not resume remote thread: %s\n", mach_error_string(error));
        return 1;
    }
#endif

    usleep(10000);

    for (int i = 0; i < 10; ++i) {
        kern_return_t error = thread_get_state(thread, thread_flavor, (thread_state_t)&thread_state, &thread_flavor_count);

        if (error != KERN_SUCCESS) {
            result = 1;
            goto terminate;
        }

#ifdef __x86_64__
        if (thread_state.__rax == 0x79616265) {
#elif __arm64__
        if (thread_state.ts_64.__x[0] == 0x79616265) {
#endif
            result = 0;
            goto terminate;
        }

        usleep(20000);
    }

terminate:
    error = thread_terminate(thread);
    if (error != KERN_SUCCESS) {
        fprintf(stderr, "failed to terminate remote thread: %s\n", mach_error_string(error));
    }

    return result;
}

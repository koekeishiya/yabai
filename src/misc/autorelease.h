#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#include <execinfo.h>

IMP g_nsobject_autorelease;
IMP g_nsautoreleasepool_drain;
IMP g_nsautoreleasepool_release;

@implementation NSObject(swizzle)
- (NSObject *)fake_autorelease
{
    void *addr[40];
    int frame_count = backtrace(addr, sizeof(addr)/sizeof(*addr));
    if (frame_count > 1) {
        char **syms = backtrace_symbols(addr, frame_count);
        for (int f = 0; f < frame_count; ++f) {
            printf("%s caller: %s\n", __FUNCTION__, syms[f]);
        }
        printf("\n");
        free(syms);
    } else {
        printf("%s: *** Failed to generate backtrace.", __FUNCTION__);
    }

    ((void(*)(id))g_nsobject_autorelease)(self);

    return self;
}
@end

@implementation NSAutoreleasePool(swizzle)
- (void)fake_drain
{
    void *addr[40];
    int frame_count = backtrace(addr, sizeof(addr)/sizeof(*addr));
    if (frame_count > 1) {
        char **syms = backtrace_symbols(addr, frame_count);
        for (int f = 0; f < frame_count; ++f) {
            printf("%s caller: %s\n", __FUNCTION__, syms[f]);
        }
        printf("\n");
        free(syms);
    } else {
        printf("%s: *** Failed to generate backtrace.", __FUNCTION__);
    }

    ((void(*)(id))g_nsautoreleasepool_drain)(self);
}

- (void)fake_release
{
    void *addr[40];
    int frame_count = backtrace(addr, sizeof(addr)/sizeof(*addr));
    if (frame_count > 1) {
        char **syms = backtrace_symbols(addr, frame_count);
        for (int f = 0; f < frame_count; ++f) {
            printf("%s caller: %s\n", __FUNCTION__, syms[f]);
        }
        printf("\n");
        free(syms);
    } else {
        printf("%s: *** Failed to generate backtrace.", __FUNCTION__);
    }

    ((void(*)(id))g_nsautoreleasepool_release)(self);
}
@end

static bool hook_nsobject_autorelease(void)
{
    Class c = objc_getClass("NSObject");
    if (!c) return false;

    Method m = class_getInstanceMethod(c, @selector(autorelease));
    g_nsobject_autorelease = method_setImplementation(m, (IMP)method_getImplementation(class_getInstanceMethod(c, @selector(fake_autorelease))));

    return true;
}

static bool hook_autoreleasepool_drain(void)
{
    Class c = objc_getClass("NSAutoreleasePool");
    if (!c) return false;

    Method m = class_getInstanceMethod(c, @selector(drain));
    g_nsautoreleasepool_drain = method_setImplementation(m, (IMP)method_getImplementation(class_getInstanceMethod(c, @selector(fake_drain))));

    return true;
}

static bool hook_autoreleasepool_release(void)
{
    Class c = objc_getClass("NSAutoreleasePool");
    if (!c) return false;

    Method m = class_getInstanceMethod(c, @selector(release));
    g_nsautoreleasepool_release = method_setImplementation(m, (IMP)method_getImplementation(class_getInstanceMethod(c, @selector(fake_release))));

    return true;
}

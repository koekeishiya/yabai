#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#include <execinfo.h>

IMP g_nsobject_autorelease;
IMP g_nsautoreleasepool_drain;

@implementation NSObject(swizzle)
- (NSObject *)fake_autorelease
{
    void *addr[20];
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
    void *addr[20];
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
@end

static bool hook_autorelease(void)
{
    Class c = objc_getClass("NSObject");
    if (!c) return false;

    Method m = class_getInstanceMethod(c, @selector(autorelease));
    g_nsobject_autorelease = method_setImplementation(m, (IMP)method_getImplementation(class_getInstanceMethod(c, @selector(fake_autorelease))));

    Class c2 = objc_getClass("NSAutoreleasePool");
    if (!c2) return false;

    Method m2 = class_getInstanceMethod(c2, @selector(drain));
    g_nsautoreleasepool_drain = method_setImplementation(m2, (IMP)method_getImplementation(class_getInstanceMethod(c2, @selector(fake_drain))));

    return true;
}

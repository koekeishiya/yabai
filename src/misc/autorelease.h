#import <Foundation/Foundation.h>
#import <objc/runtime.h>

IMP g_nsobject_autorelease;

@implementation NSObject(swizzle)
- (NSObject *)fake_autorelease
{
    NSLog(@"autorelease: %@", [self class]);

    ((void(*)(id))g_nsobject_autorelease)(self);

    return self;
}
@end

static bool hook_autorelease(void)
{
    Class c = objc_getClass("NSObject");
    if (!c) return false;

    Method m = class_getInstanceMethod(c, @selector(autorelease));
    g_nsobject_autorelease = method_setImplementation(m, (IMP)method_getImplementation(class_getInstanceMethod(c, @selector(fake_autorelease))));

    return true;
}

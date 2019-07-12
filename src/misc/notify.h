#import <Foundation/Foundation.h>
#import <objc/runtime.h>

static bool g_notify_init;
static NSImage *g_notify_img;

@implementation NSBundle(swizzle)
- (NSString *)fake_bundleIdentifier
{
    if (self == [NSBundle mainBundle]) {
        return @"com.koekeishiya.yabai";
    } else {
        return [self fake_bundleIdentifier];
    }
}
@end

static bool notify_init(void)
{
    Class c = objc_getClass("NSBundle");
    if (!c) return false;

    method_exchangeImplementations(class_getInstanceMethod(c, @selector(bundleIdentifier)), class_getInstanceMethod(c, @selector(fake_bundleIdentifier)));
    g_notify_img = [[[NSWorkspace sharedWorkspace] iconForFile:[[[NSBundle mainBundle] executablePath] stringByResolvingSymlinksInPath]] retain];
    g_notify_init = true;

    return true;
}

static void notify(char *message, char *subtitle)
{
    @autoreleasepool {

    if (!g_notify_init) notify_init();

    NSUserNotification *notification = [[NSUserNotification alloc] init];
    notification.title = @"yabai";
    notification.subtitle = subtitle ? [NSString stringWithUTF8String:subtitle] : NULL;
    notification.informativeText = message ? [NSString stringWithUTF8String:message] : NULL;
    [notification setValue:g_notify_img forKey:@"_identityImage"];
    [notification setValue:@(false) forKey:@"_identityImageHasBorder"];
    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
    [notification release];

    }
}

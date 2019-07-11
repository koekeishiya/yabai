#import <Foundation/Foundation.h>
#import <objc/runtime.h>

@implementation NSBundle(swizzle)
- (NSString *)fake_bundleIdentifier
{
    if (self == [NSBundle mainBundle]) {
        return @"com.apple.dock";
    } else {
        return [self fake_bundleIdentifier];
    }
}
@end

static bool g_notify_init;
static NSImage *g_notify_img;

static bool notify_init(void)
{
    Class c = objc_getClass("NSBundle");
    if (!c) return false;

    method_exchangeImplementations(class_getInstanceMethod(c, @selector(bundleIdentifier)), class_getInstanceMethod(c, @selector(fake_bundleIdentifier)));
    g_notify_img = [[NSWorkspace sharedWorkspace] iconForFile:[[[NSBundle mainBundle] executablePath] stringByResolvingSymlinksInPath]];
    g_notify_init = true;

    return true;
}

static void notify(char *message, char *subtitle)
{
    if (!g_notify_init) notify_init();

    NSUserNotification *notification = [[NSUserNotification alloc] init];
    notification.title = @"yabai";
    notification.subtitle = subtitle ? [NSString stringWithUTF8String:subtitle] : NULL;
    notification.informativeText = message ? [NSString stringWithUTF8String:message] : NULL;

    if (g_notify_img) {
        [notification setValue:g_notify_img forKey:@"_identityImage"];
        [notification setValue:@(false) forKey:@"_identityImageHasBorder"];
    }

    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
    [notification release];
}

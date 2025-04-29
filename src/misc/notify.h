#import <Foundation/Foundation.h>
#import <objc/runtime.h>

static bool g_notify_init;
static NSImage *g_notify_img;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

@interface NotifyDelegate : NSObject <NSUserNotificationCenterDelegate>
@end

@implementation NotifyDelegate
- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
  return YES;
}
@end

static bool notify_init(void)
{
  [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:[NotifyDelegate alloc]];
  g_notify_img = [[[NSWorkspace sharedWorkspace] iconForFile:[[[NSBundle mainBundle] executablePath] stringByResolvingSymlinksInPath]] retain];
  g_notify_init = true;

  return true;
}

static void notify(const char *subtitle, const char *format, ...)
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  if (!g_notify_init)
    notify_init();

  va_list args;
  va_start(args, format);
  NSUserNotification *notification = [[NSUserNotification alloc] init];
  notification.title = @"nimbuswm";
  notification.subtitle = [NSString stringWithUTF8String:subtitle];
  notification.informativeText = [[[NSString alloc] initWithFormat:[NSString stringWithUTF8String:format] arguments:args] autorelease];
  [notification setValue:g_notify_img forKey:@"_identityImage"];
  [notification setValue:@(false) forKey:@"_identityImageHasBorder"];
  [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
  [notification release];
  va_end(args);

  [pool drain];
}

#pragma clang diagnostic pop

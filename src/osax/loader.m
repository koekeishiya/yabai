#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

@interface Loader : NSObject
@end

@implementation Loader
@end

static Class _instance;

OSErr yabai_osax_load(const AppleEvent *event, AppleEvent *reply, long context)
{
    NSLog(@"[yabai] attempting to load scripting addition..");

    OSErr result = noErr;
    NSBundle* loader_bundle = [NSBundle bundleForClass:[Loader class]];
    NSString *payload_path = [loader_bundle pathForResource:@"payload" ofType:@"bundle"];
    NSBundle *payload_bundle = [NSBundle bundleWithPath:payload_path];

    if (!payload_bundle) {
        NSLog(@"[yabai] could not locate payload!");
        result = 2;
        goto end;
    }

    if ([payload_bundle isLoaded]) {
        NSLog(@"[yabai] payload has already been loaded!");
        result = 2;
        goto end;
    }

    if (![payload_bundle load]) {
        NSLog(@"[yabai] could not load payload!");
        result = 2;
        goto end;
    }

    _instance = [payload_bundle principalClass];

end:
    if (result == noErr) {
        NSLog(@"[yabai] scripting addition loaded successfully..");
    } else {
        NSLog(@"[yabai] failed to load scriping addition!");
    }

    return result;
}

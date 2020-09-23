#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import "common.h"

static Class _instance;
OSErr yabai_osax_load(const AppleEvent *event, AppleEvent *reply, long context)
{
    NSLog(@"[yabai] attempting to load scripting addition..");

    OSErr result = OSAX_PAYLOAD_SUCCESS;
    NSBundle *loader_bundle = [NSBundle bundleWithIdentifier:@"com.koekeishiya.yabai-osax"];
    NSLog(@"[yabai] loader_bundle: %@", loader_bundle);

    NSString *payload_path = [loader_bundle pathForResource:@"payload" ofType:@"bundle"];
    NSLog(@"[yabai] payload_path: %@", payload_path);

    NSBundle *payload_bundle = [NSBundle bundleWithPath:payload_path];
    NSLog(@"[yabai] payload_bundle: %@", payload_bundle);

    if (!payload_bundle) {
        NSLog(@"[yabai] could not locate payload!");
        result = OSAX_PAYLOAD_NOT_FOUND;
        goto end;
    }

    if ([payload_bundle isLoaded]) {
        NSLog(@"[yabai] payload has already been loaded!");
        result = OSAX_PAYLOAD_ALREADY_LOADED;
        goto end;
    }

    if (![payload_bundle load]) {
        NSLog(@"[yabai] could not load payload!");
        result = OSAX_PAYLOAD_NOT_LOADED;
        goto end;
    }

    _instance = [payload_bundle principalClass];
    NSLog(@"[yabai] set class instance: %@", _instance);

end:
    if (result == OSAX_PAYLOAD_SUCCESS) {
        NSLog(@"[yabai] scripting addition loaded successfully..");
    } else {
        NSLog(@"[yabai] failed to load scriping addition!");
    }

    return result;
}

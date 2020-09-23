#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import "common.h"
#include <dlfcn.h>

static bool _loaded;
OSErr yabai_osax_load(const AppleEvent *event, AppleEvent *reply, long context)
{
    NSLog(@"[yabai] attempting to load scripting addition..");

    OSErr result = OSAX_PAYLOAD_SUCCESS;
    NSBundle *loader_bundle = [NSBundle bundleWithIdentifier:@"com.koekeishiya.yabai-osax"];
    NSString *payload_path = [loader_bundle pathForResource:@"payload" ofType:@"bundle"];
    NSBundle *payload_bundle = [NSBundle bundleWithPath:payload_path];

    NSLog(@"[yabai] loader_bundle: %@", loader_bundle);
    NSLog(@"[yabai] payload_path: %@", payload_path);
    NSLog(@"[yabai] payload_bundle: %@", payload_bundle);

    if (_loaded) {
        NSLog(@"[yabai] payload has already been loaded!");
        result = OSAX_PAYLOAD_ALREADY_LOADED;
        goto end;
    }

    if (!payload_bundle) {
        NSLog(@"[yabai] could not locate payload!");
        result = OSAX_PAYLOAD_NOT_FOUND;
        goto end;
    }

    void *payload_handle = dlopen([[payload_bundle executablePath] UTF8String], RTLD_NOW);
    if (!payload_handle) {
        NSLog(@"[yabai] could not locate payload!");
        result = OSAX_PAYLOAD_NOT_FOUND;
        goto end;
    }

    void *payload_load = dlsym(payload_handle, "load_payload");
    if (!payload_load) {
        NSLog(@"[yabai] could not load payload!");
        result = OSAX_PAYLOAD_NOT_LOADED;
        goto end;
    }

    _loaded = true;
    ((void(*)())payload_load)();
    dlclose(payload_handle);

end:
    if (result == OSAX_PAYLOAD_SUCCESS) {
        NSLog(@"[yabai] scripting addition loaded successfully..");
    } else {
        NSLog(@"[yabai] failed to load scriping addition!");
    }

    return result;
}

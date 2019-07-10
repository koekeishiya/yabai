#include "sa.h"

extern char g_sa_socket_file[MAXLEN];

@interface SALoader : NSObject<SBApplicationDelegate> {}
- (void) eventDidFail:(const AppleEvent*)event withError:(NSError*)error;
@end

@implementation SALoader { @public int result; }
- (void) eventDidFail:(const AppleEvent*)event withError:(NSError*)error
{
    NSNumber *errorNumber = [[error userInfo] objectForKey:@"ErrorNumber"];
    result = [errorNumber intValue];
}
@end

static char osax_base_dir[MAXLEN];
static char osax_contents_dir[MAXLEN];
static char osax_contents_macos_dir[MAXLEN];
static char osax_contents_res_dir[MAXLEN];
static char osax_payload_dir[MAXLEN];
static char osax_payload_contents_dir[MAXLEN];
static char osax_payload_contents_macos_dir[MAXLEN];
static char osax_info_plist[MAXLEN];
static char osax_sdefn_file[MAXLEN];
static char osax_payload_plist[MAXLEN];
static char osax_bin_loader[MAXLEN];
static char osax_bin_payload[MAXLEN];

static char sa_def[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<!DOCTYPE dictionary SYSTEM \"file://localhost/System/Library/DTDs/sdef.dtd\">\n"
    "<dictionary title=\"Yabai Terminology\">\n"
    "<suite name=\"Yabai Suite\" code=\"YBSA\" description=\"Yabai Scripting Addition\">\n"
    "<command name=\"init Yabai\" code=\"YBSAload\" description=\"Install payload into the process\"/>\n"
    "</suite>\n"
    "</dictionary>";

static char sa_plist[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
    "<plist version=\"1.0\">\n"
    "<dict>\n"
    "<key>CFBundleDevelopmentRegion</key>\n"
    "<string>en</string>\n"
    "<key>CFBundleExecutable</key>\n"
    "<string>loader</string>\n"
    "<key>CFBundleIdentifier</key>\n"
    "<string>com.koekeishiya.yabai-osax</string>\n"
    "<key>CFBundleInfoDictionaryVersion</key>\n"
    "<string>6.0</string>\n"
    "<key>CFBundleName</key>\n"
    "<string>yabai</string>\n"
    "<key>CFBundlePackageType</key>\n"
    "<string>osax</string>\n"
    "<key>CFBundleShortVersionString</key>\n"
    "<string>1.0</string>\n"
    "<key>CFBundleVersion</key>\n"
    "<string>1.0</string>\n"
    "<key>NSHumanReadableCopyright</key>\n"
    "<string>Copyright © 2019 Åsmund Vikane. All rights reserved.</string>\n"
    "<key>OSAScriptingDefinition</key>\n"
    "<string>yabai.sdef</string>\n"
    "<key>OSAXHandlers</key>\n"
    "<dict>\n"
    "<key>Events</key>\n"
    "<dict>\n"
    "<key>YBSAload</key>\n"
    "<dict>\n"
    "<key>Context</key>\n"
    "<string>Process</string>\n"
    "<key>Handler</key>\n"
    "<string>yabai_osax_load</string>\n"
    "<key>ThreadSafe</key>\n"
    "<false/>\n"
    "</dict>\n"
    "</dict>\n"
    "</dict>\n"
    "</dict>\n"
    "</plist>";

static char sa_bundle_plist[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
    "<plist version=\"1.0\">\n"
    "<dict>\n"
    "<key>CFBundleDevelopmentRegion</key>\n"
    "<string>en</string>\n"
    "<key>CFBundleExecutable</key>\n"
    "<string>payload</string>\n"
    "<key>CFBundleIdentifier</key>\n"
    "<string>com.koekeishiya.yabai-sa</string>\n"
    "<key>CFBundleInfoDictionaryVersion</key>\n"
    "<string>6.0</string>\n"
    "<key>CFBundleName</key>\n"
    "<string>payload</string>\n"
    "<key>CFBundlePackageType</key>\n"
    "<string>BNDL</string>\n"
    "<key>CFBundleShortVersionString</key>\n"
    "<string>1.0</string>\n"
    "<key>CFBundleVersion</key>\n"
    "<string>1</string>\n"
    "<key>NSHumanReadableCopyright</key>\n"
    "<string>Copyright © 2019 Åsmund Vikane. All rights reserved.</string>\n"
    "<key>NSPrincipalClass</key>\n"
    "<string></string>\n"
    "</dict>\n"
    "</plist>";

static void scripting_addition_set_path(void)
{
    NSOperatingSystemVersion os_version = [[NSProcessInfo processInfo] operatingSystemVersion];
    if (os_version.minorVersion >= 14) {
        snprintf(osax_base_dir, sizeof(osax_base_dir), "%s", "/Library/ScriptingAdditions/yabai.osax");
    } else {
        snprintf(osax_base_dir, sizeof(osax_base_dir), "%s", "/System/Library/ScriptingAdditions/yabai.osax");
    }

    snprintf(osax_contents_dir, sizeof(osax_contents_dir), "%s/%s", osax_base_dir, "Contents");
    snprintf(osax_contents_macos_dir, sizeof(osax_contents_macos_dir), "%s/%s", osax_contents_dir, "MacOS");
    snprintf(osax_contents_res_dir, sizeof(osax_contents_res_dir), "%s/%s", osax_contents_dir, "Resources");

    snprintf(osax_payload_dir, sizeof(osax_payload_dir), "%s/%s", osax_contents_res_dir, "payload.bundle");
    snprintf(osax_payload_contents_dir, sizeof(osax_payload_contents_dir), "%s/%s", osax_payload_dir, "Contents");
    snprintf(osax_payload_contents_macos_dir, sizeof(osax_payload_contents_macos_dir), "%s/%s", osax_payload_contents_dir, "MacOS");

    snprintf(osax_info_plist, sizeof(osax_info_plist), "%s/%s", osax_contents_dir, "Info.plist");
    snprintf(osax_sdefn_file, sizeof(osax_sdefn_file), "%s/%s", osax_contents_res_dir, "yabai.sdef");

    snprintf(osax_payload_plist, sizeof(osax_payload_plist), "%s/%s", osax_payload_contents_dir, "Info.plist");
    snprintf(osax_bin_loader, sizeof(osax_bin_loader), "%s/%s", osax_contents_macos_dir, "loader");
    snprintf(osax_bin_payload, sizeof(osax_bin_payload), "%s/%s", osax_payload_contents_macos_dir, "payload");
}

static bool scripting_addition_create_directory(void)
{
    if (mkdir(osax_base_dir, 0755))                   goto err;
    if (mkdir(osax_contents_dir, 0755))               goto err;
    if (mkdir(osax_contents_macos_dir, 0755))         goto err;
    if (mkdir(osax_contents_res_dir, 0755))           goto err;
    if (mkdir(osax_payload_dir, 0755))                goto err;
    if (mkdir(osax_payload_contents_dir, 0755))       goto err;
    if (mkdir(osax_payload_contents_macos_dir, 0755)) goto err;
    return true;
err:
    return false;
}

static bool scripting_addition_write_file(char *buffer, unsigned int size, char *file, char *file_mode)
{
    FILE *handle = fopen(file, file_mode);
    if (!handle) return false;

    size_t bytes = fwrite(buffer, size, 1, handle);
    bool result = bytes == 1;
    fclose(handle);

    return result;
}

static void scripting_addition_prepare_binaries(void)
{
    char cmd[MAXLEN];

    snprintf(cmd, sizeof(cmd), "%s %s", "chmod +x", osax_bin_loader);
    system(cmd);

    snprintf(cmd, sizeof(cmd), "%s %s %s", "codesign -f -s -", osax_bin_loader, "2>/dev/null");
    system(cmd);

    snprintf(cmd, sizeof(cmd), "%s %s", "chmod +x", osax_bin_payload);
    system(cmd);

    snprintf(cmd, sizeof(cmd), "%s %s %s", "codesign -f -s -", osax_bin_payload, "2>/dev/null");
    system(cmd);
}

static char *scripting_addition_request_handshake(uint32_t *flags)
{
    char *result = NULL;

    int sockfd;
    char message[MAXLEN];

    if (socket_connect_un(&sockfd, g_sa_socket_file)) {
        snprintf(message, sizeof(message), "handshake");
        if (socket_write(sockfd, message)) {
            int length;
            result = socket_read(sockfd, &length);
            if (!result) goto out;

            char *attrib = result + 6;
            assert(attrib[-1] == '\0');
            memcpy(flags, attrib, sizeof(uint32_t));
        }
    }

out:
    socket_close(sockfd);
    return result;
}

static void scripting_addition_perform_validation(void)
{
    uint32_t attrib;
    char *version = scripting_addition_request_handshake(&attrib);
    if (version) debug("yabai: osax version = %s, osax attrib = 0x%X\n", version, attrib);

    if (!version || !string_equals(version, OSAX_VERSION)) {
        warn("yabai: scripting-addition payload is outdated, please reinstall!\n");
    } else if ((attrib & OSAX_ATTRIB_ALL) != OSAX_ATTRIB_ALL) {
        warn("yabai: scripting-addition payload failed to locate required resources inside Dock.app!\n");
    } else {
        debug("yabai: scripting-addition payload successfully located all requsted resources inside Dock.app..\n");
    }
}

bool scripting_addition_is_installed(void)
{
    if (osax_base_dir[0] == 0) scripting_addition_set_path();

    DIR *dir = opendir(osax_base_dir);
    if (!dir) return false;

    closedir(dir);
    return true;
}

static bool scripting_addition_remove(void)
{
    char cmd[MAXLEN];
    snprintf(cmd, sizeof(cmd), "%s %s %s", "rm -rf", osax_base_dir, "2>/dev/null");

    int code = system(cmd);
    if (code ==  -1) return false;
    if (code == 127) return false;
    return code == 0;
}

int scripting_addition_install(void)
{
    if ((scripting_addition_is_installed()) &&
        (!scripting_addition_remove())) {
        return 1;
    }

    if (!scripting_addition_create_directory()) {
        goto cleanup;
    }

    if (!scripting_addition_write_file(sa_plist, strlen(sa_plist), osax_info_plist, "w")) {
        goto cleanup;
    }

    if (!scripting_addition_write_file(sa_def, strlen(sa_def), osax_sdefn_file, "w")) {
        goto cleanup;
    }

    if (!scripting_addition_write_file(sa_bundle_plist, strlen(sa_bundle_plist), osax_payload_plist, "w")) {
        goto cleanup;
    }

    if (!scripting_addition_write_file((char *) __src_osax_loader, __src_osax_loader_len, osax_bin_loader, "wb")) {
        goto cleanup;
    }

    if (!scripting_addition_write_file((char *) __src_osax_payload, __src_osax_payload_len, osax_bin_payload, "wb")) {
        goto cleanup;
    }

    scripting_addition_prepare_binaries();
    return 0;

cleanup:
    scripting_addition_remove();
    return 2;
}

int scripting_addition_uninstall(void)
{
    if (!scripting_addition_is_installed()) return  0;
    if (!scripting_addition_remove())       return -1;
    return 0;
}

int scripting_addition_load(void)
{
    if (!scripting_addition_is_installed()) return 1;

    SALoader *loader = [[SALoader alloc] init];
    loader->result = OSAX_PAYLOAD_SUCCESS;

    // temporarily redirect stderr to /dev/null to silence
    // meaningless warning reported by the scripting-bridge
    // framework, because Dock.app does not provide a .sdef

    int stderr_fd = dup(2);
    int null_fd = open("/dev/null", O_WRONLY);
    fflush(stderr);
    dup2(null_fd, 2);
    close(null_fd);

    // @memory_leak
    // [SBApplication applicationWithBundleIdentifier] leaks memory and there is nothing we
    // can do about it.. So much for all the automatic memory management techniques in objc

    SBApplication *dock = [SBApplication applicationWithBundleIdentifier:@"com.apple.Dock"];
    [dock setTimeout:10*60];
    [dock setSendMode:kAEWaitReply];
    [dock sendEvent:'ascr' id:'gdut' parameters:0];
    [dock setDelegate:loader];
    [dock sendEvent:'YBSA' id:'load' parameters:0];
    [dock release];

    //
    // restore stderr back to normal
    //

    fflush(stderr);
    dup2(stderr_fd, 2);
    close(stderr_fd);

    if (loader->result == OSAX_PAYLOAD_NOT_FOUND) {
        warn("yabai: scripting-addition was located, but is not valid - the payload could not be found!\n");
    } else if (loader->result == OSAX_PAYLOAD_NOT_LOADED) {
        warn("yabai: scripting-addition was located, but failed to inject payload into Dock.app!\n");
    } else if (loader->result == OSAX_PAYLOAD_UNAUTHORIZED) {
        warn("yabai: scripting-addition could not inject payload into Dock.app, make sure SIP is disabled!\n");
    } else if (loader->result == OSAX_PAYLOAD_ALREADY_LOADED) {
        warn("yabai: scripting-addition has previously injected the payload into Dock.app!\n");
        scripting_addition_perform_validation();
    } else if (loader->result == OSAX_PAYLOAD_SUCCESS) {
        debug("yabai: scripting-addition successfully injected payload into Dock.app..\n");
        scripting_addition_perform_validation();
    } else {
        warn("yabai: scripting-addition could not inject payload into Dock.app due to unknown error (%d)!\n", loader->result);
    }

    [loader release];
    return 0;
}

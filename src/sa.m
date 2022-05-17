#include "sa.h"

extern int csr_get_active_config(uint32_t *config);
#define CSR_ALLOW_UNRESTRICTED_FS 0x02
#define CSR_ALLOW_TASK_FOR_PID    0x04

#define SA_SOCKET_PATH_FMT      "/tmp/yabai-sa_%s.socket"
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
static char osax_bin_mach_loader[MAXLEN];

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
    "<string>"OSAX_VERSION"</string>\n"
    "<key>CFBundleVersion</key>\n"
    "<string>"OSAX_VERSION"</string>\n"
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
    "<string>"OSAX_VERSION"</string>\n"
    "<key>CFBundleVersion</key>\n"
    "<string>"OSAX_VERSION"</string>\n"
    "<key>NSHumanReadableCopyright</key>\n"
    "<string>Copyright © 2019 Åsmund Vikane. All rights reserved.</string>\n"
    "<key>NSPrincipalClass</key>\n"
    "<string></string>\n"
    "</dict>\n"
    "</plist>";

static void scripting_addition_set_path(void)
{
    NSOperatingSystemVersion os_version = [[NSProcessInfo processInfo] operatingSystemVersion];
    if (os_version.majorVersion >= 11 || os_version.minorVersion >= 14) {
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
    snprintf(osax_bin_mach_loader, sizeof(osax_bin_mach_loader), "%s/%s", osax_contents_macos_dir, "mach_loader");
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

    snprintf(cmd, sizeof(cmd), "%s %s", "chmod +x", osax_bin_mach_loader);
    system(cmd);

    snprintf(cmd, sizeof(cmd), "%s %s %s", "codesign -f -s -", osax_bin_mach_loader, "2>/dev/null");
    system(cmd);

    snprintf(cmd, sizeof(cmd), "%s %s", "chmod +x", osax_bin_payload);
    system(cmd);

    snprintf(cmd, sizeof(cmd), "%s %s %s", "codesign -f -s -", osax_bin_payload, "2>/dev/null");
    system(cmd);
}

static bool scripting_addition_request_handshake(char *version, uint32_t *attrib)
{
    int sockfd;
    bool result = false;
    char rsp[BUFSIZ] = {};

    if (socket_open(&sockfd)) {
        if (socket_connect(sockfd, g_sa_socket_file)) {
            if (send(sockfd, "handshake", 9, 0) != -1) {
                int length = recv(sockfd, rsp, sizeof(rsp)-1, 0);
                if (length <= 0) goto out;

                char *zero = rsp;
                while (*zero != '\0') ++zero;

                assert(*zero == '\0');
                memcpy(version, rsp, zero - rsp + 1);
                memcpy(attrib, zero+1, sizeof(uint32_t));

                result = true;
            }
        }

out:
        socket_close(sockfd);
    }

    return result;
}

static int scripting_addition_perform_validation(bool loaded)
{
    uint32_t attrib = 0;
    char version[MAXLEN] = {};

    if (!scripting_addition_request_handshake(version, &attrib)) {
        notify("scripting-addition", "connection failed!");
        return PAYLOAD_STATUS_CON_ERROR;
    }

    debug("yabai: osax version = %s, osax attrib = 0x%X\n", version, attrib);
    bool is_latest_version_installed = scripting_addition_check() == 0;

    if (!string_equals(version, OSAX_VERSION)) {
        if (loaded && is_latest_version_installed) {
            notify("scripting-addition", "payload is outdated, restart Dock.app!");
        } else {
            notify("scripting-addition", "payload is outdated, please reinstall!");
        }

        return PAYLOAD_STATUS_OUTDATED;
    } else if ((attrib & OSAX_ATTRIB_ALL) != OSAX_ATTRIB_ALL) {
        notify("scripting-addition", "payload (0x%X) doesn't support this macOS version!", attrib);
        return PAYLOAD_STATUS_NO_ATTRIB;
    } else {
        notify("scripting-addition", "payload v%s", version);
        return PAYLOAD_STATUS_SUCCESS;
    }
}

static void scripting_addition_restart_dock(void)
{
    NSArray *dock = [NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.apple.dock"];
    [dock makeObjectsPerformSelector:@selector(terminate)];
}

static bool scripting_addition_is_sip_friendly(void)
{
    uint32_t config = 0;
    csr_get_active_config(&config);

    if (!(config & CSR_ALLOW_UNRESTRICTED_FS)) {
        warn("yabai: System Integrity Protection: Filesystem Protections must be disabled!\n");
        notify("scripting-addition", "System Integrity Protection: Filesystem Protections must be disabled!");
        return false;
    }

    if (!(config & CSR_ALLOW_TASK_FOR_PID)) {
        warn("yabai: System Integrity Protection: Debugging Restrictions must be disabled!\n");
        notify("scripting-addition", "System Integrity Protection: Debugging Restrictions must be disabled!");
        return false;
    }

    return true;
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
    if (!scripting_addition_is_sip_friendly()) {
        return 1;
    }

    if (!is_root()) {
        warn("yabai: scripting-addition must be installed as root!\n");
        notify("scripting-addition", "must be installed as root!");
        return 1;
    }

    umask(S_IWGRP | S_IWOTH);

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

    if (!scripting_addition_write_file((char *) __src_osax_mach_loader, __src_osax_mach_loader_len, osax_bin_mach_loader, "wb")) {
        goto cleanup;
    }

    if (!scripting_addition_write_file((char *) __src_osax_payload, __src_osax_payload_len, osax_bin_payload, "wb")) {
        goto cleanup;
    }

    scripting_addition_prepare_binaries();
    scripting_addition_restart_dock();
    return 0;

cleanup:
    scripting_addition_remove();
    return 2;
}

int scripting_addition_uninstall(void)
{
    if (!scripting_addition_is_sip_friendly()) {
        return 1;
    }

    if (!is_root()) {
        warn("yabai: scripting-addition must be uninstalled as root!\n");
        notify("scripting-addition", "must be uninstalled as root!");
        return 1;
    }

    if (!scripting_addition_is_installed()) return  0;
    if (!scripting_addition_remove())       return -1;
    return 0;
}

int scripting_addition_check(void)
{
    bool result = 0;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    if (scripting_addition_is_installed()) {
        NSString *payload_path = [NSString stringWithUTF8String:osax_payload_dir];
        NSBundle *payload_bundle = [NSBundle bundleWithPath:payload_path];
        NSString *ns_version = [payload_bundle objectForInfoDictionaryKey:@"CFBundleVersion"];

        bool status = string_equals([ns_version UTF8String], OSAX_VERSION);
        result = status ? 0 : 1;
    } else {
        result = 1;
    }

    [pool drain];
    return result;
}

static bool drop_sudo_privileges_and_set_sa_socket_path(void)
{
    uid_t uid = getuid();
    assert(uid == 0);

    const char *sudo_uid = getenv("SUDO_UID");
    if (sudo_uid == NULL) return false;

    if (sscanf(sudo_uid, "%u", &uid) != 1) return false;
    if (setuid(uid) != 0) return false;

    struct passwd *pw = getpwuid(uid);
    if (!pw) return false;

    snprintf(g_sa_socket_file, sizeof(g_sa_socket_file), SA_SOCKET_PATH_FMT, pw->pw_name);
    return true;
}

static bool mach_loader_inject_payload(void)
{
    FILE *handle = popen("/Library/ScriptingAdditions/yabai.osax/Contents/MacOS/mach_loader", "r");
    if (!handle) return false;

    int result = pclose(handle);
    if (WIFEXITED(result)) {
        return WEXITSTATUS(result) == 0;
    } else if (WIFSIGNALED(result)) {
        return false;
    } else if (WIFSTOPPED(result)) {
        return false;
    }

    return false;
}

int scripting_addition_load(void)
{
    int result = 0;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    if (!scripting_addition_is_installed()) {
        result = 1;
        goto out;
    }

    if (!workspace_is_macos_highsierra() &&
        !scripting_addition_is_sip_friendly()) {
        result = 1;
        goto out;
    }

    if (workspace_is_macos_monterey() || workspace_is_macos_bigsur()) {
        if (!is_root()) {
            warn("yabai: scripting-addition must be loaded as root!\n");
            notify("scripting-addition", "must be loaded as root!");
            result = 1;
            goto out;
        }

        if (mach_loader_inject_payload()) {
            debug("yabai: scripting-addition successfully injected payload into Dock.app..\n");
            if (drop_sudo_privileges_and_set_sa_socket_path()) {
                scripting_addition_perform_validation(false);
            }
            result = 0;
            goto out;
        } else {
            warn("yabai: scripting-addition failed to inject payload into Dock.app!\n");
            notify("scripting-addition", "failed to inject payload into Dock.app!");
            result = 1;
            goto out;
        }
    } else {
        if (is_root()) {
            warn("yabai: scripting-addition should not be loaded as root!\n");
            notify("scripting-addition", "should not be loaded as root!");
            result = 1;
            goto out;
        }

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

        //
        // restore stderr back to normal
        //

        fflush(stderr);
        dup2(stderr_fd, 2);
        close(stderr_fd);

        int result = loader->result;
        [loader release];

        if (result == OSAX_PAYLOAD_SUCCESS) {
            debug("yabai: scripting-addition successfully injected payload into Dock.app..\n");
            scripting_addition_perform_validation(false);
            result = 0;
            goto out;
        } else if (result == OSAX_PAYLOAD_ALREADY_LOADED) {
            debug("yabai: scripting-addition payload was already injected into Dock.app!\n");
            scripting_addition_perform_validation(true);
            result = 0;
            goto out;
        } else if (result == OSAX_PAYLOAD_NOT_LOADED) {
            notify("scripting-addition", "failed to inject payload into Dock.app!");
            warn("yabai: scripting-addition failed to inject payload into Dock.app!\n");
            result = 1;
            goto out;
        } else if (result == OSAX_PAYLOAD_NOT_FOUND) {
            notify("scripting-addition", "payload could not be found!");
            warn("yabai: scripting-addition payload could not be found!\n");
            result = 1;
            goto out;
        } else {
            notify("scripting-addition", "failed to load or inject payload into Dock.app!");
            warn("yabai: scripting-addition either failed to load or could not inject payload into Dock.app! Error: %d\n", result);
            result = 1;
            goto out;
        }

        result = 0;
        goto out;
    }

out:
    [pool drain];
    return result;
}

static bool scripting_addition_run_command(char *message)
{
    int sockfd;
    char dummy;
    bool result = false;

    if (socket_open(&sockfd)) {
        if (socket_connect(sockfd, g_sa_socket_file)) {
            if (send(sockfd, message, strlen(message), 0) != -1) {
                recv(sockfd, &dummy, 1, 0);
                result = true;
            }
        }

        socket_close(sockfd);
    }

    return result;
}

bool scripting_addition_create_space(uint64_t sid)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "space_create %lld", sid);
    return scripting_addition_run_command(message);
}

bool scripting_addition_destroy_space(uint64_t sid)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "space_destroy %lld", sid);
    return scripting_addition_run_command(message);
}

bool scripting_addition_focus_space(uint64_t sid)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "space %lld", sid);
    return scripting_addition_run_command(message);
}

bool scripting_addition_move_space_after_space(uint64_t src_sid, uint64_t dst_sid, bool focus)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "space_move %lld %lld %d", src_sid, dst_sid, focus);
    return scripting_addition_run_command(message);
}

bool scripting_addition_move_window(uint32_t wid, int x, int y)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "window_move %d %d %d", wid, x, y);
    return scripting_addition_run_command(message);
}

bool scripting_addition_set_opacity(uint32_t wid, float opacity, float duration)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "window_alpha_fade %d %f %f", wid, opacity, duration);
    return scripting_addition_run_command(message);
}

bool scripting_addition_set_layer(uint32_t wid, int layer)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "window_level %d %d", wid, layer);
    return scripting_addition_run_command(message);
}

bool scripting_addition_set_sticky(uint32_t wid, bool sticky)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "window_sticky %d %d", wid, sticky);
    return scripting_addition_run_command(message);
}

bool scripting_addition_set_shadow(uint32_t wid, bool shadow)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "window_shadow %d %d", wid, shadow);
    return scripting_addition_run_command(message);
}

bool scripting_addition_focus_window(uint32_t wid)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "window_focus %d", wid);
    return scripting_addition_run_command(message);
}

bool scripting_addition_scale_window(uint32_t wid, float x, float y, float w, float h)
{
    char message[MAXLEN];
    snprintf(message, sizeof(message), "window_scale %d %f %f %f %f", wid, x, y, w, h);
    return scripting_addition_run_command(message);
}

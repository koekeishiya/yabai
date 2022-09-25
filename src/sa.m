#include "sa.h"

extern int csr_get_active_config(uint32_t *config);
#define CSR_ALLOW_UNRESTRICTED_FS 0x02
#define CSR_ALLOW_TASK_FOR_PID    0x04

#define SA_SOCKET_PATH_FMT "/tmp/yabai-sa_%s.socket"
extern char g_sa_socket_file[MAXLEN];

static char osax_base_dir[MAXLEN];
static char osax_contents_dir[MAXLEN];
static char osax_contents_macos_dir[MAXLEN];
static char osax_contents_res_dir[MAXLEN];
static char osax_info_plist[MAXLEN];
static char osax_payload_dir[MAXLEN];
static char osax_payload_contents_dir[MAXLEN];
static char osax_payload_contents_macos_dir[MAXLEN];
static char osax_payload_plist[MAXLEN];
static char osax_bin_payload[MAXLEN];
static char osax_bin_loader[MAXLEN];

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
    "<key>OSAXHandlers</key>\n"
    "<dict>\n"
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
    snprintf(osax_base_dir, sizeof(osax_base_dir), "%s", "/Library/ScriptingAdditions/yabai.osax");

    snprintf(osax_contents_dir, sizeof(osax_contents_dir), "%s/%s", osax_base_dir, "Contents");
    snprintf(osax_contents_macos_dir, sizeof(osax_contents_macos_dir), "%s/%s", osax_contents_dir, "MacOS");
    snprintf(osax_contents_res_dir, sizeof(osax_contents_res_dir), "%s/%s", osax_contents_dir, "Resources");
    snprintf(osax_info_plist, sizeof(osax_info_plist), "%s/%s", osax_contents_dir, "Info.plist");

    snprintf(osax_payload_dir, sizeof(osax_payload_dir), "%s/%s", osax_contents_res_dir, "payload.bundle");
    snprintf(osax_payload_contents_dir, sizeof(osax_payload_contents_dir), "%s/%s", osax_payload_dir, "Contents");
    snprintf(osax_payload_contents_macos_dir, sizeof(osax_payload_contents_macos_dir), "%s/%s", osax_payload_contents_dir, "MacOS");
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

static void scripting_addition_restart_dock(void)
{
    NSArray *dock = [NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.apple.dock"];
    [dock makeObjectsPerformSelector:@selector(terminate)];
}

static bool scripting_addition_set_socket_path(void)
{
    const char *sudo_uid = getenv("SUDO_UID");

    uid_t uid = getuid();
    assert(uid == 0);

    if (sudo_uid == NULL)                  return false;
    if (sscanf(sudo_uid, "%u", &uid) != 1) return false;

    struct passwd *pw = getpwuid(uid);
    if (!pw) return false;

    snprintf(g_sa_socket_file, sizeof(g_sa_socket_file), SA_SOCKET_PATH_FMT, pw->pw_name);
    return true;
}

static bool scripting_addition_is_installed(void)
{
    if (osax_base_dir[0] == 0) scripting_addition_set_path();

    DIR *dir = opendir(osax_base_dir);
    if (!dir) return false;

    closedir(dir);
    return true;
}

static int scripting_addition_check(void)
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

static bool scripting_addition_remove(void)
{
    char cmd[MAXLEN];
    snprintf(cmd, sizeof(cmd), "%s %s %s", "rm -rf", osax_base_dir, "2>/dev/null");

    int code = system(cmd);
    if (code ==  -1) return false;
    if (code == 127) return false;
    return code == 0;
}

static int scripting_addition_install(void)
{
    umask(S_IWGRP | S_IWOTH);

    if ((scripting_addition_is_installed()) && (!scripting_addition_remove())) {
        return 1;
    }

    if (!scripting_addition_create_directory()) {
        goto cleanup;
    }

    if (!scripting_addition_write_file(sa_plist, strlen(sa_plist), osax_info_plist, "w")) {
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
    scripting_addition_restart_dock();
    return 0;

cleanup:
    scripting_addition_remove();
    return 2;
}

static bool scripting_addition_request_handshake(char *version, uint32_t *attrib)
{
    int sockfd;
    bool result = false;
    char rsp[BUFSIZ] = {};
    char bytes[0x100] = { 0x01, 0x0C };

    if (socket_open(&sockfd)) {
        if (socket_connect(sockfd, g_sa_socket_file)) {
            if (send(sockfd, bytes, 2, 0) != -1) {
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

static int scripting_addition_perform_validation(void)
{
    uint32_t attrib = 0;
    char version[0x100] = {};
    bool is_latest_version_installed = scripting_addition_check() == 0;

    if (!scripting_addition_request_handshake(version, &attrib)) {
        notify("scripting-addition", "connection failed!");
        return 1;
    }

    if (string_equals(version, OSAX_VERSION)) {
        if ((attrib & OSAX_ATTRIB_ALL) == OSAX_ATTRIB_ALL) {
            notify("scripting-addition", "payload v%s", version);
            return 0;
        }

        notify("scripting-addition", "payload (0x%X) doesn't support this macOS version!", attrib);
        return 1;
    }

    if (!is_latest_version_installed) {
        notify("scripting-addition", "payload is outdated, updating..");
        return scripting_addition_install();
    }

    notify("scripting-addition", "payload is outdated, restarting Dock.app..");
    scripting_addition_restart_dock();
    return 0;
}

static bool scripting_addition_is_sip_friendly(void)
{
    uint32_t config = 0;
    csr_get_active_config(&config);

    if (!(config & CSR_ALLOW_UNRESTRICTED_FS)) {
        return false;
    }

    if (!(config & CSR_ALLOW_TASK_FOR_PID)) {
        return false;
    }

    return true;
}

static bool mach_loader_inject_payload(void)
{
    FILE *handle = popen("/Library/ScriptingAdditions/yabai.osax/Contents/MacOS/loader", "r");
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

int scripting_addition_uninstall(void)
{
    if (!scripting_addition_is_sip_friendly()) {
        warn("yabai: System Integrity Protection: Filesystem Protections and Debugging Restrictions must be disabled!\n");
        notify("scripting-addition", "System Integrity Protection: Filesystem Protections and Debugging Restrictions must be disabled!");
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

int scripting_addition_load(void)
{
    int result = 0;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    if (!is_root()) {
        warn("yabai: scripting-addition must be loaded as root!\n");
        notify("scripting-addition", "must be loaded as root!");
        result = 1;
        goto out;
    }

    if (!scripting_addition_is_sip_friendly()) {
        warn("yabai: System Integrity Protection: Filesystem Protections and Debugging Restrictions must be disabled!\n");
        notify("scripting-addition", "System Integrity Protection: Filesystem Protections and Debugging Restrictions must be disabled!");
        result = 1;
        goto out;
    }

    if (scripting_addition_check() != 0) {
        result = scripting_addition_install();
        goto out;
    }

    if (!mach_loader_inject_payload()) {
        warn("yabai: scripting-addition failed to inject payload into Dock.app!\n");
        notify("scripting-addition", "failed to inject payload into Dock.app!");
        result = 1;
        goto out;
    }

    if (scripting_addition_set_socket_path()) {
        result = scripting_addition_perform_validation();
    }

out:
    [pool drain];
    return result;
}

#define pack(b,v,l) memcpy(b+l, &v, sizeof(v)); l += sizeof(v)

static bool scripting_addition_send_bytes(char *bytes, int length)
{
    int sockfd;
    char dummy;
    bool result = false;

    if (socket_open(&sockfd)) {
        if (socket_connect(sockfd, g_sa_socket_file)) {
            if (send(sockfd, bytes, length, 0) != -1) {
                recv(sockfd, &dummy, 1, 0);
                result = true;
            }
        }

        socket_close(sockfd);
    }

    return result;
}

bool scripting_addition_focus_space(uint64_t sid)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, sid, length);
    bytes[1] = 0x01;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_create_space(uint64_t sid)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, sid, length);
    bytes[1] = 0x02;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_destroy_space(uint64_t sid)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, sid, length);
    bytes[1] = 0x03;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_move_space_after_space(uint64_t src_sid, uint64_t dst_sid, bool focus)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, src_sid, length);
    pack(bytes, dst_sid, length);
    pack(bytes, focus, length);
    bytes[1] = 0x04;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_move_window(uint32_t wid, int x, int y)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, wid, length);
    pack(bytes, x, length);
    pack(bytes, y, length);
    bytes[1] = 0x05;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_set_opacity(uint32_t wid, float opacity, float duration)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, wid, length);
    pack(bytes, opacity, length);
    pack(bytes, duration, length);
    bytes[1] = duration > 0 ? 0x06 : 0x0D;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_set_layer(uint32_t wid, int layer)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, wid, length);
    pack(bytes, layer, length);
    bytes[1] = 0x07;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_set_sticky(uint32_t wid, bool sticky)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, wid, length);
    pack(bytes, sticky, length);
    bytes[1] = 0x08;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_set_shadow(uint32_t wid, bool shadow)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, wid, length);
    pack(bytes, shadow, length);
    bytes[1] = 0x09;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_focus_window(uint32_t wid)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, wid, length);
    bytes[1] = 0x0A;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_scale_window(uint32_t wid, float x, float y, float w, float h)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, wid, length);
    pack(bytes, x, length);
    pack(bytes, y, length);
    pack(bytes, w, length);
    pack(bytes, h, length);
    bytes[1] = 0x0B;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_swap_window_proxy(uint32_t a_wid, uint32_t b_wid, float opacity, int order)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, a_wid, length);
    pack(bytes, b_wid, length);
    pack(bytes, opacity, length);
    pack(bytes, order, length);
    bytes[1] = 0x0E;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

bool scripting_addition_order_window(uint32_t a_wid, int order, uint32_t b_wid)
{
    char bytes[0x100];

    char length = 2;
    pack(bytes, a_wid, length);
    pack(bytes, order, length);
    pack(bytes, b_wid, length);
    bytes[1] = 0x0F;
    bytes[0] = length-1;

    return scripting_addition_send_bytes(bytes, length);
}

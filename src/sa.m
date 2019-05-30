#include <ScriptingBridge/ScriptingBridge.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "sa.h"
#include "sa_core.c"
#include "sa_bundle.c"

#define OSAX_DIR                  "/System/Library/ScriptingAdditions/CHWMInjector.osax"
#define CONTENTS_DIR              "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents"
#define CONTENTS_MACOS_DIR        "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/MacOS"
#define RESOURCES_DIR             "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/Resources"
#define BUNDLE_DIR                "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/Resources/chunkwm-sa.bundle"
#define BUNDLE_CONTENTS_DIR       "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/Resources/chunkwm-sa.bundle/Contents"
#define BUNDLE_CONTENTS_MACOS_DIR "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/Resources/chunkwm-sa.bundle/Contents/MacOS"

#define INFO_PLIST_FILE           "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/Info.plist"
#define DEFINITION_FILE           "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/Resources/CHWMInjector.sdef"
#define BUNDLE_INFO_PLIST_FILE    "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/Resources/chunkwm-sa.bundle/Contents/Info.plist"

#define BIN_INJECTOR              "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/MacOS/CHWMInjector"
#define BIN_CORE                  "/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/Resources/chunkwm-sa.bundle/Contents/MacOS/chunkwm-sa"

#define CMD_SA_REMOVE             "rm -rf /System/Library/ScriptingAdditions/CHWMInjector.osax 2>/dev/null"

static char sa_def[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<!DOCTYPE dictionary SYSTEM \"file://localhost/System/Library/DTDs/sdef.dtd\">\n"
    "<dictionary title=\"CHWMInjector Terminology\">\n"
    "<suite name=\"CHWMInjector Suite\" code=\"CHWM\" description=\"CHWM Injector commands\">\n"
    "<command name=\"init CHWMInjector\" code=\"CHWMinjc\" description=\"Install Payload into the process\"/>\n"
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
    "<string>CHWMInjector</string>\n"
    "<key>CFBundleIdentifier</key>\n"
    "<string>com.koekeishiya.CHWMInjector</string>\n"
    "<key>CFBundleInfoDictionaryVersion</key>\n"
    "<string>6.0</string>\n"
    "<key>CFBundleName</key>\n"
    "<string>CHWMInjector</string>\n"
    "<key>CFBundlePackageType</key>\n"
    "<string>osax</string>\n"
    "<key>CFBundleShortVersionString</key>\n"
    "<string>1.0</string>\n"
    "<key>CFBundleVersion</key>\n"
    "<string>1.0</string>\n"
    "<key>NSHumanReadableCopyright</key>\n"
    "<string>Copyright © 2017 Åsmund Vikane. All rights reserved.</string>\n"
    "<key>OSAScriptingDefinition</key>\n"
    "<string>CHWMInjector.sdef</string>\n"
    "<key>OSAXHandlers</key>\n"
    "<dict>\n"
    "<key>Events</key>\n"
    "<dict>\n"
    "<key>CHWMinjc</key>\n"
    "<dict>\n"
    "<key>Context</key>\n"
    "<string>Process</string>\n"
    "<key>Handler</key>\n"
    "<string>CHWMhandleInject</string>\n"
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
    "<string>chunkwm-sa</string>\n"
    "<key>CFBundleIdentifier</key>\n"
    "<string>com.koekeishiya.chunkwm-sa</string>\n"
    "<key>CFBundleInfoDictionaryVersion</key>\n"
    "<string>6.0</string>\n"
    "<key>CFBundleName</key>\n"
    "<string>chunkwm-sa</string>\n"
    "<key>CFBundlePackageType</key>\n"
    "<string>BNDL</string>\n"
    "<key>CFBundleShortVersionString</key>\n"
    "<string>1.0</string>\n"
    "<key>CFBundleVersion</key>\n"
    "<string>1</string>\n"
    "<key>NSHumanReadableCopyright</key>\n"
    "<string>Copyright © 2017 Åsmund Vikane. All rights reserved.</string>\n"
    "<key>NSPrincipalClass</key>\n"
    "<string></string>\n"
    "</dict>\n"
    "</plist>";

static bool scripting_addition_create_directory(void)
{
    if (mkdir(OSAX_DIR, 0755))                  goto err;
    if (mkdir(CONTENTS_DIR, 0755))              goto err;
    if (mkdir(CONTENTS_MACOS_DIR, 0755))        goto err;
    if (mkdir(RESOURCES_DIR, 0755))             goto err;
    if (mkdir(BUNDLE_DIR, 0755))                goto err;
    if (mkdir(BUNDLE_CONTENTS_DIR, 0755))       goto err;
    if (mkdir(BUNDLE_CONTENTS_MACOS_DIR, 0755)) goto err;
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
    system("chmod +x \"/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/MacOS/CHWMInjector\"");
    system("chmod +x \"/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/Resources/chunkwm-sa.bundle/Contents/MacOS/chunkwm-sa\"");
    system("codesign -f -s - \"/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/MacOS/CHWMInjector\" 2>/dev/null");
    system("codesign -f -s - \"/System/Library/ScriptingAdditions/CHWMInjector.osax/Contents/Resources/chunkwm-sa.bundle/Contents/MacOS/chunkwm-sa\" 2>/dev/null");
}

bool scripting_addition_is_installed(void)
{
    DIR *dir = opendir(OSAX_DIR);
    if (!dir) return false;

    closedir(dir);
    return true;
}

static bool scripting_addition_remove(void)
{
    int code = system(CMD_SA_REMOVE);
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

    if (!scripting_addition_write_file(sa_plist, strlen(sa_plist), INFO_PLIST_FILE, "w")) {
        goto cleanup;
    }

    if (!scripting_addition_write_file(sa_def, strlen(sa_def), DEFINITION_FILE, "w")) {
        goto cleanup;
    }

    if (!scripting_addition_write_file(sa_bundle_plist, strlen(sa_bundle_plist), BUNDLE_INFO_PLIST_FILE, "w")) {
        goto cleanup;
    }

    if (!scripting_addition_write_file((char *) bin_CHWMInjector_osax_Contents_MacOS_CHWMInjector,
                                       bin_CHWMInjector_osax_Contents_MacOS_CHWMInjector_len,
                                       BIN_INJECTOR, "wb")) {
        goto cleanup;
    }

    if (!scripting_addition_write_file((char *) bin_CHWMInjector_osax_Contents_Resources_chunkwm_sa_bundle_Contents_MacOS_chunkwm_sa,
                                       bin_CHWMInjector_osax_Contents_Resources_chunkwm_sa_bundle_Contents_MacOS_chunkwm_sa_len,
                                       BIN_CORE, "wb")) {
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

    // @memory_leak
    // [SBApplication applicationWithBundleIdentifier] leaks memory and there is nothing we
    // can do about it.. So much for all the automatic memory management techniques in objc

    SBApplication *dock = [SBApplication applicationWithBundleIdentifier:@"com.apple.Dock"];
    [dock setTimeout:10*60];
    [dock setSendMode:kAEWaitReply];
    [dock sendEvent:'ascr' id:'gdut' parameters:0];
    [dock setSendMode:kAEWaitReply];
    [dock sendEvent:'CHWM' id:'injc' parameters:0];
    [dock release];

    return 0;
}

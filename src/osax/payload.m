#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>

#include <mach-o/getsect.h>
#include <mach-o/dyld.h>

#include <objc/message.h>
#include <objc/runtime.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>
#include <dlfcn.h>

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common.h"

#define SOCKET_PATH_FMT "/tmp/yabai-sa_%s.socket"

#define BUF_SIZE 256
#define kCGSOnAllWorkspacesTagBit (1 << 11)
#define kCGSNoShadowTagBit (1 << 3)

extern int CGSMainConnectionID(void);
extern CGError CGSGetConnectionPSN(int cid, ProcessSerialNumber *psn);

extern CGError CGSSetWindowAlpha(int cid, uint32_t wid, float alpha);
extern CGError CGSSetWindowListAlpha(int cid, const uint32_t *window_list, int window_count, float alpha, float duration);
extern CGError CGSSetWindowLevel(int cid, uint32_t wid, int level);
extern OSStatus CGSMoveWindow(const int cid, const uint32_t wid, CGPoint *point);
extern CGError CGSGetWindowOwner(int cid, uint32_t wid, int *window_cid);
extern CGError CGSSetWindowShadowParameters(int cid, CGWindowID wid, CGFloat standardDeviation, CGFloat density, int offsetX, int offsetY);
extern CGError CGSInvalidateWindowShadow(int cid, CGWindowID wid);
extern CGError CGSSetWindowTags(int cid, uint32_t wid, const int tags[2], size_t maxTagSize);
extern CGError CGSClearWindowTags(int cid, uint32_t wid, const int tags[2], size_t maxTagSize);

extern void CGSManagedDisplaySetCurrentSpace(int cid, CFStringRef display_ref, uint64_t spid);
extern uint64_t CGSManagedDisplayGetCurrentSpace(int cid, CFStringRef display_ref);
extern CFArrayRef CGSCopyManagedDisplaySpaces(const int cid);
extern CFStringRef CGSCopyManagedDisplayForSpace(const int cid, uint64_t spid);
extern void CGSShowSpaces(int cid, CFArrayRef spaces);
extern void CGSHideSpaces(int cid, CFArrayRef spaces);

static int _connection;
static id dock_spaces;
static id dp_desktop_picture_manager;
static uint64_t add_space_fp;
static uint64_t remove_space_fp;
static uint64_t move_space_fp;
static uint64_t set_front_window_fp;
static Class managed_space;

static socklen_t sin_size = sizeof(struct sockaddr);
static pthread_t daemon_thread;
static int daemon_sockfd;

static void dump_class_info(Class c)
{
    const char *name = class_getName(c);
    unsigned int count = 0;

    Ivar *ivar_list = class_copyIvarList(c, &count);
    for (int i = 0; i < count; i++) {
        Ivar ivar = ivar_list[i];
        const char *ivar_name = ivar_getName(ivar);
        NSLog(@"%s ivar: %s", name, ivar_name);
    }
    if (ivar_list) free(ivar_list);

    objc_property_t *property_list = class_copyPropertyList(c, &count);
    for (int i = 0; i < count; i++) {
        objc_property_t property = property_list[i];
        const char *prop_name = property_getName(property);
        NSLog(@"%s property: %s", name, prop_name);
    }
    if (property_list) free(property_list);

    Method *method_list = class_copyMethodList(c, &count);
    for (int i = 0; i < count; i++) {
        Method method = method_list[i];
        const char *method_name = sel_getName(method_getName(method));
        NSLog(@"%s method: %s", name, method_name);
    }
    if (method_list) free(method_list);
}

static Class dump_class_info_by_name(const char *name)
{
    Class c = objc_getClass(name);
    if (c != nil) {
        dump_class_info(c);
    }
    return c;
}

static uint64_t static_base_address(void)
{
    const struct segment_command_64 *command = getsegbyname("__TEXT");
    uint64_t addr = command->vmaddr;
    return addr;
}

static uint64_t image_slide(void)
{
    char path[1024];
    uint32_t size = sizeof(path);

    if (_NSGetExecutablePath(path, &size) != 0) {
        return -1;
    }

    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        if (strcmp(_dyld_get_image_name(i), path) == 0) {
            return _dyld_get_image_vmaddr_slide(i);
        }
    }

    return 0;
}

static uint64_t hex_find_seq(uint64_t baddr, const char *c_pattern)
{
    if (!baddr || !c_pattern) return 0;

    uint64_t addr = baddr;
    uint64_t pattern_length = (strlen(c_pattern) + 1) / 3;
    char buffer_a[pattern_length];
    char buffer_b[pattern_length];
    memset(buffer_a, 0, sizeof(buffer_a));
    memset(buffer_b, 0, sizeof(buffer_b));

    char *pattern = (char *) c_pattern + 1;
    for (int i = 0; i < pattern_length; ++i) {
        char c = pattern[-1];
        if (c == '?') {
            buffer_b[i] = 1;
        } else {
            int temp = c <= '9' ? 0 : 9;
            temp = (temp + c) << 0x4;
            c = pattern[0];
            int temp2 = c <= '9' ? 0xd0 : 0xc9;
            buffer_a[i] = temp2 + c + temp;
        }
        pattern += 3;
    }

loop:
    for (int counter = 0; counter < pattern_length; ++counter) {
        if ((buffer_b[counter] == 0) && (((char *)addr)[counter] != buffer_a[counter])) {
            addr = (uint64_t)((char *)addr + 1);
            if (addr - baddr < 0x286a0) {
                goto loop;
            } else {
                return 0;
            }
        }
    }

    return addr;
}

uint64_t get_dock_spaces_offset(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return 0x8000;
    } else if (os_version.minorVersion == 14) {
        if (os_version.patchVersion >= 4) {
            return 0x8f00;
        } else {
            return 0x9a00;
        }
    } else if (os_version.minorVersion == 13) {
        return 0xe10;
    }
    return 0;
}

uint64_t get_dppm_offset(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return 0x6000;
    } else if (os_version.minorVersion == 14) {
        return 0x7000;
    } else if (os_version.minorVersion == 13) {
        return 0x7000;
    }
    return 0;
}

uint64_t get_add_space_offset(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return 0x240000;
    } else if (os_version.minorVersion == 14) {
        return 0x27e500;
    } else if (os_version.minorVersion == 13) {
        return 0x335000;
    }
    return 0;
}

uint64_t get_remove_space_offset(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return 0x320000;
    } else if (os_version.minorVersion == 14) {
        return 0x37fb00;
    } else if (os_version.minorVersion == 13) {
        return 0x495000;
    }
    return 0;
}

uint64_t get_move_space_offset(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return 0x320000;
    } else if (os_version.minorVersion == 14) {
        if (os_version.patchVersion >= 4) {
            return 0x37db10;
        } else {
            return 0x36f500;
        }
    } else if (os_version.minorVersion == 13) {
        if (os_version.patchVersion == 6) {
            return 0x499b00;
        }
    }
    return 0;
}

uint64_t get_set_front_window_offset(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return 0x53000;
    } else if (os_version.minorVersion == 14) {
        if (os_version.patchVersion >= 4) {
            return 0x57500;
        }
    } else if (os_version.minorVersion == 13) {
        if (os_version.patchVersion == 6) {
            return 0x59600;
        }
    }
    return 0;
}

const char *get_dock_spaces_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return "?? ?? ?? 00 ?? 8B ?? 48 8B 35 ?? ?? ?? 00 44 89 ?? 41 FF D4 48 89 C7 E8 ?? ?? 37 00 48 89 85 ?? ?? FF FF 48 8B 3D ?? ?? 46 00 E8 ?? ?? 37 00 4C 8B 35 ?? ?? ?? 00 48 89 C7 4C 89 F6 31 D2 41 FF D4 48 89 85 ?? FE FF FF ?? 8B ?? 48 8B 35 ?? ?? 46 00";
    } else if (os_version.minorVersion == 14) {
        return "?? ?? ?? 00 49 8B 3C 24 48 8B 35 ?? ?? ?? 00 44 89 BD 94 FE FF FF 44 89 FA 41 FF D5 48 89 C7 E8 ?? ?? ?? 00 48 ?? 85 40 FE FF FF 48 8B 3D ?? ?? ?? 00 48 89 DE 41 FF D5 48 8B 35 ?? ?? ?? 00 31 D2 48 89 C7 41 FF D5 48 89 85 70 FE FF FF 49 8B 3C 24";
    } else if (os_version.minorVersion == 13) {
        return "?? ?? ?? 00 48 8B 38 48 8B B5 E0 FD FF FF 4C 8B BD B8 FE FF FF 4C 89 FA 41 FF D5 48 89 C7 E8 ?? ?? ?? 00 49 89 C5 4C 89 EF 48 8B B5 80 FE FF FF FF 15 ?? ?? ?? 00 48 89 C7 E8 ?? ?? ?? 00 48 89 C3 48 89 9D C8 FE FF FF 4C 89 EF 48 8B 05 ?? ?? ?? 00";
    }
    return NULL;
}

const char *get_dppm_pattern(NSOperatingSystemVersion os_version)
{
    if (os_version.minorVersion == 15) {
        return "?? ?? ?? 00 48 89 C6 E8 ?? ?? 37 00 4D 85 F6 0F 84 67 06 00 00 48 8B 3D ?? ?? 46 00 48 8B 35 ?? ?? ?? 00 FF 15 ?? ?? 3E 00 48 89 C7 E8 ?? ?? 37 00 48 89 85 08 FF FF FF C7 85 1C FF FF FF 00 00 00 00 E8 ?? ?? 37 00 48 8D 75 B0 89 C7 E8 ?? ?? 37 00";
    } else if (os_version.minorVersion == 14) {
        return "?? ?? ?? 00 48 89 C6 E8 ?? ?? 3C 00 4D 85 FF 0F 84 AC 06 00 00 48 8B 3D ?? 00 4C 00 48 8B 35 ?? A4 4B 00 FF 15 ?? ?? 43 00 48 89 C7 E8 ?? 6B 3C 00 48 89 85 10 FF FF FF C7 85 1C FF FF FF 00 00 00 00 E8 ?? 69 3C 00 48 8D 75 B0 89 C7 E8 ?? 6A 3C 00";
    } else if (os_version.minorVersion == 13) {
        return "?? ?? ?? 00 4C 89 FE E8 99 6D 4A 00 4D 85 FF 0F 84 A2 06 00 00 48 8B 3D EB ED 5A 00 48 8B 35 AC 90 5A 00 FF 15 BE 1B 52 00 48 89 C7 E8 5C 6D 4A 00 48 89 85 30 FF FF FF 48 8D 3D A4 11 4E 00 E8 5A 8B 17 00 88 05 5D 28 5D 00 84 C0 74 14 48 8B 3D 6A";
    }
    return NULL;
}

const char *get_add_space_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 28 4C 89 6D B8 48 BA 01 00 00 00 00 00 00 40 48 B9 F8 FF FF FF FF FF FF 00 49 8D 45 28 48 89 45 C8 4D 8B 7D 28 41 80 7D 38 01 48 89 7D C0 75 5B 49 89 FC 49 85 D7 0F 84 A7 00 00 00 4C 89 FB 48 21 CB 41 F6 C7 01 49 0F 45 DF 4C 89 FF E8 ?? ??";
    } else if (os_version.minorVersion == 14) {
        if (os_version.patchVersion >= 4) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 78 49 BC 01 00 00 00 00 00 00 40 49 BE F8 FF FF FF FF FF FF 00 49 8D 5D 20 4C 89 6D B8 41 80 7D 30 01 48 89 7D C0 48 89 5D D0 75 32 49 89 FF 48 8D 75 80 31 D2 31 C9 48 89 DF E8 03 79 14 00 48 8B 1B 4C 85 E3 0F 85 CE 03 00 00 4D 89 E5 4C 21 F3 48 8B 43 10 48 89 45 C8 E9 B1 01 00 00 48 8D 75 80 31 D2 31 C9 48 89 DF E8 D4 78 14 00 4C 8B 33 4D 85 E6 4D 89 E5 0F 85 FF 03 00 00 4C 89 F0 48 B9 F8 FF FF FF FF FF FF 00 48 21 C8 4C 8B 60 10 4C 89 F7 E8 BB 78 14 00 4D 85 E4 0F 84 39 01 00 00 4D 89 E7 49 FF CF 0F 80 EA 04 00 00 4C 89 65 C8 48 BB 03 00 00 00 00 00 00 C0 31 F6 49 85 DE 40 0F 94 C6 4C 89 FF 4C 89 F2 E8 D0 F0 00 00 49 85 DE";
        } else {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 18 48 89 7D C0 41 8A 5D 30 4C 89 6D C8 4D 8B 65 20 4C 89 E7 E8 8B 17 15 00 4C 89 E7 E8 0F 7E F3 FF 49 89 C6 4D 89 F7 80 FB 01 74 6E 4D 85 F6 0F 84 24 01 00 00 49 FF CE 0F 80 7E 02 00 00 48 BB 03 00 00 00 00 00 00 C0 31 F6 49 85 DC 40 0F 94 C6 4C 89 F7 4C 89 E2 E8 F4 7E 10 00 49 85 DC 0F 85 29 02 00 00 4F 8B 6C F4 20 4C 89 EF E8 92 14 15 00 48 8B 05 6F 8E 1C 00 48 89 C3 48 8B 08 49 23 4D 00 FF 91 80 00 00 00 88 45 D0 4C 89 EF E8 6A 14 15 00 F6 45 D0 01 74 08 4C 89 E7 E9 ED 00 00 00 4D 85 F6 49 89 DF 0F 84 AB 00 00 00 49 FF CE 0F 80 9E 00 00 00 48 B8 F8 FF FF FF FF FF FF 00 4C 21 E0 48 89 45 D0 48 B8 03 00 00 00 00 00 00 C0 49 85 C4 74 27 4C 89 E7 E8 C5 16 15 00 4C 89 F7 4C 89 E6 48 8D 15 64 11 F7 FF E8 8F C4 00 00 48 89 C3 4C 89 E7 E8 9C 16 15 00 EB 25 48 8B 05 7B 83 1C 00";
        }
    } else if (os_version.minorVersion == 13) {
        return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 38 4C 89 6D B0 49 89 FC 48 BB 01 00 00 00 00 00 00 C0 48 B9 01 00 00 00 00 00 00 80 49 BF F8 FF FF FF FF FF FF 00 49 8D 45 28 48 89 45 C0 4D 8B 75 28 41 80 7D 38 01 4C 89 65 C8";
    }
    return NULL;
}

const char *get_remove_space_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC B8 00 00 00 49 89 CF 48 89 55 80 49 89 F5 48 89 7D B8 48 BB F8 FF FF FF FF FF FF 00 E8 ?? ?? F0 FF 49 89 C4 48 B8 01 00 00 00 00 00 00 40 49 85 C4 0F 85 AA 05 00 00 4C 21 E3 4C 8B 73 10 49 83 FE 02 0F 8C B6 02 00 00 4C 89 7D 90 4C 89 AD 50 FF FF FF 4C 89 65 88 48 8D 05 ?? ?? ?? 00 48 8B";
    } else if (os_version.minorVersion == 14) {
        if (os_version.patchVersion >= 4) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC 18 01 00 00 48 89 4D B8 48 89 55 90 49 89 F7 48 89 7D B0 48 BB F8 FF FF FF FF FF FF 00 49 89 F5 E8 FB 8B EF FF 49 89 C4 48 B8 01 00 00 00 00 00 00 40 49 85 C4 0F 85 6F 06 00 00 4C 21 E3 4C 8B 73 10 49 83 FE 02 0F 8C 08 03 00 00 4C 89 7D 80 4C 89 65 98 48 8D 05 E5 A6 14 00 48 8B 00 48 8B 5D B0 48 8B 0C 03 48 89 8D 70 FF FF FF 4C 8B 64 03 08 4C 89 65 C0 4C 8D 35 83 8E 15 00 48 8D B5 F0 FE FF FF 31 D2 31 C9 4C 89 F7 E8 C2 0E 04 00 4D 8B 3E 4C 8B 35 5E 92 13 00 4C 89 E7 E8 C2 0E 04 00 4C 89 FF 4C 89 F6 48 89 DA E8 F6 0B 04 00 4C 8B 35 E1 FA 14 00";
        } else {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 48 48 89 4D A0 49 89 D4 49 89 F7 49 89 FE 4D 89 FD E8 ?? ?? EF FF 48 89 C3 48 89 DF E8 ?? ?? ?? FF 48 83 F8 02 0F 8C 94 01 00 00 4C 89 7D A8 48 89 5D B8";
        }
    } else if (os_version.minorVersion == 13) {
        return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 68 4C 89 45 80 48 89 4D C0 48 89 55 D0 48 89 F3 49 89 FC 49 BE 01 00 00 00 00 00 00 C0 49 89 DD E8 ?? ?? E9 FF 49 89 C7 4D 85 F7";
    }
    return NULL;
}

const char *get_move_space_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 48 4C 89 E9 41 89 D5 49 89 F4 49 89 FE 48 8D 1D ?? ?? 15 00 48 8B 03 4C 8B 3C 07 4C 89 FF 48 89 4D 98 48 89 CE E8 ?? ?? 00 00 48 89 55 D0 48 89 45 C8 48 85 C0 0F 84 97 03 00 00 48 8D 05 ?? ?? ?? 00 80 38 01 75 33 4C 8B 7D D0 4D 89 FD 49 83 C5 28 48 8B 5D C8 48 89 DF E8 ?? ?? ?? FF 48 89";
    } else if (os_version.minorVersion == 14) {
        if (os_version.patchVersion >= 4) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC A8 00 00 00 41 89 D7 48 89 75 D0 48 89 FB 4C 8D 35 03 A2 15 00 49 8B 06 4C 8B 24 07 4C 89 E7 4C 89 6D A8 4C 89 EE E8 65 CC 00 00 48 89 55 C0 48 85 C0 0F 84 89 00 00 00 48 89 5D B8 48 89 45 C8 48 8D 1D 6E 85 16 00 48 8D B5 38 FF FF FF 31 D2 31 C9 48 89 DF E8 C8 09 05 00 80 3B 01 75 1C 4C 8B 7D C8 4C 89 FF 48 8B 75 D0 4C 8B 75 C0 4D 89 F5 E8 7A F5 F0 FF E9 6E 05 00 00 48 8B 5D D0 48 85 DB 74 44 49 8B 06 4C 8B 34 03 48 89 DF E8 FB 06 05 00 4C 89 F7 48 8B 75 A8 E8 31 CD 00 00 49 89 C7 48 89 DF E8 DE 06 05 00 4D 85 FF 75 44 48 8B 7D C0 E8 8A 0A 05 00 48 8B 7D C8 E8 C7 06 05 00";
        } else {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 48 4D 89 EC 41 89 D5 49 89 ?? ?? 89 FB 48 8B 05 ?? ?? ?? 00 4C 8B 3C 03 4C 89 ?? 4C 89 E6 E8 ?? CC 00 00 48 89 55 ?? 48 89 45 ?? 48 85 C0 0F 84 ?? ?? 00 00";
        }
    } else if (os_version.minorVersion == 13) {
        if (os_version.patchVersion == 6) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 48 4D 89 EC 41 89 D5 49 89 F7 49 89 FE 48 8B 05 4C 2F 13 00 49 8B 1C 06 48 89 DF 4C 89 E6 E8 AD EB FE FF 48 89 55 C8 48 89 45 C0 48 85 C0 0F 84 84 00 00 00";
        }
    }
    return NULL;
}

const char *get_set_front_window_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.minorVersion == 15) {
        return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 58 48 8B 05 ?? ?? ?? 00 48 8B 00 48 89 45 D0 85 F6 0F 84 0A 02 00 00 41 89 F5 49 89 FF 49 89 FE 49 C1 EE 20 48 8D 75 AF C6 06 00 E8 ?? ?? 02 00 48 8B 3D ?? ?? ?? 00 BE 01 00 00 00 E8 ?? ?? 32 00 84 C0 74 59 0F B6 5D AF 4C 8D 45";
    } else if (os_version.minorVersion == 14) {
        if (os_version.patchVersion >= 4) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 58 48 8B 05 ?? C8 3E 00 48 8B 00 48 89 45 D0 85 F6 0F 84 0A 02 00 00 41 89 F5 49 89 FE 49 89 FF 49 C1 EF 20 48 8D 75 AF C6 06 00 E8 ?? 16 03 00 48 8B 3D ?? C9 3E 00 BE 01 00 00 00 E8 ?? 6C 37 00 84 C0 74 59 0F B6 5D AF";
        }
    } else if (os_version.minorVersion == 13) {
        if (os_version.patchVersion == 6) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 48 41 89 F5 49 89 FE 48 8B 05 C6 FE 4C 00 48 8B 00 48 89 45 D0 45 85 ED 0F 84 1F 02 00 00 4D 89 F7 49 C1 EF 20 48 8D 75 CF C6 06 00 4C 89 F7 E8 07 1C 03 00 48 8B 3D 01 00 4D 00 BE 01 00 00 00 E8 D5 56 45 00 84 C0 74 6C 49 89 E4 48 89 E0 4C 8D 40 E0 4C";
        }
    }
    return NULL;
}

static void init_instances()
{
    // TODO(koekeishiya): Do proper version checks with minor and patch version..
    NSOperatingSystemVersion os_version = [[NSProcessInfo processInfo] operatingSystemVersion];
    if (os_version.minorVersion != 13 && os_version.minorVersion != 14 && os_version.minorVersion != 15) {
        NSLog(@"[yabai-sa] spaces functionality is only supported on macOS High Sierra, Mojave and Catalina!");
        return;
    }

    uint64_t baseaddr = static_base_address() + image_slide();
    uint64_t dock_spaces_addr = hex_find_seq(baseaddr + get_dock_spaces_offset(os_version), get_dock_spaces_pattern(os_version));
    if (dock_spaces_addr == 0) {
        NSLog(@"[yabai-sa] could not locate pointer to dock.spaces! spaces functionality will not work!");
        return;
    }

    uint32_t dock_spaces_offset = *(int32_t *)dock_spaces_addr;
    NSLog(@"[yabai-sa] (0x%llx) dock.spaces found at address 0x%llX (0x%llx)", baseaddr, dock_spaces_addr, dock_spaces_addr - baseaddr);
    dock_spaces = [(*(id *)(dock_spaces_addr + dock_spaces_offset + 0x4)) retain];

    uint64_t dppm_addr = hex_find_seq(baseaddr + get_dppm_offset(os_version), get_dppm_pattern(os_version));
    if (dppm_addr == 0) {
        dp_desktop_picture_manager = nil;
        NSLog(@"[yabai-sa] could not locate pointer to dppm! moving spaces will not work!");
    } else {
        uint32_t dppm_offset = *(int32_t *)dppm_addr;
        NSLog(@"[yabai-sa] (0x%llx) dppm found at address 0x%llX (0x%llx)", baseaddr, dppm_addr, dppm_addr - baseaddr);
        dp_desktop_picture_manager = [(*(id *)(dppm_addr + dppm_offset + 0x4)) retain];
    }

    uint64_t add_space_addr = hex_find_seq(baseaddr + get_add_space_offset(os_version), get_add_space_pattern(os_version));
    if (add_space_addr == 0x0) {
        NSLog(@"[yabai-sa] failed to get pointer to addSpace function..");
        add_space_fp = 0;
    } else {
        NSLog(@"[yabai-sa] (0x%llx) addSpace found at address 0x%llX (0x%llx)", baseaddr, add_space_addr, add_space_addr - baseaddr);
        add_space_fp = add_space_addr;
    }

    uint64_t remove_space_addr = hex_find_seq(baseaddr + get_remove_space_offset(os_version), get_remove_space_pattern(os_version));
    if (remove_space_addr == 0x0) {
        NSLog(@"[yabai-sa] failed to get pointer to removeSpace function..");
        remove_space_fp = 0;
    } else {
        NSLog(@"[yabai-sa] (0x%llx) removeSpace found at address 0x%llX (0x%llx)", baseaddr, remove_space_addr, remove_space_addr - baseaddr);
        remove_space_fp = remove_space_addr;
    }

    uint64_t move_space_addr = hex_find_seq(baseaddr + get_move_space_offset(os_version), get_move_space_pattern(os_version));
    if (move_space_addr == 0x0) {
        NSLog(@"[yabai-sa] failed to get pointer to moveSpace function..");
        move_space_fp = 0;
    } else {
        NSLog(@"[yabai-sa] (0x%llx) moveSpace found at address 0x%llX (0x%llx)", baseaddr, move_space_addr, move_space_addr - baseaddr);
        move_space_fp = move_space_addr;
    }

    uint64_t set_front_window_addr = hex_find_seq(baseaddr + get_set_front_window_offset(os_version), get_set_front_window_pattern(os_version));
    if (set_front_window_addr == 0x0) {
        NSLog(@"[yabai-sa] failed to get pointer to setFrontWindow function..");
        set_front_window_fp = 0;
    } else {
        NSLog(@"[yabai-sa] (0x%llx) setFrontWindow found at address 0x%llX (0x%llx)", baseaddr, set_front_window_addr, set_front_window_addr - baseaddr);
        set_front_window_fp = set_front_window_addr;
    }

    managed_space = objc_getClass("Dock.ManagedSpace");
    _connection = CGSMainConnectionID();
}

typedef struct
{
    const char *text;
    unsigned int length;
} Token;

static bool token_equals(Token token, const char *match)
{
    const char *at = match;
    for (int i = 0; i < token.length; ++i, ++at) {
        if ((*at == 0) || (token.text[i] != *at)) {
            return false;
        }
    }
    return *at == 0;
}

static uint64_t token_to_uint64t(Token token)
{
    uint64_t result = 0;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%lld", &result);
    return result;
}

static uint32_t token_to_uint32t(Token token)
{
    uint32_t result = 0;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%d", &result);
    return result;
}

static int token_to_int(Token token)
{
    int result = 0;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%d", &result);
    return result;
}

static float token_to_float(Token token)
{
    float result = 0.0f;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%f", &result);
    return result;
}

static Token get_token(const char **message)
{
    Token token;

    token.text = *message;
    while (**message && !isspace(**message)) {
        ++(*message);
    }
    token.length = *message - token.text;

    if (isspace(**message)) {
        ++(*message);
    } else {
        // NOTE(koekeishiya): don't go past the null-terminator
    }

    return token;
}

static inline id get_ivar_value(id instance, const char *name)
{
    id result = nil;
    object_getInstanceVariable(instance, name, (void **) &result);
    return result;
}

static inline void set_ivar_value(id instance, const char *name, id value)
{
    object_setInstanceVariable(instance, name, value);
}

static inline uint64_t get_space_id(id space)
{
    return (uint64_t) objc_msgSend(space, @selector(spid));
}

static inline id space_for_display_with_id(CFStringRef display_uuid, uint64_t space_id)
{
    NSArray *spaces_for_display = (NSArray *) objc_msgSend(dock_spaces, @selector(spacesForDisplay:), display_uuid);
    for (id space in spaces_for_display) {
        if (space_id == get_space_id(space)) {
            return space;
        }
    }
    return nil;
}

static inline id display_space_for_display_uuid(CFStringRef display_uuid)
{
    id result = nil;

    NSArray *display_spaces = get_ivar_value(dock_spaces, "_displaySpaces");
    if (display_spaces != nil) {
        for (id display_space in display_spaces) {
            id display_source_space = get_ivar_value(display_space, "_currentSpace");
            uint64_t sid = get_space_id(display_source_space);
            CFStringRef uuid = CGSCopyManagedDisplayForSpace(_connection, sid);
            bool match = CFEqual(uuid, display_uuid);
            CFRelease(uuid);
            if (match) {
                result = display_space;
                break;
            }
        }
    }

    return result;
}

static inline id display_space_for_space_with_id(uint64_t space_id)
{
    NSArray *display_spaces = get_ivar_value(dock_spaces, "_displaySpaces");
    if (display_spaces != nil) {
        for (id display_space in display_spaces) {
            id display_source_space = get_ivar_value(display_space, "_currentSpace");
            if (get_space_id(display_source_space) == space_id) {
                return display_space;
            }
        }
    }
    return nil;
}

#define asm__call_move_space(v0,v1,v2,v3,func) \
        __asm__("movq %0, %%rdi;""movq %1, %%rsi;""movq %2, %%rdx;""movq %3, %%r13;""callq *%4;" : :"r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(func) :"%rdi", "%rsi", "%rdx", "%r13");
static void do_space_move(const char *message)
{
    Token source_token = get_token(&message);
    uint64_t source_space_id = token_to_uint64t(source_token);

    Token dest_token = get_token(&message);
    uint64_t dest_space_id = token_to_uint64t(dest_token);

    Token focus_token = get_token(&message);
    bool focus_dest_space = token_to_int(focus_token);

    CFStringRef source_display_uuid = CGSCopyManagedDisplayForSpace(_connection, source_space_id);
    id source_space = space_for_display_with_id(source_display_uuid, source_space_id);
    id source_display_space = display_space_for_display_uuid(source_display_uuid);

    CFStringRef dest_display_uuid = CGSCopyManagedDisplayForSpace(_connection, dest_space_id);
    id dest_space = space_for_display_with_id(dest_display_uuid, dest_space_id);
    unsigned dest_display_id = ((unsigned (*)(id, SEL, id)) objc_msgSend)(dock_spaces, @selector(displayIDForSpace:), dest_space);
    id dest_display_space = display_space_for_display_uuid(dest_display_uuid);

    asm__call_move_space(source_space, dest_space, dest_display_uuid, dock_spaces, move_space_fp);

    dispatch_sync(dispatch_get_main_queue(), ^{
        objc_msgSend(dp_desktop_picture_manager, @selector(moveSpace:toDisplay:displayUUID:), source_space, dest_display_id, dest_display_uuid);
    });

    if (focus_dest_space) {
        uint64_t new_source_space_id = CGSManagedDisplayGetCurrentSpace(_connection, source_display_uuid);
        id new_source_space = space_for_display_with_id(source_display_uuid, new_source_space_id);
        set_ivar_value(source_display_space, "_currentSpace", [new_source_space retain]);

        NSArray *ns_dest_monitor_space = @[ @(dest_space_id) ];
        CGSHideSpaces(_connection, (__bridge CFArrayRef) ns_dest_monitor_space);
        CGSManagedDisplaySetCurrentSpace(_connection, dest_display_uuid, source_space_id);
        set_ivar_value(dest_display_space, "_currentSpace", [source_space retain]);
        [ns_dest_monitor_space release];
    }

    CFRelease(source_display_uuid);
    CFRelease(dest_display_uuid);
}

typedef void (*remove_space_call)(id space, id display_space, id dock_spaces, uint64_t space_id1, uint64_t space_id2);
static void do_space_destroy(const char *message)
{
    Token space_id_token = get_token(&message);
    uint64_t space_id = token_to_uint64t(space_id_token);
    CFStringRef display_uuid = CGSCopyManagedDisplayForSpace(_connection, space_id);
    uint64_t active_space_id = CGSManagedDisplayGetCurrentSpace(_connection, display_uuid);

    id space = space_for_display_with_id(display_uuid, space_id);
    id display_space = display_space_for_display_uuid(display_uuid);

    dispatch_sync(dispatch_get_main_queue(), ^{
        ((remove_space_call) remove_space_fp)(space, display_space, dock_spaces, space_id, space_id);
    });

    if (active_space_id == space_id) {
        uint64_t dest_space_id = CGSManagedDisplayGetCurrentSpace(_connection, display_uuid);
        id dest_space = space_for_display_with_id(display_uuid, dest_space_id);
        set_ivar_value(display_space, "_currentSpace", [dest_space retain]);
    }

    CFRelease(display_uuid);
}

#define asm__call_add_space(v0,v1,func) \
        __asm__("movq %0, %%rdi;""movq %1, %%r13;""callq *%2;" : :"r"(v0), "r"(v1), "r"(func) :"%rdi", "%r13");
static void do_space_create(const char *message)
{
    Token space_id_token = get_token(&message);
    uint64_t space_id = token_to_uint64t(space_id_token);
    CFStringRef __block display_uuid = CGSCopyManagedDisplayForSpace(_connection, space_id);
    dispatch_sync(dispatch_get_main_queue(), ^{
        id new_space = [[managed_space alloc] init];
        id display_space = display_space_for_display_uuid(display_uuid);
        asm__call_add_space(new_space, display_space, add_space_fp);
        CFRelease(display_uuid);
    });
}

static void do_space_change(const char *message)
{
    Token token = get_token(&message);
    uint64_t dest_space_id = token_to_uint64t(token);
    if (dest_space_id) {
        CFStringRef dest_display = CGSCopyManagedDisplayForSpace(_connection, dest_space_id);
        id source_space = objc_msgSend(dock_spaces, @selector(currentSpaceforDisplayUUID:), dest_display);
        uint64_t source_space_id = get_space_id(source_space);

        if (source_space_id != dest_space_id) {
            id dest_space = space_for_display_with_id(dest_display, dest_space_id);
            if (dest_space != nil) {
                id display_space = display_space_for_space_with_id(source_space_id);
                if (display_space != nil) {
                    NSArray *ns_source_space = @[ @(source_space_id) ];
                    NSArray *ns_dest_space = @[ @(dest_space_id) ];
                    CGSShowSpaces(_connection, (__bridge CFArrayRef) ns_dest_space);
                    CGSHideSpaces(_connection, (__bridge CFArrayRef) ns_source_space);
                    CGSManagedDisplaySetCurrentSpace(_connection, dest_display, dest_space_id);
                    set_ivar_value(display_space, "_currentSpace", [dest_space retain]);
                    [ns_dest_space release];
                    [ns_source_space release];
                }
            }
        }
        CFRelease(dest_display);
    }
}

static void do_window_move(const char *message)
{
    Token wid_token = get_token(&message);
    uint32_t wid = token_to_uint32t(wid_token);
    if (!wid) return;

    Token x_token = get_token(&message);
    int x = token_to_int(x_token);
    Token y_token = get_token(&message);
    int y = token_to_int(y_token);
    CGPoint point = CGPointMake(x, y);
    CGSMoveWindow(_connection, wid, &point);
}

static void do_window_alpha(const char *message)
{
    Token wid_token = get_token(&message);
    uint32_t wid = token_to_uint32t(wid_token);
    if (!wid) return;

    Token alpha_token = get_token(&message);
    float alpha = token_to_float(alpha_token);
    CGSSetWindowAlpha(_connection, wid, alpha);
}

static void do_window_alpha_fade(const char *message)
{
    Token wid_token = get_token(&message);
    uint32_t wid = token_to_uint32t(wid_token);
    if (!wid) return;

    Token alpha_token = get_token(&message);
    float alpha = token_to_float(alpha_token);
    Token duration_token = get_token(&message);
    float duration = token_to_float(duration_token);
    CGSSetWindowListAlpha(_connection, &wid, 1, alpha, duration);
}

static void do_window_level(const char *message)
{
    Token wid_token = get_token(&message);
    uint32_t wid = token_to_uint32t(wid_token);
    if (!wid) return;

    Token key_token = get_token(&message);
    int key = token_to_int(key_token);
    CGSSetWindowLevel(_connection, wid, CGWindowLevelForKey(key));
}

static void do_window_sticky(const char *message)
{
    Token wid_token = get_token(&message);
    uint32_t wid = token_to_uint32t(wid_token);
    if (!wid) return;

    Token value_token = get_token(&message);
    int value = token_to_int(value_token);
    int tags[2] = { kCGSOnAllWorkspacesTagBit, 0 };
    if (value == 1) {
        CGSSetWindowTags(_connection, wid, tags, 32);
    } else {
        CGSClearWindowTags(_connection, wid, tags, 32);
    }
}

typedef void (*focus_window_call)(ProcessSerialNumber psn, uint32_t wid);
static void do_window_focus(const char *message)
{
    int window_connection;
    ProcessSerialNumber window_psn;

    Token wid_token = get_token(&message);
    uint32_t window_id = token_to_uint32t(wid_token);

    CGSGetWindowOwner(_connection, window_id, &window_connection);
    CGSGetConnectionPSN(window_connection, &window_psn);

    ((focus_window_call) set_front_window_fp)(window_psn, window_id);
}

static void do_window_shadow(const char *message)
{
    Token wid_token = get_token(&message);
    uint32_t wid = token_to_uint32t(wid_token);
    if (!wid) return;

    Token value_token = get_token(&message);
    int value = token_to_int(value_token);
    int tags[2] = { kCGSNoShadowTagBit,  0};
    if (value == 1) {
        CGSClearWindowTags(_connection, wid, tags, 32);
    } else {
        CGSSetWindowTags(_connection, wid, tags, 32);
    }
    CGSInvalidateWindowShadow(_connection, wid);
}

static void do_window_shadow_irreversible(const char *message)
{
    Token wid_token = get_token(&message);
    uint32_t wid = token_to_uint32t(wid_token);
    if (!wid) return;

    CGSSetWindowShadowParameters(_connection, wid, 0, 0, 0, 0);
}

static inline bool can_focus_space()
{
    return dock_spaces != nil;
}

static inline bool can_create_space()
{
    return dock_spaces != nil && add_space_fp != 0;
}

static inline bool can_destroy_space()
{
    return dock_spaces != nil && remove_space_fp != 0;
}

static inline bool can_move_space()
{
    return dock_spaces != nil && dp_desktop_picture_manager != nil && move_space_fp != 0;
}

static inline bool can_focus_window()
{
    return set_front_window_fp != 0;
}

static void do_handshake(int sockfd)
{
    uint32_t attrib = 0;

    if (dock_spaces != nil)                attrib |= OSAX_ATTRIB_DOCK_SPACES;
    if (dp_desktop_picture_manager != nil) attrib |= OSAX_ATTRIB_DPPM;
    if (can_create_space())                attrib |= OSAX_ATTRIB_ADD_SPACE;
    if (can_destroy_space())               attrib |= OSAX_ATTRIB_REM_SPACE;
    if (can_move_space())                  attrib |= OSAX_ATTRIB_MOV_SPACE;
    if (can_focus_window())                attrib |= OSAX_ATTRIB_SET_WINDOW;

    char bytes[BUFSIZ] = {};
    int version_length = strlen(OSAX_VERSION);
    int attrib_length = sizeof(uint32_t);
    int bytes_length = version_length + 1 + attrib_length;

    memcpy(bytes, OSAX_VERSION, version_length);
    memcpy(bytes + version_length + 1, &attrib, attrib_length);
    bytes[version_length] = '\0';
    bytes[bytes_length] = '\n';

    send(sockfd, bytes, bytes_length+1, 0);
}

static void handle_message(int sockfd, const char *message)
{
    /*
     * NOTE(koekeishiya): interaction is supposed to happen through an
     * external program (yabai), and so we do not bother doing input
     * validation, as the program in question should do this.
     */

    Token token = get_token(&message);
    if (token_equals(token, "handshake")) {
        do_handshake(sockfd);
    } else if (token_equals(token, "space")) {
        if (!can_focus_space()) return;
        do_space_change(message);
    } else if (token_equals(token, "space_create")) {
        if (!can_create_space()) return;
        do_space_create(message);
    } else if (token_equals(token, "space_destroy")) {
        if (!can_destroy_space()) return;
        do_space_destroy(message);
    } else if (token_equals(token, "space_move")) {
        if (!can_move_space()) return;
        do_space_move(message);
    } else if (token_equals(token, "window_move")) {
        do_window_move(message);
    } else if (token_equals(token, "window_alpha")) {
        do_window_alpha(message);
    } else if (token_equals(token, "window_alpha_fade")) {
        do_window_alpha_fade(message);
    } else if (token_equals(token, "window_level")) {
        do_window_level(message);
    } else if (token_equals(token, "window_sticky")) {
        do_window_sticky(message);
    } else if (token_equals(token, "window_focus")) {
        if (!can_focus_window()) return;
        do_window_focus(message);
    } else if (token_equals(token, "window_shadow")) {
        do_window_shadow(message);
    } else if (token_equals(token, "window_shadow_irreversible")) {
        do_window_shadow_irreversible(message);
    }
}

static bool recv_socket(int sockfd, char *message, size_t message_size)
{
    int len = recv(sockfd, message, message_size, 0);
    if (len > 0) {
        message[len] = '\0';
        return true;
    }
    return false;
}

static void *handle_connection(void *unused)
{
    while (1) {
        int sockfd = accept(daemon_sockfd, NULL, 0);
        if (sockfd == -1) continue;

        char message[BUF_SIZE];
        if (recv_socket(sockfd, message, sizeof(message))) {
            handle_message(sockfd, message);
        }

        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
    }

    return NULL;
}

static bool start_daemon(char *socket_path)
{
    struct sockaddr_un socket_address;
    socket_address.sun_family = AF_UNIX;
    snprintf(socket_address.sun_path, sizeof(socket_address.sun_path), "%s", socket_path);
    unlink(socket_path);

    if ((daemon_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        return false;
    }

    if (bind(daemon_sockfd, (struct sockaddr *) &socket_address, sizeof(socket_address)) == -1) {
        return false;
    }

    if (chmod(socket_path, 0600) != 0) {
        return false;
    }

    if (listen(daemon_sockfd, SOMAXCONN) == -1) {
        return false;
    }

    pthread_create(&daemon_thread, NULL, &handle_connection, NULL);
    return true;
}

@interface Payload : NSObject
+ (void) load;
@end

@implementation Payload
+ (void) load
{
    NSLog(@"[yabai-sa] loaded payload");
    init_instances();

    const char *user = getenv("USER");
    if (!user) {
        NSString *ns_user = NSUserName();
        if (ns_user) user = [ns_user UTF8String];
    }

    if (!user) {
        NSLog(@"[yabai-sa] could not get 'env USER'! abort..");
        return;
    }

    char socket_file[255];
    snprintf(socket_file, sizeof(socket_file), SOCKET_PATH_FMT, user);

    if (start_daemon(socket_file)) {
        NSLog(@"[yabai-sa] now listening..");
    } else {
        NSLog(@"[yabai-sa] failed to spawn thread..");
    }
}
@end

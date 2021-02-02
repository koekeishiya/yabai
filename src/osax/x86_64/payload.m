#define asm__call_add_space(v0,v1,func) \
        __asm__("movq %0, %%rdi;""movq %1, %%r13;""callq *%2;" : :"r"(v0), "r"(v1), "r"(func) :"%rdi", "%r13");

#define asm__call_move_space(v0,v1,v2,v3,func) \
        __asm__("movq %0, %%rdi;""movq %1, %%rsi;""movq %2, %%rdx;""movq %3, %%r13;""callq *%4;" : :"r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(func) :"%rdi", "%rsi", "%rdx", "%r13");

uint64_t get_dock_spaces_offset(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x8d00;
    } if (os_version.minorVersion == 15) {
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
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x7000;
    } else if (os_version.minorVersion == 15) {
        return 0x6000;
    } else if (os_version.minorVersion == 14) {
        return 0x7000;
    } else if (os_version.minorVersion == 13) {
        return 0x7000;
    }
    return 0;
}

uint64_t get_add_space_offset(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x230000;
    } else if (os_version.minorVersion == 15) {
        return 0x230000;
    } else if (os_version.minorVersion == 14) {
        return 0x27e500;
    } else if (os_version.minorVersion == 13) {
        return 0x335000;
    }
    return 0;
}

uint64_t get_remove_space_offset(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x2F0000;
    } else if (os_version.minorVersion == 15) {
        return 0x320000;
    } else if (os_version.minorVersion == 14) {
        return 0x37fb00;
    } else if (os_version.minorVersion == 13) {
        return 0x495000;
    }
    return 0;
}

uint64_t get_move_space_offset(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x2E0000;
    } else if (os_version.minorVersion == 15) {
        return 0x310000;
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
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x51000;
    } else if (os_version.minorVersion == 15) {
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
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return "?? ?? ?? 00 49 8B 7D 00 48 8B 35 ?? ?? ?? 00 44 89 BD 38 FE FF FF 44 89 FA 41 FF D4 48 89 C7 E8 ?? ?? 32 00 48 89 85 E8 FD FF FF 48 8B 3D ?? ?? 41 00 E8 ?? ?? 32 00 48 8B 35 ?? ?? ?? 00 48 89 C7 31 D2 41 FF D4 48 89 85 28 FE FF FF 49 8B 7D 00 48";
    } else if (os_version.minorVersion == 15) {
        if (os_version.patchVersion >= 4) {
            return "?? ?? ?? 00 49 8B 3F 48 8B 35 ?? 3D 44 00 44 89 B5 40 FE FF FF 44 89 F2 41 FF D4 48 89 C7 E8 ?? 70 35 00 48 89 85 E8 FD FF FF 48 8B 3D ?? 8D 44 00 E8 ?? 6F 35 00 4C 8B 35 ?? 3E 44 00 48 89 C7 4C 89 F6 31 D2 41 FF D4 48 89 85 28 FE FF FF 49 8B 3F 48 8B 35 ?? 82 44 00 48 89 DA 41 FF";
        } else {
            return "?? ?? ?? 00 ?? 8B ?? 48 8B 35 ?? ?? ?? 00 44 89 ?? 41 FF D4 48 89 C7 E8 ?? ?? 37 00 48 89 85 ?? ?? FF FF 48 8B 3D ?? ?? 46 00 E8 ?? ?? 37 00 4C 8B 35 ?? ?? ?? 00 48 89 C7 4C 89 F6 31 D2 41 FF D4 48 89 85 ?? FE FF FF ?? 8B ?? 48 8B 35 ?? ?? 46 00";
        }
    } else if (os_version.minorVersion == 14) {
        return "?? ?? ?? 00 49 8B 3C 24 48 8B 35 ?? ?? ?? 00 44 89 BD 94 FE FF FF 44 89 FA 41 FF D5 48 89 C7 E8 ?? ?? ?? 00 48 ?? 85 40 FE FF FF 48 8B 3D ?? ?? ?? 00 48 89 DE 41 FF D5 48 8B 35 ?? ?? ?? 00 31 D2 48 89 C7 41 FF D5 48 89 85 70 FE FF FF 49 8B 3C 24";
    } else if (os_version.minorVersion == 13) {
        return "?? ?? ?? 00 48 8B 38 48 8B B5 E0 FD FF FF 4C 8B BD B8 FE FF FF 4C 89 FA 41 FF D5 48 89 C7 E8 ?? ?? ?? 00 49 89 C5 4C 89 EF 48 8B B5 80 FE FF FF FF 15 ?? ?? ?? 00 48 89 C7 E8 ?? ?? ?? 00 48 89 C3 48 89 9D C8 FE FF FF 4C 89 EF 48 8B 05 ?? ?? ?? 00";
    }
    return NULL;
}

const char *get_dppm_pattern(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return "?? ?? ?? 00 48 89 C6 E8 ?? ?? ?? 00 4D 85 F6 0F 84 ?? 06 00 00 48 8B 3D ?? ?? ?? 00 48 8B 35 ?? ?? ?? 00 FF 15 ?? ?? ?? 00 48 89 C7 E8 ?? ?? ?? 00 48 89 85 08 FF FF FF C7 85 1C FF FF FF 00 00 00 00 E8 ?? ?? ?? 00 48 8D 75 B0 89 C7 E8 ?? ?? ?? ??";
    } else if (os_version.minorVersion == 15) {
        return "?? ?? ?? 00 48 89 C6 E8 ?? ?? ?? 00 4D 85 F6 0F 84 ?? 06 00 00 48 8B 3D ?? ?? ?? 00 48 8B 35 ?? ?? ?? 00 FF 15 ?? ?? ?? 00 48 89 C7 E8 ?? ?? ?? 00 48 89 85 08 FF FF FF C7 85 1C FF FF FF 00 00 00 00 E8 ?? ?? ?? 00 48 8D 75 B0 89 C7 E8 ?? ?? ?? 00";
    } else if (os_version.minorVersion == 14) {
        return "?? ?? ?? 00 48 89 C6 E8 ?? ?? 3C 00 4D 85 FF 0F 84 AC 06 00 00 48 8B 3D ?? 00 4C 00 48 8B 35 ?? A4 4B 00 FF 15 ?? ?? 43 00 48 89 C7 E8 ?? 6B 3C 00 48 89 85 10 FF FF FF C7 85 1C FF FF FF 00 00 00 00 E8 ?? 69 3C 00 48 8D 75 B0 89 C7 E8 ?? 6A 3C 00";
    } else if (os_version.minorVersion == 13) {
        return "?? ?? ?? 00 4C 89 FE E8 99 6D 4A 00 4D 85 FF 0F 84 A2 06 00 00 48 8B 3D EB ED 5A 00 48 8B 35 AC 90 5A 00 FF 15 BE 1B 52 00 48 89 C7 E8 5C 6D 4A 00 48 89 85 30 FF FF FF 48 8D 3D A4 11 4E 00 E8 5A 8B 17 00 88 05 5D 28 5D 00 84 C0 74 14 48 8B 3D 6A";
    }
    return NULL;
}

const char *get_add_space_pattern(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        if (os_version.majorVersion == 11 && os_version.minorVersion == 2) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 18 48 BA 01 00 00 00 00 00 00 40 48 B9 F8 FF FF FF FF FF FF 00 49 8D 45 28 48 89 45 C8 4D 8B 7D 28 41 80 7D 38 01 48 89 7D D0 4C 89 6D C0 75 2E 49 89 FC 49 85 D7 74 59 4C 89 FB 48 21 CB 41 F6 C7 01 49 0F 45 DF 4C 89 FF E8 ?? ?? 0F 00 48 89 DF E8 ??";
        }
        return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 18 4C 89 6D C0 48 BA 01 00 00 00 00 00 00 40 48 B9 F8 FF FF FF FF FF FF 00 49 8D 45 28 48 89 45 D0 4D 8B 7D 28 41 80 7D 38 01 48 89 7D C8 75 2E 49 89 FC 49 85 D7 74 59 4C 89 FB 48 21 CB 41 F6 C7 01 49 0F 45 DF 4C 89 FF E8 ?? ?? 0F 00 48 89 DF E8 ??";
    } else if (os_version.minorVersion == 15) {
        if (os_version.patchVersion >= 4) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 28 4C 89 6D C0 48 89 7D C8 48 B9 01 00 00 00 00 00 00 40 49 BE F8 FF FF FF FF FF FF 00 49 8D 45 28 48 89 45 D0 4D 8B 7D 28 41 80 7D 38 01 75 2B 49 85 CF 74 59 4C 89 FB 4C 21 F3 41 F6 C7 01 49 0F 45 DF 4C 89 FF E8 ?? 26 12 00 48 89 DF E8 9B";
        } else {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 28 4C 89 6D B8 48 BA 01 00 00 00 00 00 00 40 48 B9 F8 FF FF FF FF FF FF 00 49 8D 45 28 48 89 45 C8 4D 8B 7D 28 41 80 7D 38 01 48 89 7D C0 75 5B 49 89 FC 49 85 D7 0F 84 A7 00 00 00 4C 89 FB 48 21 CB 41 F6 C7 01 49 0F 45 DF 4C 89 FF E8 ?? ??";
        }
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
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 68 48 89 4D 98 49 89 D4 49 89 F6 49 89 FF 49 89 F5 E8 ?? ?? F3 FF 49 89 C5 48 B8 01 00 00 00 00 00 00 40 49 85 C5 0F 85 DE 03 00 00 48 B8 F8 FF FF FF FF FF FF 00 4C 21 E8 48 8B 58 10 48 83 FB 02 0F 8C CD 01 00 00 4C 89 75 A0 48 8D 05 ?? ?? ?? 00 48 8B 00 4D 8B 34 07 4C 89 E3 4D 8B 64 07";
    } else if (os_version.minorVersion == 15) {
        if (os_version.patchVersion >= 4) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 48 48 89 4D A0 49 89 D4 49 89 F5 49 89 FE E8 FD 7B F1 FF 48 89 C3 48 B8 01 00 00 00 00 00 00 40 48 85 C3 0F 85 23 03 00 00 48 B8 F8 FF FF FF FF FF FF 00 48 21 D8 4C 8B 78 10 49 83 FF 02 0F 8C 3F 01 00 00 4C 89 6D A8 48 89 5D B8 48 8D 05 ??";
        } else {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC B8 00 00 00 49 89 CF 48 89 55 80 49 89 F5 48 89 7D B8 48 BB F8 FF FF FF FF FF FF 00 E8 ?? ?? F0 FF 49 89 C4 48 B8 01 00 00 00 00 00 00 40 49 85 C4 0F 85 AA 05 00 00 4C 21 E3 4C 8B 73 10 49 83 FE 02 0F 8C B6 02 00 00 4C 89 7D 90 4C 89 AD 50 FF FF FF 4C 89 65 88 48 8D 05 ?? ?? ?? 00 48 8B";
        }
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
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 48 4C 89 E9 41 89 D5 49 89 F6 49 89 FF 48 8D 1D ?? ?? ?? 00 48 8B 03 4C 8B 24 07 4C 89 E7 48 89 4D A0 48 89 CE E8 ?? ?? 00 00 48 85 C0 74 27 48 8D 0D ?? ?? ?? 00 80 39 01 48 89 55 C8 48 89 45 A8 75 1A 48 89 C7 4C 89 F6 49 89 D5 E8 ?? ?? F4";
    } else if (os_version.minorVersion == 15) {
        if (os_version.patchVersion >= 4) {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 48 4C 89 E9 41 89 D5 49 89 F6 49 89 FF 48 8D 1D ?? ?? 14 00 48 8B 03 4C 8B 24 07 4C 89 E7 48 89 4D A0 48 89 CE E8 E6 BE 00 00 48 85 C0 74 27 48 8D 0D ?? D8 15 00 80 39 01 48 89 55 C8 48 89 45 A8 75 1A 48 89 C7 4C 89 F6 49 89 D5 E8 3F D8 F2";
        } else {
            return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 48 4C 89 E9 41 89 D5 49 89 F4 49 89 FE 48 8D 1D ?? ?? 15 00 48 8B 03 4C 8B 3C 07 4C 89 FF 48 89 4D 98 48 89 CE E8 ?? ?? 00 00 48 89 55 D0 48 89 45 C8 48 85 C0 0F 84 97 03 00 00 48 8D 05 ?? ?? ?? 00 80 38 01 75 33 4C 8B 7D D0 4D 89 FD 49 83 C5 28 48 8B 5D C8 48 89 DF E8 ?? ?? ?? FF 48 89";
        }
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
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 58 48 8B 05 ?? ?? ?? 00 48 8B 00 48 89 45 D0 85 F6 0F 84 ?? 02 00 00 41 89 F5 49 89 FF 49 89 FE 49 C1 EE 20 48 8D 75 AF C6 06 00 E8 ?? ?? 02 00 48 8B 3D ?? ?? ?? 00 BE 01 00 00 00 E8 ?? ?? ?? 00 84 C0 74 59 0F B6 5D AF 4C 8D 45";
    } else if (os_version.minorVersion == 15) {
        return "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 58 48 8B 05 ?? ?? ?? 00 48 8B 00 48 89 45 D0 85 F6 0F 84 ?? 02 00 00 41 89 F5 49 89 FF 49 89 FE 49 C1 EE 20 48 8D 75 AF C6 06 00 E8 ?? ?? 02 00 48 8B 3D ?? ?? ?? 00 BE 01 00 00 00 E8 ?? ?? ?? 00 84 C0 74 59 0F B6 5D AF 4C 8D 45";
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

#define asm__call_add_space(v0,v1,func) \
    __asm__("mov x0, %0\n""mov x20, %1\n" : :"r"(v0), "r"(v1) :"x0", "x20"); ((void (*)())(func))();

#define asm__call_move_space(v0,v1,v2,v3,func) \
    __asm__("mov x0, %0\n""mov x1, %1\n""mov x2, %2\n""mov x20, %3\n" : :"r"(v0), "r"(v1), "r"(v2), "r"(v3) :"x0", "x1", "x2", "x20"); ((void (*)())(func))();

uint64_t get_dock_spaces_offset(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return 0xB200;
    }

    return 0;
}

uint64_t get_dppm_offset(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return 0x9D00;
    }

    return 0;
}

uint64_t get_add_space_offset(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return 0x220000;
    }

    return 0;
}

uint64_t get_remove_space_offset(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return 0x2E0000;
    }

    return 0;
}

uint64_t get_move_space_offset(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return 0x2D0000;
    }

    return 0;
}

uint64_t get_set_front_window_offset(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return 0x4F000;
    }

    return 0;
}

const char *get_dock_spaces_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return "55 21 00 90 B5 02 33 91 A0 02 40 F9 C8 1F 00 F0 01 C1 47 F9 E2 03 1B AA B1 54 0C 94 FD 03 1D AA DB 54 0C 94 E0 13 00 F9 08 20 00 B0 00 25 40 F9 63 54 0C 94 E8 1F 00 90 13 F1 40 F9 E1 03 13 AA 02 00 80 D2 A6 54 0C 94 E0 27 00 F9 A0 02 40 F9 08 20 00 90 01 7D 42 F9";
    }

    return NULL;
}

const char *get_dppm_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return "40 21 00 D0 00 00 35 91 E1 03 13 AA 62 5A 0C 94 D3 30 00 B4 08 20 00 F0 00 01 42 F9 E8 1F 00 B0 19 05 47 F9 E1 03 19 AA 1F 5A 0C 94 FD 03 1D AA 49 5A 0C 94 F4 03 00 AA FF 6F 01 B9 22 59 0C 94 A1 43 02 D1 88 59 0C 94 15 21 00 D0 00 01 00 35 C1 1A 00 F0 21 F4 1F 91";
    }

    return NULL;
}

const char *get_add_space_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return "7F 23 03 D5 FF C3 01 D1 E1 03 1E AA 16 32 00 94 FE 03 01 AA FD 7B 06 A9 FD 83 01 91 F5 03 14 AA F3 03 00 AA 89 E2 40 39 96 16 40 F9 C8 FE 7E D3 3F 05 00 71 A1 00 00 54 88 14 00 B5 C8 E2 7D 92 17 09 40 F9 3B 00 00 14 E8 14 00 B5 C8 E2 7D 92 17 09 40 F9 DB 31 00 94 F7 05 00 B4 F8 06 00 F1 26 15 00 54 DA 0A 42 F2 E1 17 9F 1A E0 03 18 AA E2 03 16 AA 9D FF FD 97 9A 14 00 B5 C8 0E 18 8B 7B 32 00 94 F4 03 00 AA";
    }

    return NULL;
}

const char *get_remove_space_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return "7F 23 03 D5 FF 83 03 D1 FC 6F 08 A9 FA 67 09 A9 F8 5F 0A A9 F6 57 0B A9 F4 4F 0C A9 FD 7B 0D A9 FD 43 03 91 F7 03 03 AA F6 03 02 AA F5 03 01 AA F3 03 00 AA F4 03 01 AA B4 32 FD 97 F4 03 00 AA 08 FC 7E D3 88 2D 00 B5 88 E2 7D 92 00 09 40 F9 1F 08 00 F1 AB 0E 00 54 F4 5F 03 A9 F5 17 00 F9 08 0A 00 D0 1F 20 03 D5 08 7D 47 F9 68 02 08 8B 14 69 40 A9 88 0A 00 D0 1F 20 03 D5 00 A1 46 F9 48 09 00 D0 01 5D 42 F9 F3 0B 00 F9 E2 03 13 AA";
    }

    return NULL;
}

const char *get_move_space_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return "7F 23 03 D5 E3 03 1E AA 02 5F 00 94 FE 03 03 AA FD 7B 06 A9 FD 83 01 91 F6 03 14 AA F4 03 02 AA FA 03 01 AA FB 03 00 AA 97 0A 00 B0 F7 02 3C 91 E8 02 40 F9 18 68 68 F8 E0 03 18 AA E1 03 16 AA 4C 2C 00 94 80 01 00 B4 F3 03 00 AA F5 03 01 AA 08 0B 00 B0 08 A1 26 91 08 01 40 39 1F 05 00 71 E1 00 00 54 B1 62 00 94 C7 36 01 94 1C 62 00 94 CF 00 00 14 14 00 80 52 D2 00 00 14 1A 01 00 B4 E8 02 40 F9 40 6B 68 F8 E1 03 16 AA F4 2C 00 94";
    }

    return NULL;
}

const char *get_set_front_window_pattern(NSOperatingSystemVersion os_version) {
    if (os_version.majorVersion == 12) {
        return "7F 23 03 D5 FF 43 02 D1 F6 57 06 A9 F4 4F 07 A9 FD 7B 08 A9 FD 03 02 91 A8 1A 00 90 08 41 44 F9 08 01 40 F9 A8 83 1D F8 C1 10 00 34 F3 03 01 AA F4 03 00 AA 15 FC 60 D3 FF BF 00 39 E1 BF 00 91 72 A1 00 94 A0 1A 00 90 00 04 45 F9 21 00 80 52 F2 41 0B 94 20 0A 00 36 F6 BF 40 39 E8 83 06 32 E8 53 06 29 08 80 80 52 E8 73 00 79 F5 A3 03 B8 E8 7F 00 79 F3 43 00 B9 E8 8B 00 79 F6 63 04 B8 A0 03 D8 10";
    }

    return NULL;
}

uint64_t get_dock_spaces_offset(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x0;
    }
    return 0;
}

uint64_t get_dppm_offset(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x0;
    }
    return 0;
}

uint64_t get_add_space_offset(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x0;
    }
    return 0;
}

uint64_t get_remove_space_offset(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x0;
    }
    return 0;
}

uint64_t get_move_space_offset(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x0;
    }
    return 0;
}

uint64_t get_set_front_window_offset(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return 0x0;
    }
    return 0;
}

const char *get_dock_spaces_pattern(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return NULL;
    }
    return NULL;
}

const char *get_dppm_pattern(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return NULL;
    }
    return NULL;
}

const char *get_add_space_pattern(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return NULL;
    }
    return NULL;
}

const char *get_remove_space_pattern(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return NULL;
    }
    return NULL;
}

const char *get_move_space_pattern(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return NULL;
    }
    return NULL;
}

const char *get_set_front_window_pattern(NSOperatingSystemVersion os_version) {
    if ((os_version.majorVersion == 11) || (os_version.majorVersion == 10 && os_version.minorVersion == 16)) {
        return NULL;
    }
    return NULL;
}

static struct mach_header_64 *macho_find_image_header(char *target_name, uint64_t *slide)
{
    int image_count = _dyld_image_count();

    for (int i = 0; i < image_count; ++i) {
        const char *image_name = _dyld_get_image_name(i);
        if (!image_name) continue;

        if (string_equals(image_name, target_name)) {
            *slide = _dyld_get_image_vmaddr_slide(i);
            return (struct mach_header_64 *) _dyld_get_image_header(i);
        }
    }

    return NULL;
}

static struct segment_command_64 *macho_find_linkedit_segment(struct mach_header_64 *header)
{
    uint64_t offset = sizeof(struct mach_header_64);

    for (int i = 0; i < header->ncmds; ++i) {
        struct load_command *cmd = (struct load_command *)(((uint8_t *) header) + offset);

        if (cmd->cmd == LC_SEGMENT_64) {
            struct segment_command_64 *segment = (struct segment_command_64 *) cmd;
            if (string_equals(segment->segname, SEG_LINKEDIT)) {
                return segment;
            }
        }

        offset += cmd->cmdsize;
    }

    return NULL;
}

static struct symtab_command *macho_find_symtab_command(struct mach_header_64 *header)
{
    uint64_t offset = sizeof(struct mach_header_64);

    for (int i = 0; i < header->ncmds; ++i) {
        struct load_command *cmd = (struct load_command *)(((uint8_t *) header) + offset);

        if (cmd->cmd == LC_SYMTAB) {
            return (struct symtab_command *) cmd;
        }

        offset += cmd->cmdsize;
    }

    return NULL;
}

void *macho_find_symbol(char *target_image, char *target_symbol)
{
    uint64_t slide = 0;
    struct mach_header_64 *header = macho_find_image_header(target_image, &slide);
    if (!header) return NULL;

    struct segment_command_64 *linkedit_segment = macho_find_linkedit_segment(header);
    if (!linkedit_segment) return NULL;

    struct symtab_command *symtab_command = macho_find_symtab_command(header);
    if (!symtab_command) return NULL;

    int symbol_count = symtab_command->nsyms;
    void *symbol_str = (void *)(linkedit_segment->vmaddr - linkedit_segment->fileoff) + symtab_command->stroff + slide;
    void *symbol_sym = (void *)(linkedit_segment->vmaddr - linkedit_segment->fileoff) + symtab_command->symoff + slide;

    for (int i = 0; i < symbol_count; ++i) {
        struct nlist_64 *list = (void *) symbol_sym + (i * sizeof(struct nlist_64));
        char *symbol_name = (char *) symbol_str + list->n_un.n_strx;
        if (string_equals(symbol_name, target_symbol)) {
            return (void *)(list->n_value + slide);
        }
    }

    return NULL;
}

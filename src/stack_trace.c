#include "stack_trace.h"

#include <string.h>

#include <drivers/console.h>
#include <kernaux/multiboot2.h>

#include <kernaux/macro/packing_start.run>

struct SectionEntry {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t vaddr;
    uint32_t file_offset;
    uint32_t file_size;
    uint32_t link;
    uint32_t info;
    uint32_t alignment;
    uint32_t ent_size;
}
KERNAUX_PACKED;

#include <kernaux/macro/packing_end.run>

void stack_trace_init(
    const struct KernAux_Multiboot2_Info *const multiboot2_info
) {
    const struct KernAux_Multiboot2_ITag_ELFSymbols *const elf_symbols_tag =
        (const struct KernAux_Multiboot2_ITag_ELFSymbols*)
        KernAux_Multiboot2_Info_first_tag_with_type(
            multiboot2_info,
            KERNAUX_MULTIBOOT2_ITAG_ELF_SYMBOLS
        );

    if (!elf_symbols_tag) {
        drivers_console_puts("ELF symbols tag not found");
        return;
    }

    const struct SectionEntry *const section_headers =
        // FIXME: GRUB 2 doesn't conform the spec!
        // https://www.mail-archive.com/grub-devel@gnu.org/msg30790.html
        // (const struct SectionEntry*)KERNAUX_MULTIBOOT2_DATA(elf_symbols_tag);
        (const struct SectionEntry*)(&((uint8_t*)elf_symbols_tag)[20]);

    drivers_console_puts("ELF symbols tag:");
    KernAux_Multiboot2_ITag_ELFSymbols_print(
        elf_symbols_tag,
        drivers_console_printf
    );
    drivers_console_printf("  data: 0x%p\n", (void*)section_headers);
    drivers_console_putc('\n');

    const struct SectionEntry *const shstrtab =
        // FIXME: GRUB 2 doesn't conform the spec!
        // https://www.mail-archive.com/grub-devel@gnu.org/msg30790.html
        // &section_headers[elf_symbols_tag->shndx];
        &section_headers[(uint32_t)(((uint8_t*)elf_symbols_tag)[16])];

    if (shstrtab == section_headers) return;

    size_t debug_info_index   = 0;
    size_t debug_abbrev_index = 0;
    size_t debug_str_index    = 0;

    for (size_t index = 0; index < elf_symbols_tag->num; ++index) {
        const struct SectionEntry *const section_header =
            &section_headers[index];

        const char *const section_name =
            &((const char*)shstrtab->vaddr)[section_header->name];

        drivers_console_printf("section %lu: %s\n", index, section_name);

        if (strcmp(section_name, ".debug_info") == 0) {
            debug_info_index = index;
        } else if (strcmp(section_name, ".debug_abbrev") == 0) {
            debug_abbrev_index = index;
        } else if (strcmp(section_name, ".debug_str") == 0) {
            debug_str_index = index;
        }
    }

    drivers_console_putc('\n');

    drivers_console_printf(".debug_info:   %lu\n", debug_info_index);
    drivers_console_printf(".debug_abbrev: %lu\n", debug_abbrev_index);
    drivers_console_printf(".debug_str:    %lu\n", debug_str_index);
    drivers_console_putc('\n');

    if (!debug_info_index || !debug_abbrev_index || !debug_str_index) return;
/*
    const struct SectionEntry *const debug_info =
        &section_headers[debug_info_index];
    const struct SectionEntry *const debug_abbrev =
        &section_headers[debug_abbrev_index];
    const struct SectionEntry *const debug_str =
        &section_headers[debug_str_index];
*/
}

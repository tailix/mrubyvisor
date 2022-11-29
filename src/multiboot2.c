#include <kernaux/multiboot2.h>

#include <kernaux/macro/packing_start.run>

__attribute__((
    section(".multiboot2"),
    used,
    aligned(KERNAUX_MULTIBOOT2_HEADER_ALIGN)
))
const struct {
    struct KernAux_Multiboot2_Header header;
    struct KernAux_Multiboot2_HTag_None tag_none;
}
KERNAUX_PACKED
multiboot2 = {
    .header = {
        .magic = KERNAUX_MULTIBOOT2_HEADER_MAGIC,
        .arch = KERNAUX_MULTIBOOT2_HEADER_ARCH_I386,
        .total_size = sizeof(multiboot2),
        .checksum = KERNAUX_MULTIBOOT2_HEADER_CHECKSUM(
            KERNAUX_MULTIBOOT2_HEADER_ARCH_I386,
            sizeof(multiboot2)
        ),
    },
    .tag_none = {
        .base = {
            .type = KERNAUX_MULTIBOOT2_HTAG_NONE,
            .flags = 0,
            .size = sizeof(multiboot2.tag_none),
        },
    },
};

#include <kernaux/macro/packing_end.run>

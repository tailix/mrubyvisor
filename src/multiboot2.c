#include <stdint.h>

#include <kernaux/multiboot2.h>

#include <kernaux/macro/packing_start.run>

__attribute__((
    section(".multiboot2"),
    aligned(KERNAUX_MULTIBOOT2_HEADER_ALIGN),
    used
))
const struct {
    struct KernAux_Multiboot2_Header header;

    struct {
        struct KernAux_Multiboot2_HTag_InfoReq tag;
        uint32_t mbi_tag_types[1];
    } KERNAUX_PACKED tag_info_req;
    uint8_t _align1[4];

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
    .tag_info_req = {
        .tag = {
            .base = {
                .type = KERNAUX_MULTIBOOT2_HTAG_INFO_REQ,
                .flags = KERNAUX_MULTIBOOT2_HTAG_BASE_FLAG_OPTIONAL,
                .size = sizeof(multiboot2.tag_info_req),
            },
        },
        .mbi_tag_types = {
            KERNAUX_MULTIBOOT2_ITAG_ELF_SYMBOLS,
        },
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

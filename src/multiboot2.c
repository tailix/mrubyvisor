#include <stdint.h>

#include <kernaux/multiboot2.h>

// TODO: move these macros to libkernaux

#define HFIELDS_COMMON(whole_name, htag_name, type) \
    struct {                                             \
        struct KernAux_Multiboot2_HTag_##type htag_name; \
    } KERNAUX_PACKED whole_name;

#define HFIELDS_INFO_REQ_ODD(whole_name, htag_name, data_name, align_name, \
                             mbi_tag_types_count)                          \
    struct {                                              \
        struct KernAux_Multiboot2_HTag_InfoReq htag_name; \
        uint32_t data_name[mbi_tag_types_count];          \
    } KERNAUX_PACKED whole_name;                          \
    uint8_t align_name[4];

#define HFIELDS_INFO_REQ_EVEN(whole_name, htag_name, data_name, align_name, \
                              mbi_tag_types_count)                          \
    struct {                                              \
        struct KernAux_Multiboot2_HTag_InfoReq htag_name; \
        uint32_t data_name[mbi_tag_types_count];          \
    } KERNAUX_PACKED whole_name;

#include <kernaux/macro/packing_start.run>

__attribute__((
    section(".multiboot2"),
    aligned(KERNAUX_MULTIBOOT2_HEADER_ALIGN),
    used
))
const struct {
    struct KernAux_Multiboot2_Header header;
    HFIELDS_INFO_REQ_ODD(tag_info_req, tag, mbi_tag_types, _align1, 1)
    HFIELDS_COMMON      (tag_none,     tag, None)
}
KERNAUX_PACKED
multiboot2_header = {
    .header = {
        .magic = KERNAUX_MULTIBOOT2_HEADER_MAGIC,
        .arch = KERNAUX_MULTIBOOT2_HEADER_ARCH_I386,
        .total_size = sizeof(multiboot2_header),
        .checksum = KERNAUX_MULTIBOOT2_HEADER_CHECKSUM(
            KERNAUX_MULTIBOOT2_HEADER_ARCH_I386,
            sizeof(multiboot2_header)
        ),
    },
    .tag_info_req = {
        .tag = {
            .base = {
                .type = KERNAUX_MULTIBOOT2_HTAG_INFO_REQ,
                .flags = KERNAUX_MULTIBOOT2_HTAG_BASE_FLAG_OPTIONAL,
                .size = sizeof(multiboot2_header.tag_info_req),
            },
        },
        .mbi_tag_types = {
            KERNAUX_MULTIBOOT2_ITAG_ELF_SYMBOLS,
        },
    },
    .tag_none = {
        .tag = {
            .base = {
                .type = KERNAUX_MULTIBOOT2_HTAG_NONE,
                .flags = 0,
                .size = sizeof(multiboot2_header.tag_none),
            },
        },
    },
};

#include <kernaux/macro/packing_end.run>

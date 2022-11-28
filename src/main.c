#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <kernaux/assert.h>
#include <kernaux/drivers/console.h>
#include <kernaux/drivers/shutdown.h>
#include <kernaux/generic/malloc.h>
#include <kernaux/free_list.h>
#include <kernaux/libc.h>
#include <kernaux/multiboot2.h>

#include <mruby.h>
#include <mruby/presym.h>
#include <mruby/string.h>

#define PANIC(msg) (assert_cb(__FILE__, __LINE__, msg))
#define ASSERT(cond) ((cond) ? (void)0 : PANIC(#cond))

static struct KernAux_FreeList allocator;
static uint8_t memory[1024 * 128]; // 128 KiB

static mrb_state *mrb = NULL;

static void panic(const char *str);
static void assert_cb(const char *file, int line, const char *str);

static void *my_calloc(size_t nmemb, size_t size);
static void my_free(void *ptr);
static void *my_malloc(size_t size);
static void *my_realloc(void *ptr, size_t size);

void main(
    const uint32_t multiboot2_info_magic,
    const struct KernAux_Multiboot2_Info *const multiboot2_info
) {
    kernaux_assert_cb = assert_cb;

    KernAux_FreeList_init(&allocator, NULL);
    KernAux_FreeList_add_zone(&allocator, memory, sizeof(memory));

    kernaux_libc.calloc  = my_calloc;
    kernaux_libc.free    = my_free;
    kernaux_libc.malloc  = my_malloc;
    kernaux_libc.realloc = my_realloc;

    {
        char *const hello = malloc(100);
        strcpy(hello, "Hello, World! The allocator works!\n");
        kernaux_drivers_console_print(hello);
        free(hello);
    }

    ASSERT(mrb = mrb_open());

    {
        mrb_value hello = mrb_str_new_lit(mrb, "Hello, World! Ruby works!\n");
        kernaux_drivers_console_print(RSTRING_CSTR(mrb, hello));
    }

    {
        const char *const source =
            "begin                                                \n"
            "   hello = 'Hello, World'                            \n"
            "   works = 'Ruby eval works'                         \n"
            "   s = [hello, works].map { |s| \"#{s}!\" }.join ' ' \n"
            "   \"#{s}\\n\"                                       \n"
            "end                                                  \n";

        mrb_value program = mrb_str_new_cstr(mrb, source);
        mrb_value hello =
            mrb_funcall_id(mrb, mrb_nil_value(), MRB_SYM(eval), 1, program);
        kernaux_drivers_console_print(RSTRING_CSTR(mrb, hello));
    }

    {
        const char *cmdline = "";

        const struct KernAux_Multiboot2_ITag_BootCmdLine *const cmdline_tag =
            KernAux_Multiboot2_Info_first_tag_with_type(
                multiboot2_info,
                KERNAUX_MULTIBOOT2_ITAG_BOOT_CMD_LINE
            );

        if (cmdline_tag) cmdline = KERNAUX_MULTIBOOT2_DATA(cmdline_tag);

        kernaux_drivers_console_printf("cmdline: %s\n", cmdline);
    }

    for (
        const struct KernAux_Multiboot2_ITag_Module *module_tag =
            KernAux_Multiboot2_Info_first_tag_with_type(
                multiboot2_info,
                KERNAUX_MULTIBOOT2_ITAG_MODULE
            );
        module_tag;
        module_tag =
            KernAux_Multiboot2_Info_tag_with_type_after(
                multiboot2_info,
                KERNAUX_MULTIBOOT2_ITAG_MODULE,
                module_tag
            )
    ) {
        const char *const cmdline = KERNAUX_MULTIBOOT2_DATA(module_tag);
        const char *const source = module_tag->mod_start;
        const size_t size = module_tag->mod_end - module_tag->mod_start;

        kernaux_drivers_console_print("========================================\n");
        kernaux_drivers_console_printf("module cmdline: %s\n", cmdline);
        kernaux_drivers_console_print("----------------------------------------\n");
        kernaux_drivers_console_write(source, size);
        if (source[size - 1] != '\n') kernaux_drivers_console_putc('\n');
    }
}

void assert_cb(const char *const file, const int line, const char *const str)
{
    kernaux_drivers_console_printf("panic: %s:%u: \"%s\"\n", file, line, str);
    kernaux_drivers_shutdown_poweroff();
}

void *my_calloc(size_t nmemb, size_t size)
{
    return KernAux_Malloc_calloc(&allocator.malloc, nmemb, size);
}

void my_free(void *ptr)
{
    KernAux_Malloc_free(&allocator.malloc, ptr);
}

void *my_malloc(size_t size)
{
    return KernAux_Malloc_malloc(&allocator.malloc, size);
}

void *my_realloc(void *ptr, size_t size)
{
    return KernAux_Malloc_realloc(&allocator.malloc, ptr, size);
}

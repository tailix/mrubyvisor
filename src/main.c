#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <kernaux/drivers/console.h>
#include <kernaux/generic/malloc.h>
#include <kernaux/free_list.h>
#include <kernaux/libc.h>
#include <kernaux/multiboot2.h>

#include <mruby.h>
#include <mruby/string.h>

#define DEBUG(call, result) \
    kernaux_drivers_console_printf("[%s]: %s\n", ((result) ? " OK " : "FAIL"), (call))

static struct KernAux_FreeList allocator;
static uint8_t memory[1024 * 128]; // 128 KiB

static mrb_state *mrb = NULL;

static void *my_calloc(size_t nmemb, size_t size);
static void my_free(void *ptr);
static void *my_malloc(size_t size);
static void *my_realloc(void *ptr, size_t size);

void main(
    const uint32_t multiboot2_info_magic,
    const struct KernAux_Multiboot2_Info *const multiboot2_info
) {
    KernAux_FreeList_init(&allocator, NULL);
    KernAux_FreeList_add_zone(&allocator, memory, sizeof(memory));

    kernaux_libc.calloc  = my_calloc;
    kernaux_libc.free    = my_free;
    kernaux_libc.malloc  = my_malloc;
    kernaux_libc.realloc = my_realloc;

    {
        char *const hello = malloc(100);
        DEBUG("malloc(100)", hello);
        if (hello) {
            strcpy(hello, "Hello, World!\n");
            kernaux_drivers_console_print(hello);
            free(hello);
        }
    }

    {
        char *const hello = realloc(NULL, 100);
        DEBUG("realloc(NULL, 100)", hello);
        if (hello) {
            strcpy(hello, "Hello, World!\n");
            kernaux_drivers_console_print(hello);
            free(hello);
        }
    }

    kernaux_drivers_console_printf("sizeof(mrb_state): %lu\n", sizeof(mrb_state));

    {
        mrb_state *tmp = malloc(sizeof(mrb_state));
        DEBUG("malloc(sizeof(mrb_state))", tmp);
        if (tmp) free(tmp);
    }

    {
        mrb_state *tmp = realloc(NULL, sizeof(mrb_state));
        DEBUG("realloc(NULL, sizeof(mrb_state))", tmp);
        if (tmp) free(tmp);
    }

    mrb = mrb_open();
    DEBUG("mrb_open()", mrb);

    {
        mrb_value hello = mrb_str_new_lit(mrb, "Hello, World!\n");
        kernaux_drivers_console_print(RSTRING_CSTR(mrb, hello));
    }
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

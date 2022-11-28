#include "libc.h"

#include <stdlib.h>

#include <kernaux/generic/malloc.h>
#include <kernaux/free_list.h>
#include <kernaux/libc.h>

static struct KernAux_FreeList allocator;
static uint8_t memory[1024 * 128]; // 128 KiB

__attribute__((noreturn))
static void my_abort();
__attribute__((noreturn))
static void my_exit(int status);

static void *my_calloc(size_t nmemb, size_t size);
static void my_free(void *ptr);
static void *my_malloc(size_t size);
static void *my_realloc(void *ptr, size_t size);

void libc_init()
{
    KernAux_FreeList_init(&allocator, NULL);
    KernAux_FreeList_add_zone(&allocator, memory, sizeof(memory));

    kernaux_libc.abort   = my_abort;
    kernaux_libc.exit    = my_exit;

    kernaux_libc.calloc  = my_calloc;
    kernaux_libc.free    = my_free;
    kernaux_libc.malloc  = my_malloc;
    kernaux_libc.realloc = my_realloc;
}

void my_abort()
{
    exit(EXIT_FAILURE);
}

void my_exit(const int status)
{
    kernaux_drivers_console_printf("exit: %d\n", status);
    kernaux_drivers_shutdown_poweroff();
}

void *my_calloc(size_t nmemb, size_t size)
{
    void *const result = KernAux_Malloc_calloc(&allocator.malloc, nmemb, size);
    if (result) {
        kernaux_drivers_console_printf("calloc(%lu, %lu) = %p\n", nmemb, size, result);
    } else if (nmemb * size != 0) {
        kernaux_drivers_console_printf("!calloc(%lu, %lu)\n", nmemb, size);
    }
    return result;
}

void my_free(void *ptr)
{
    kernaux_drivers_console_printf("free(%p)\n", ptr);
    KernAux_Malloc_free(&allocator.malloc, ptr);
}

void *my_malloc(size_t size)
{
    void *const result = KernAux_Malloc_malloc(&allocator.malloc, size);
    if (result) {
        kernaux_drivers_console_printf("malloc(%lu) = %p\n", size, result);
    } else if (size != 0) {
        kernaux_drivers_console_printf("!malloc(%lu)\n", size);
    }
    return result;
}

void *my_realloc(void *ptr, size_t size)
{
    void *const result = KernAux_Malloc_realloc(&allocator.malloc, ptr, size);
    if (result) {
        kernaux_drivers_console_printf("realloc(%p, %lu) = %p\n", ptr, size, result);
    } else if (size != 0) {
        kernaux_drivers_console_printf("!realloc(%p, %lu)\n", ptr, size);
    }
    return result;
}

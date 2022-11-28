#include "libc.h"
#include "logger.h"

#include <stdlib.h>

#include <kernaux/drivers/console.h>
#include <kernaux/drivers/shutdown.h>
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
    logger_exit(status);
    kernaux_drivers_shutdown_poweroff();

    // TODO: libkernaux shutdown poweroff noreturn
    volatile int x = 0;
    for (;;) ++x;
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

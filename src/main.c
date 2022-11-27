#include <stdint.h>

#include <kernaux/drivers/console.h>
#include <kernaux/multiboot2.h>

void main(
    const uint32_t multiboot2_info_magic,
    const struct KernAux_Multiboot2_Info *const multiboot2_info
) {
    kernaux_drivers_console_print("Hello, World!\n");
}

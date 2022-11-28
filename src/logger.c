#include "logger.h"

#include <kernaux/drivers/console.h>

void logger_assert(
    const char *const file,
    const int line,
    const char *const str
) {
    kernaux_drivers_console_printf("panic: %s:%u: \"%s\"\n", file, line, str);
}

void logger_exit(const int status)
{
    kernaux_drivers_console_printf("exit: %d\n", status);
}

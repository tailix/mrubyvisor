#include "logger.h"

#include <drivers/console.h>

void logger_assert(
    const char *const file,
    const int line,
    const char *const str
) {
    drivers_console_printf("panic: %s:%u: \"%s\"\n", file, line, str);
}

void logger_exit(const int status)
{
    drivers_console_printf("exit: %d\n", status);
}

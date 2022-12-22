#include "logger.h"
#include "panic.h"

#include <drivers/shutdown.h>
#include <kernaux/runtime.h>

void panic_init()
{
    kernaux_assert_cb = assert;
}

void assert(const char *const file, const int line, const char *const str)
{
    logger_assert(file, line, str);
    drivers_shutdown_poweroff();
}

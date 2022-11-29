#include "logger.h"
#include "panic.h"

#include <kernaux/assert.h>
#include <kernaux/drivers/shutdown.h>

void panic_init()
{
    kernaux_assert_cb = assert;
}

void assert(const char *const file, const int line, const char *const str)
{
    logger_assert(file, line, str);
    kernaux_drivers_shutdown_poweroff();
}

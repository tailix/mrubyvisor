#ifndef INCLUDED_LOGGER
#define INCLUDED_LOGGER

void logger_assert(const char *file, int line, const char *str);
void logger_exit(int status);

#endif

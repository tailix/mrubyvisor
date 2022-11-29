#ifndef INCLUDED_PANIC
#define INCLUDED_LOGGERPANIC

#define PANIC(msg) (assert(__FILE__, __LINE__, msg))
#define ASSERT(cond) ((cond) ? (void)0 : PANIC(#cond))

void panic_init();

void assert(const char *file, int line, const char *str);

#endif

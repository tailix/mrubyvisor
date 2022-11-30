#ifndef INCLUDED_STACK_TRACE
#define INCLUDED_STACK_TRACE

#include <kernaux/multiboot2.h>

void stack_trace_init(const struct KernAux_Multiboot2_Info *multiboot2_info);

#endif

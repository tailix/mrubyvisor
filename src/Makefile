AR     = $(CCPREFIX)ar
AS     = $(CCPREFIX)as
CC     = $(CCPREFIX)gcc
LD     = $(CCPREFIX)ld
RANLIB = $(CCPREFIX)ranlib

MRUBYVISOR = mrubyvisor.multiboot2

CFLAGS = \
	-std=gnu99           \
	-pedantic            \
	-Wall                \
	-Wextra              \
	-ffreestanding       \
	-fno-builtin         \
	-fno-stack-protector \
	-I$(DEST)/include

CPPFLAGS =        \
	$(MRUBY_FLAGS)  \
	-DKERNAUX_DEBUG \

OBJS =            \
	libc.c.o        \
	logger.c.o      \
	main.c.o        \
	multiboot2.c.o  \
	panic.c.o       \
	stack_trace.c.o \
	start.S.o

all: $(MRUBYVISOR)

clean: 
	rm -f $(MRUBYVISOR) $(OBJS)

$(MRUBYVISOR): $(OBJS)
	$(CC) -T linker.ld -o $@ $^ -ffreestanding -nostdlib -lmruby -ldrivers -lkernaux -lc -lgcc -Wl,-L$(DEST)/lib
	grub-file --is-x86-multiboot2 $@

%.c.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

%.S.o: %.S
	$(AS) $< -o $@

.PHONY: $(MRUBYVISOR)

CCPREFIX = /home/kotovalexarian/repos/global/tailix/cross/root/bin/x86_64-elf-

AR     = $(CCPREFIX)ar
AS     = $(CCPREFIX)as
CC     = $(CCPREFIX)gcc
LD     = $(CCPREFIX)ld
RAKE   = rake
RANLIB = $(CCPREFIX)ranlib

ABS_REPO = $(shell pwd)

IMAGE  = image.iso
ROOTFS = rootfs
DEST   = dest

GRUBCFG    = $(ROOTFS)/boot/grub/grub.cfg
LIBKERNAUX = $(DEST)/lib/libkernaux.a
LIBMRUBY   = $(DEST)/lib/libmruby.a
MRUBYVISOR = $(ROOTFS)/boot/mrubyvisor.multiboot2

MRUBY_NAME = mrubyvisor

all: runc

runc: $(IMAGE)
	qemu-system-x86_64 -cdrom $< -serial stdio -display none

runw: $(IMAGE)
	qemu-system-x86_64 -cdrom $< -serial stdio

clean:
	rm -rf $(MRUBYVISOR) $(DEST)/*

$(IMAGE): $(GRUBCFG) $(MRUBYVISOR)
	grub-mkrescue $(ROOTFS) -o $@

$(MRUBYVISOR): $(LIBKERNAUX) $(LIBMRUBY)
	make -C src mrubyvisor.multiboot2
	cp src/mrubyvisor.multiboot2 $@

$(LIBKERNAUX):
	cd vendor/libkernaux && ./autogen.sh
	cd vendor/libkernaux && ./configure --host='x86_64-elf' --prefix='$(ABS_REPO)/$(DEST)' --enable-freestanding --with-drivers --with-libc AR='$(AR)' AS='$(AS)' CC='$(CC)' LD='$(LD)' RANLIB='$(RANLIB)'
	cd vendor/libkernaux && make
	cd vendor/libkernaux && make install

$(LIBMRUBY): $(LIBKERNAUX)
	cd vendor/mruby && $(RAKE) MRUBY_CONFIG='$(ABS_REPO)/build_config.rb' CROSS_AR='$(AR)' CROSS_CC='$(CC)' CROSS_LD='$(LD)'
	mkdir -p $(DEST)/include $(DEST)/lib
	cp vendor/mruby/build/$(MRUBY_NAME)/lib/libmruby.a $(DEST)/lib
	cp -r vendor/mruby/include/* $(DEST)/include
	cp -r vendor/mruby/build/$(MRUBY_NAME)/include/* $(DEST)/include

CCPREFIX = /home/kotovalexarian/repos/global/tailix/cross/root/bin/i386-elf-

AR     = $(CCPREFIX)ar
AS     = $(CCPREFIX)as
CC     = $(CCPREFIX)gcc
LD     = $(CCPREFIX)ld
RAKE   = rake
RANLIB = $(CCPREFIX)ranlib
QEMU   = qemu-system-i386

ABS_REPO = $(shell pwd)

DEST       = dest
IMAGE      = image.iso
MRUBY_CONF = build_config.rb
ROOTFS     = rootfs
SRC        = src

GRUBCFG    = $(ROOTFS)/boot/grub/grub.cfg
LIBKERNAUX = $(DEST)/lib/libkernaux.a
LIBMRUBY   = $(DEST)/lib/libmruby.a
MRUBYVISOR = $(ROOTFS)/boot/mrubyvisor.multiboot2

MRUBY_NAME = mrubyvisor

.PHONY: $(IMAGE) $(MRUBYVISOR)

all: runc

runc: $(IMAGE)
	$(QEMU) -cdrom $< -serial stdio -display none

runw: $(IMAGE)
	$(QEMU) -cdrom $< -serial stdio

clean: clean-src clean-dest clean-mruby clean-libkernaux

clean-src:
	$(MAKE) -C $(SRC) clean

clean-dest:
	rm -rf $(MRUBYVISOR) $(DEST)/*

clean-mruby:
	cd vendor/mruby && $(RAKE) clean

clean-libkernaux:
	$(MAKE) -C vendor/libkernaux distclean

$(IMAGE): $(GRUBCFG) $(MRUBYVISOR)
	grub-mkrescue $(ROOTFS) -o $@

$(MRUBYVISOR): $(LIBKERNAUX) $(LIBMRUBY)
	$(MAKE) -C $(SRC) mrubyvisor.multiboot2 CCPREFIX='$(CCPREFIX)' DEST='$(ABS_REPO)/$(DEST)'
	cp $(SRC)/mrubyvisor.multiboot2 $@

$(LIBKERNAUX):
	cd vendor/libkernaux && ./autogen.sh
	cd vendor/libkernaux && ./configure --host='i386-elf' --prefix='$(ABS_REPO)/$(DEST)' --enable-freestanding --enable-split-libc --with-drivers --with-libc AR='$(AR)' AS='$(AS)' CC='$(CC)' LD='$(LD)' RANLIB='$(RANLIB)'
	cd vendor/libkernaux && $(MAKE)
	cd vendor/libkernaux && $(MAKE) install

$(LIBMRUBY): $(LIBKERNAUX) $(MRUBY_CONF)
	cd vendor/mruby && $(RAKE) clean
	cd vendor/mruby && $(RAKE) MRUBY_CONFIG='$(ABS_REPO)/$(MRUBY_CONF)' CROSS_AR='$(AR)' CROSS_CC='$(CC)' CROSS_LD='$(LD)'
	mkdir -p $(DEST)/include $(DEST)/lib
	cp vendor/mruby/build/$(MRUBY_NAME)/lib/libmruby.a $(DEST)/lib
	cp -r vendor/mruby/include/* $(DEST)/include
	cp -r vendor/mruby/build/$(MRUBY_NAME)/include/* $(DEST)/include

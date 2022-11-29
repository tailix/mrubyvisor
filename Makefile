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

GRUBCFG     = $(ROOTFS)/boot/grub/grub.cfg
MRUBYVISOR  = $(ROOTFS)/boot/mrubyvisor.multiboot2
INCLUDE_DIR = $(DEST)/include
LIB_DIR     = $(DEST)/lib
LIBKERNAUX  = $(LIB_DIR)/libkernaux.a
LIBMRUBY    = $(LIB_DIR)/libmruby.a

LIBKERNAUX_ARGS =       \
	--enable-freestanding \
	--enable-split-libc   \
	--enable-debug        \
	--disable-float       \
	--with-drivers        \
	--with-libc

MRUBY_FLAGS =     \
	-DMRB_NO_BOXING \
	-DMRB_NO_FLOAT  \
	-DMRB_NO_STDIO

MRUBY_BUILD_NAME = mrubyvisor

.PHONY: $(IMAGE) $(MRUBYVISOR)

all: runc

runc: $(IMAGE)
	$(QEMU) -cdrom $< -serial stdio -display none

runw: $(IMAGE)
	$(QEMU) -cdrom $< -serial stdio

clean:      clean-src clean-dest clean-mruby clean-libkernaux
clean-most: clean-src clean-dest clean-mruby

clean-src:
	$(MAKE) -C $(SRC) clean

clean-dest:
	rm -rf $(MRUBYVISOR) $(DEST)/*

clean-mruby:
	cd vendor/mruby && $(RAKE) clean

clean-libkernaux:
	$(MAKE) -C vendor/libkernaux distclean || true

$(IMAGE): $(GRUBCFG) $(MRUBYVISOR)
	grub-mkrescue $(ROOTFS) -o $@

$(MRUBYVISOR): $(LIBKERNAUX) $(LIBMRUBY)
	$(MAKE) -C $(SRC) mrubyvisor.multiboot2 CCPREFIX='$(CCPREFIX)' DEST='$(ABS_REPO)/$(DEST)' MRUBY_FLAGS='$(MRUBY_FLAGS)'
	cp $(SRC)/mrubyvisor.multiboot2 $@

$(LIBKERNAUX):
	cd vendor/libkernaux && ./autogen.sh
	cd vendor/libkernaux && ./configure --host='i386-elf' --prefix='$(ABS_REPO)/$(DEST)' $(LIBKERNAUX_ARGS) AR='$(AR)' AS='$(AS)' CC='$(CC)' LD='$(LD)' RANLIB='$(RANLIB)'
	cd vendor/libkernaux && $(MAKE)
	cd vendor/libkernaux && $(MAKE) install

$(LIBMRUBY): $(LIBKERNAUX) $(MRUBY_CONF)
	$(MAKE) clean-mruby
	mkdir -p $(INCLUDE_DIR) $(LIB_DIR)
	cd vendor/mruby && $(RAKE) MRUBY_CONFIG='$(ABS_REPO)/$(MRUBY_CONF)' CROSS_AR='$(AR)' CROSS_CC='$(CC)' CROSS_LD='$(LD)' FLAGS='$(MRUBY_FLAGS)' BUILD_NAME='$(MRUBY_BUILD_NAME)' INCLUDE_DIR='$(ABS_REPO)/$(INCLUDE_DIR)' LIB_DIR='$(ABS_REPO)/$(LIB_DIR)'
	cp -r vendor/mruby/include/*                                $(INCLUDE_DIR)
	cp -r vendor/mruby/build/$(MRUBY_BUILD_NAME)/include/*      $(INCLUDE_DIR)
	cp    vendor/mruby/build/$(MRUBY_BUILD_NAME)/lib/libmruby.a $(LIB_DIR)

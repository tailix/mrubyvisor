CCPREFIX = /home/kotovalexarian/repos/global/tailix/cross/root/bin/i386-elf-

AR     = $(CCPREFIX)ar
AS     = $(CCPREFIX)as
CC     = $(CCPREFIX)gcc
LD     = $(CCPREFIX)ld
RANLIB = $(CCPREFIX)ranlib

GRUB_MKRESCUE = grub-mkrescue
RAKE          = rake
QEMU          = qemu-system-i386

ABS_REPO = $(shell pwd)

# Basic paths
MRUBY_CONF     = build_config.rb
DEST_DIR       = dest
IMAGE          = image.iso
ROOTFS_DIR     = rootfs
SRC_DIR        = src
LIBKERNAUX_DIR = vendor/libkernaux
MRUBY_DIR      = vendor/mruby

# Deeper paths
INCLUDE_DIR = $(DEST_DIR)/include
LIB_DIR     = $(DEST_DIR)/lib
LIBKERNAUX  = $(LIB_DIR)/libkernaux.a
LIBMRUBY    = $(LIB_DIR)/libmruby.a
GRUBCFG     = $(ROOTFS_DIR)/boot/grub/grub.cfg
MRUBYVISOR  = $(ROOTFS_DIR)/boot/mrubyvisor.multiboot2

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
	$(MAKE) -C $(SRC_DIR) clean

clean-dest:
	rm -rf $(MRUBYVISOR) $(DEST_DIR)/*

clean-mruby:
	cd $(MRUBY_DIR) && $(RAKE) clean

clean-libkernaux:
	$(MAKE) -C $(LIBKERNAUX_DIR) distclean || true

$(IMAGE): $(GRUBCFG) $(MRUBYVISOR)
	$(GRUB_MKRESCUE) $(ROOTFS_DIR) -o $@

$(MRUBYVISOR): $(LIBKERNAUX) $(LIBMRUBY)
	$(MAKE) -C $(SRC_DIR) mrubyvisor.multiboot2 CCPREFIX='$(CCPREFIX)' DEST='$(ABS_REPO)/$(DEST_DIR)' MRUBY_FLAGS='$(MRUBY_FLAGS)'
	cp $(SRC_DIR)/mrubyvisor.multiboot2 $@

$(LIBKERNAUX):
	cd $(LIBKERNAUX_DIR) && ./autogen.sh
	cd $(LIBKERNAUX_DIR) && ./configure --host='i386-elf' --prefix='$(ABS_REPO)/$(DEST_DIR)' $(LIBKERNAUX_ARGS) AR='$(AR)' AS='$(AS)' CC='$(CC)' LD='$(LD)' RANLIB='$(RANLIB)'
	cd $(LIBKERNAUX_DIR) && $(MAKE)
	cd $(LIBKERNAUX_DIR) && $(MAKE) install

$(LIBMRUBY): $(LIBKERNAUX) $(MRUBY_CONF)
	$(MAKE) clean-mruby
	mkdir -p $(INCLUDE_DIR) $(LIB_DIR)
	cd $(MRUBY_DIR) && $(RAKE) MRUBY_CONFIG='$(ABS_REPO)/$(MRUBY_CONF)' CROSS_AR='$(AR)' CROSS_CC='$(CC)' CROSS_LD='$(LD)' FLAGS='$(MRUBY_FLAGS)' BUILD_NAME='$(MRUBY_BUILD_NAME)' INCLUDE_DIR='$(ABS_REPO)/$(INCLUDE_DIR)' LIB_DIR='$(ABS_REPO)/$(LIB_DIR)'
	cp -r $(MRUBY_DIR)/include/*                                $(INCLUDE_DIR)
	cp -r $(MRUBY_DIR)/build/$(MRUBY_BUILD_NAME)/include/*      $(INCLUDE_DIR)
	cp    $(MRUBY_DIR)/build/$(MRUBY_BUILD_NAME)/lib/libmruby.a $(LIB_DIR)

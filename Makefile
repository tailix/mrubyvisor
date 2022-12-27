include config.mk

############
# Programs #
############

# Toolchain
AR     = $(CCPREFIX)ar
AS     = $(CCPREFIX)as
CC     = $(CCPREFIX)gcc
LD     = $(CCPREFIX)ld
RANLIB = $(CCPREFIX)ranlib

# Common programs
CP    = cp
MKDIR = mkdir
PWD   = pwd

# Additional programs
GRUB_MKRESCUE = grub-mkrescue
RAKE          = rake
QEMU          = qemu-system-i386

#########
# Paths #
#########

ABS_REPO = $(shell $(PWD))

# Basic paths
MRUBY_CONF = build_config.rb
DEST_DIR   = dest
IMAGE      = image.iso
ROOTFS_DIR = rootfs
SRC_DIR    = src

# Basic paths (dependencies)
LIBCLAYER_DIR  = vendor/libclayer
LIBKERNAUX_DIR = vendor/libkernaux
DRIVERS_DIR    = vendor/drivers
MRUBY_DIR      = vendor/mruby

# Deeper paths
INCLUDE_DIR = $(DEST_DIR)/include
LIB_DIR     = $(DEST_DIR)/lib
LIBCLAYER   = $(LIB_DIR)/libc.a
LIBDRIVERS  = $(LIB_DIR)/libdrivers.a
LIBKERNAUX  = $(LIB_DIR)/libkernaux.a
LIBMRUBY    = $(LIB_DIR)/libmruby.a
GRUBCFG     = $(ROOTFS_DIR)/boot/grub/grub.cfg
MRUBYVISOR  = $(ROOTFS_DIR)/boot/mrubyvisor.multiboot2

#############
# libclayer #
#############

LIBCLAYER_ARGS = \
	--enable-libc       \
	--disable-libclayer \
	--enable-freestanding

##############
# libkernaux #
##############

LIBKERNAUX_ARGS = \
	--enable-freestanding \
	--enable-split-libc   \
	--enable-debug        \
	--disable-float       \
	CFLAGS='-I$(ABS_REPO)/$(INCLUDE_DIR)'

###########
# drivers #
###########

DRIVERS_CFLAGS = \
	-nostdlib      \
	-ffreestanding \
	-fno-pic       \
	-fno-stack-protector

#########
# mruby #
#########

MRUBY_BUILD_NAME = mrubyvisor
MRUBY_FLAGS = \
	-DMRB_NO_BOXING \
	-DMRB_NO_FLOAT  \
	-DMRB_NO_STDIO

#########
# Tasks #
#########

.PHONY: $(IMAGE) $(MRUBYVISOR)

all: runc

runc: $(IMAGE)
	$(QEMU) -cdrom $< -serial stdio -display none

runw: $(IMAGE)
	$(QEMU) -cdrom $< -serial stdio

clean: clean-src clean-dest clean-mruby clean-libkernaux clean-libclayer

clean-src:
	$(MAKE) -C $(SRC_DIR) clean

clean-dest:
	rm -rf $(MRUBYVISOR) $(DEST_DIR)/*

clean-mruby:
	cd $(MRUBY_DIR) && $(RAKE) clean

clean-drivers:
	$(MAKE) -C $(DRIVERS_DIR) distclean || true

clean-libkernaux:
	$(MAKE) -C $(LIBKERNAUX_DIR) distclean || true

clean-libclayer:
	$(MAKE) -C $(LIBCLAYER_DIR) distclean || true

##############
# File tasks #
##############

$(IMAGE): $(GRUBCFG) $(MRUBYVISOR)
	$(GRUB_MKRESCUE) $(ROOTFS_DIR) -o $@

$(MRUBYVISOR): $(LIBCLAYER) $(LIBKERNAUX) $(LIBDRIVERS) $(LIBMRUBY)
	$(MAKE) -C $(SRC_DIR) mrubyvisor.multiboot2 CCPREFIX='$(CCPREFIX)' DEST='$(ABS_REPO)/$(DEST_DIR)' MRUBY_FLAGS='$(MRUBY_FLAGS)'
	$(CP) $(SRC_DIR)/mrubyvisor.multiboot2 $@

$(LIBCLAYER):
	cd $(LIBCLAYER_DIR) && ./autogen.sh
	cd $(LIBCLAYER_DIR) && ./configure --host='i386-elf' --prefix='$(ABS_REPO)/$(DEST_DIR)' $(LIBCLAYER_ARGS) AR='$(AR)' AS='$(AS)' CC='$(CC)' LD='$(LD)' RANLIB='$(RANLIB)'
	cd $(LIBCLAYER_DIR) && $(MAKE)
	cd $(LIBCLAYER_DIR) && $(MAKE) install

$(LIBKERNAUX): $(LIBCLAYER)
	cd $(LIBKERNAUX_DIR) && ./autogen.sh
	cd $(LIBKERNAUX_DIR) && ./configure --host='i386-elf' --prefix='$(ABS_REPO)/$(DEST_DIR)' $(LIBKERNAUX_ARGS) AR='$(AR)' AS='$(AS)' CC='$(CC)' LD='$(LD)' RANLIB='$(RANLIB)'
	cd $(LIBKERNAUX_DIR) && $(MAKE)
	cd $(LIBKERNAUX_DIR) && $(MAKE) install

$(LIBDRIVERS):
	cd $(DRIVERS_DIR) && ./autogen.sh
	cd $(DRIVERS_DIR) && ./configure --host='i386-elf' --prefix='$(ABS_REPO)/$(DEST_DIR)' CFLAGS='$(DRIVERS_CFLAGS)' AR='$(AR)' AS='$(AS)' CC='$(CC)' LD='$(LD)' RANLIB='$(RANLIB)'
	cd $(DRIVERS_DIR) && $(MAKE)
	cd $(DRIVERS_DIR) && $(MAKE) install

$(LIBMRUBY): $(LIBKERNAUX) $(MRUBY_CONF)
	$(MAKE) clean-mruby
	$(MKDIR) -p $(INCLUDE_DIR) $(LIB_DIR)
	cd $(MRUBY_DIR) && $(RAKE) MRUBY_CONFIG='$(ABS_REPO)/$(MRUBY_CONF)' CROSS_AR='$(AR)' CROSS_CC='$(CC)' CROSS_LD='$(LD)' FLAGS='$(MRUBY_FLAGS)' BUILD_NAME='$(MRUBY_BUILD_NAME)' INCLUDE_DIR='$(ABS_REPO)/$(INCLUDE_DIR)' LIB_DIR='$(ABS_REPO)/$(LIB_DIR)'
	$(CP) -r $(MRUBY_DIR)/include/*                                $(INCLUDE_DIR)
	$(CP) -r $(MRUBY_DIR)/build/$(MRUBY_BUILD_NAME)/include/*      $(INCLUDE_DIR)
	$(CP)    $(MRUBY_DIR)/build/$(MRUBY_BUILD_NAME)/lib/libmruby.a $(LIB_DIR)

.PHONY: clean

CCPREFIX = /home/kotovalexarian/repos/global/tailix/cross/root/bin/x86_64-elf-

AR     = $(CCPREFIX)ar
AS     = $(CCPREFIX)as
CC     = $(CCPREFIX)gcc
LD     = $(CCPREFIX)ld
RANLIB = $(CCPREFIX)ranlib

RAKE = rake

ROOT = $(shell pwd)
DEST = $(ROOT)/dest
MRUBY_CONFIG = $(ROOT)/build_config.rb

MRUBY_NAME = mrubyvisor

clean:
	rm -rf $(DEST)/*

dest/lib/libkernaux.a:
	cd vendor/libkernaux && ./autogen.sh
	cd vendor/libkernaux && ./configure --host='x86_64-elf' --prefix='$(DEST)' --enable-freestanding --with-drivers --with-libc AR='$(AR)' AS='$(AS)' CC='$(CC)' LD='$(LD)' RANLIB='$(RANLIB)'
	cd vendor/libkernaux && make
	cd vendor/libkernaux && make install

dest/lib/libmruby.a: dest/lib/libkernaux.a
	cd vendor/mruby && $(RAKE) MRUBY_CONFIG='$(MRUBY_CONFIG)' CROSS_AR='$(AR)' CROSS_CC='$(CC)' CROSS_LD='$(LD)'
	mkdir -p $(DEST)/include $(DEST)/lib
	cp vendor/mruby/build/$(MRUBY_NAME)/lib/libmruby.a $(DEST)/lib
	cp -r vendor/mruby/include/* $(DEST)/include
	cp -r vendor/mruby/build/$(MRUBY_NAME)/include/* $(DEST)/include

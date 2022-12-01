mrubyvisor
==========

* **mrubyvisor** is **mruby** running in supervisor (kernel) mode.
* **mruby** is a lightweight implementation of **Ruby**.
* **Ruby** is an interpreted, dynamically typed programming language.



Dependencies
------------

Edit the variable `CCPREFIX` in `Makefile` to point to the
[cross-compiler for i386](https://wiki.osdev.org/GCC_Cross-Compiler).

### Debian

The list may be incomplete.

* `build-essential`
* `grub-common`
* `qemu-system-x86`
* `rake`



Build
-----

Create file `config.mk`, use `config.example.mk` as a reference.

Then just do `make`.

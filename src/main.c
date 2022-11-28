#include "libc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernaux/assert.h>
#include <kernaux/drivers/console.h>
#include <kernaux/drivers/shutdown.h>
#include <kernaux/multiboot2.h>

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/presym.h>
#include <mruby/proc.h>
#include <mruby/string.h>

#define PANIC(msg) (assert(__FILE__, __LINE__, msg))
#define ASSERT(cond) ((cond) ? (void)0 : PANIC(#cond))

static mrb_state *mrb = NULL;
static mrbc_context *context = NULL;

static void assert(const char *file, int line, const char *str);

static bool load_module(const char *source, size_t size, const char *cmdline);

static mrb_value ruby_console_puts(mrb_state *mrb, mrb_value self);

void main(
    const uint32_t multiboot2_info_magic,
    const struct KernAux_Multiboot2_Info *const multiboot2_info
) {
    kernaux_drivers_console_puts("========================================");

    kernaux_assert_cb = assert;

    ASSERT(multiboot2_info_magic == KERNAUX_MULTIBOOT2_INFO_MAGIC);

    libc_init();

    ASSERT(mrb = mrb_open());
    ASSERT(context = mrbc_context_new(mrb));

    // Define method Kernel#console_puts
    mrb_define_method(
        mrb,
        mrb->kernel_module,
        "console_puts",
        ruby_console_puts,
        MRB_ARGS_REQ(1)
    );

    for (
        const struct KernAux_Multiboot2_ITag_Module *module_tag =
            (const struct KernAux_Multiboot2_ITag_Module*)
            KernAux_Multiboot2_Info_first_tag_with_type(
                multiboot2_info,
                KERNAUX_MULTIBOOT2_ITAG_MODULE
            );
        module_tag;
        module_tag =
            (const struct KernAux_Multiboot2_ITag_Module*)
            KernAux_Multiboot2_Info_tag_with_type_after(
                multiboot2_info,
                KERNAUX_MULTIBOOT2_ITAG_MODULE,
                (struct KernAux_Multiboot2_ITagBase*)module_tag
            )
    ) {
        const char *const cmdline =
            (const char*)KERNAUX_MULTIBOOT2_DATA(module_tag);
        const char *const source =
            (const char*)module_tag->mod_start;
        const size_t size = module_tag->mod_end - module_tag->mod_start;
        load_module(source, size, cmdline);
    }
}

void assert(const char *const file, const int line, const char *const str)
{
    kernaux_drivers_console_printf("panic: %s:%u: \"%s\"\n", file, line, str);
    kernaux_drivers_shutdown_poweroff();
}

bool load_module(
    const char *const source,
    const size_t size,
    const char *const cmdline __attribute__((unused))
) {
    const int arena = mrb_gc_arena_save(mrb);
    const bool status =
        !mrb_undef_p(mrb_load_nstring_cxt(mrb, source, size, context));
    struct REnv *const env = mrb_vm_ci_env(mrb->c->cibase);
    mrb_vm_ci_env_set(mrb->c->cibase, NULL);
    mrb_env_unshare(mrb, env, FALSE);
    mrbc_cleanup_local_variables(mrb, context);
    mrb_gc_arena_restore(mrb, arena);
    return status;
}

mrb_value ruby_console_puts(
    mrb_state *const mrb,
    mrb_value self __attribute__((unused))
) {
    const char *str = NULL;
    mrb_get_args(mrb, "z", &str);
    kernaux_drivers_console_puts(str);
    return mrb_nil_value();
}

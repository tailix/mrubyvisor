#include "libc.h"
#include "logger.h"
#include "panic.h"
#include "stack_trace.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernaux/drivers/console.h>
#include <kernaux/multiboot2.h>

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/proc.h>
#include <mruby/string.h>

static mrb_state *mrb = NULL;
static mrbc_context *context = NULL;

static bool load_module(const char *source, size_t size, const char *cmdline);

static mrb_value ruby_console_puts(mrb_state *mrb, mrb_value self);

void main(
    const uint32_t multiboot2_info_magic,
    const struct KernAux_Multiboot2_Info *const multiboot2_info
) {
    panic_init();
    libc_init();

    ASSERT(multiboot2_info_magic == KERNAUX_MULTIBOOT2_INFO_MAGIC);
    ASSERT(KernAux_Multiboot2_Info_is_valid(multiboot2_info));

    stack_trace_init(multiboot2_info);

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
    mrb_env_unshare(mrb, env);
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

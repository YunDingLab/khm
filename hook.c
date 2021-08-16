#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include "hook.h"

void filter_register(struct filter *flt)
{
    TAILQ_INSERT_TAIL(&flts, flt, list);
}

int filter_disabled(void)
{
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,7,0)
static unsigned long lookup_name(const char *name)
{
	struct kprobe kp = {
		.symbol_name = name
	};
	unsigned long retval;

	if (register_kprobe(&kp) < 0) return 0;
	retval = (unsigned long) kp.addr;
	unregister_kprobe(&kp);
	return retval;
}
#else
static unsigned long lookup_name(const char *name)
{
	return kallsyms_lookup_name(name);
}
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 11, 0))
static inline bool within_module(unsigned long addr, const struct module *mod)
{
    return within_module_init(addr, mod) || within_module_core(addr, mod);
}
#endif

static void notrace filter_ftrace_thunk(unsigned long ip,
        unsigned long parent_ip, struct ftrace_ops *ops, struct pt_regs *regs)
{
    struct filter *flt = container_of(ops, struct filter, ops);
    if (!within_module(parent_ip, THIS_MODULE)) {
        regs->ax = parent_ip;
        regs->ip = (unsigned long)flt->filter;
    }
}

static int filter_set_ftrace(struct filter *flt)
{
    void *addr;
    int err;

    addr = (void *)lookup_name(flt->fname);
    if (addr == NULL)
        return -ENOENT;

    flt->address = addr;
    flt->real = addr + MCOUNT_INSN_SIZE;
    barrier();

    flt->ops.func = filter_ftrace_thunk;
    flt->ops.flags = FTRACE_OPS_FL_SAVE_REGS
                   | FTRACE_OPS_FL_IPMODIFY;

    err = ftrace_set_filter_ip(&flt->ops, (unsigned long)addr, 0, 0);
    if (err)
        return err;

    err = register_ftrace_function(&flt->ops);
    if (err) {
        ftrace_set_filter_ip(&flt->ops, (unsigned long)addr, 1, 0);
        return err;
    }

    return 0;
}

static int filter_remove_ftrace(struct filter *flt)
{
    int err;

    err = unregister_ftrace_function(&flt->ops);
    if (err)
        return err;

    err = ftrace_set_filter_ip(&flt->ops, (unsigned long)flt->address, 1, 0);
    if (err)
        return err;

    return 0;
}

int filter_hook(void)
{
    struct filter *flt;
    int ret = 0;

    TAILQ_FOREACH(flt, &flts, list) {
        flt->error = filter_set_ftrace(flt);
        if (flt->error) {
            ret = -1;
            break;
        }
    }

    return ret;
}

int filter_unhook(void)
{
    struct filter *flt;

    TAILQ_FOREACH(flt, &flts, list) {
        int err;

        if (flt->real == NULL || flt->error < 0)
            continue;

        if (flt->flags & FLT_ALREADY_UNINSTALL)
            continue;

        err = filter_remove_ftrace(flt);

        if (err) {
            return err;
        }

        flt->flags |= FLT_ALREADY_UNINSTALL;
    }

    return 0;
}

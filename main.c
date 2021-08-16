#include <linux/module.h>
#include <linux/kallsyms.h>
#include "ctor.h"
#include "hook.h"

int kmain_init(void)
{
    int ret;

    ctor_init();

    ret = filter_hook();
    if (ret)
        goto err;
    
    return 0;

err:
    dtor_exit();
    return ret;
}

void kmain_exit(void)
{
    filter_unhook();
    dtor_exit();
}

MODULE_LICENSE("GPL");
module_init(kmain_init);
module_exit(kmain_exit);
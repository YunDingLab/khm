
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/version.h>
#include <linux/magic.h>
#include <linux/cred.h>
#include <linux/uidgid.h>
#include "hook.h"

int is_ret_legal_caller(unsigned long ret_ip)
{
    u8 opcode;
    ret_ip -= 5;

    opcode = *((u8 *)ret_ip);
    if (opcode == 0xe8)
        return 1;

    return 0;
}

FILTER_DEFINE1(commit_creds, int, struct cred *, new)
{
    unsigned long volatile parent_ip = 0;
    asm volatile("movq %%rax,%0" : "=g"(parent_ip));

    if (new->uid.val == 0 && new->gid.val == 0) {
        if (!is_ret_legal_caller(parent_ip)) {
            printk("commit_creds called in rop shellcode 0x%lx!\n", parent_ip);
            return 0;
        }
    }

    return REAL(commit_creds, new);
}
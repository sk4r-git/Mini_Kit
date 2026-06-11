#include "hooks.h"
#include "stream.h"

#include "../include/intercept.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/dcache.h>
#include <linux/path.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/errno.h>



static struct kprobe kp_glob_write;
static bool kp_glob_write_active = false;

static struct kretprobe krp_glob_read;
static bool krp_glob_read_active = false;

static bool blacklist_0_whitelist_1 = false; 

#define MAX_DUMP 256
static DEFINE_PER_CPU(int, in_hook);

static int handler_pre_glob_write(struct kprobe *p, struct pt_regs *regs)
{
    struct file *file;
    char __user *ubuf;
    size_t count, n;
    char kbuf[MAX_DUMP + 1];

#if defined(CONFIG_X86_64)
    file  = (struct file *)regs->di;
    ubuf  = (char __user *)regs->si;
    count = regs->dx;
#else
#error "arch non supportée"
#endif

    if (!file || !file->f_inode || !ubuf || count == 0)
        return 0;

    n = min_t(size_t, count, MAX_DUMP);

    if (copy_from_user_nofault(kbuf, ubuf, n))
        goto out;
    kbuf[n] = '\0';

    printk(KERN_INFO "[KPROBE] PID=%d (%s) write inode=%lu bytes=%zu\n",
           current->pid, current->comm,
           file->f_inode->i_ino, count);

    print_hex_dump(KERN_INFO, "[DATA] ", DUMP_PREFIX_OFFSET,
                   16, 1, kbuf, n, true);

out:

    return 0;
}


struct read_ctx {
    char __user *ubuf;
    struct file *file;
};

static int handler_entry_glob_read(struct kretprobe_instance *ri,
                                   struct pt_regs *regs)
{
    struct read_ctx *ctx = (struct read_ctx *)ri->data;

#if defined(CONFIG_X86_64)
    ctx->file = (struct file *)regs->di;
    ctx->ubuf = (char __user *)regs->si;
#else
#error "arch non supportée"
#endif
    return 0;  
}


static int handler_ret_glob_read(struct kretprobe_instance *ri,
                                 struct pt_regs *regs)
{
    struct read_ctx *ctx = (struct read_ctx *)ri->data;
    long ret = regs_return_value(regs);   
    char kbuf[MAX_DUMP + 1];
    size_t n;
    int *guard;


    if (!ctx->ubuf || !ctx->file || !ctx->file->f_inode)
        return 0;
    if (ret <= 0)
        return 0;


    guard = this_cpu_ptr(&in_hook);
    if (*guard)
        return 0;
    (*guard)++;

    n = min_t(size_t, ret, MAX_DUMP); 


    if (copy_from_user_nofault(kbuf, ctx->ubuf, n))
        goto out;
    kbuf[n] = '\0';

    printk(KERN_INFO "[KRETPROBE] PID=%d (%s) read inode=%lu bytes=%ld\n",
           current->pid, current->comm,
           ctx->file->f_inode->i_ino, ret);
    print_hex_dump(KERN_INFO, "[DATA] ", DUMP_PREFIX_OFFSET,
                   16, 1, kbuf, n, true);

out:
    (*guard)--;
    return 0;
}

int global_write_hook(void){
    int ret;
    if (kp_glob_write_active)
        return 0x00010000;

    memset(&kp_glob_write, 0, sizeof(kp_glob_write));
    kp_glob_write.symbol_name = "vfs_write";
    kp_glob_write.pre_handler = handler_pre_glob_write;

    ret = register_kprobe(&kp_glob_write);
    if (ret < 0) {
        printk(KERN_ERR "register_kprobe failed: %d\n", ret);
        return ret;
    }

    kp_glob_write_active = true;
    printk(KERN_INFO "[HOOK] kprobe activé\n");

    return 0;
}

int global_read_hook(void){
    int ret;
    if (krp_glob_read_active)
        return 0;

    memset(&krp_glob_read, 0, sizeof(krp_glob_read));
    krp_glob_read.kp.symbol_name = "vfs_read";
    krp_glob_read.entry_handler  = handler_entry_glob_read,
    krp_glob_read.handler        = handler_ret_glob_read,
    krp_glob_read.data_size      = sizeof(struct read_ctx),
    krp_glob_read.maxactive      = 64,

    ret = register_kretprobe(&krp_glob_read);
    if (ret < 0) {
        printk(KERN_ERR "register_kprobe failed: %d\n", ret);
        return ret;
    }

    krp_glob_read_active = true;
    printk(KERN_INFO "[HOOK] kprobe activé\n");

    return 0;
}

int global_hook(void){
    int w = global_write_hook();
    int r = global_read_hook();
    return w & r;
}

void global_unhook(void)
{
    if (kp_glob_write_active) {
        unregister_kprobe(&kp_glob_write);
        kp_glob_write_active = false;
        printk(KERN_INFO "[HOOK] kprobe write désactivé\n");
    }
    if (krp_glob_read_active) {
        unregister_kretprobe(&krp_glob_read);
        krp_glob_read_active = false;
        printk(KERN_INFO "[HOOK] kprobe read désactivé\n");
    }
}
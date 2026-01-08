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

static struct kprobe kp;
static bool kp_active = false;

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    struct file *file;
    size_t count;
    char __user *ubuf;

#if defined(CONFIG_X86_64)
    file  = (struct file *)regs->di;
    count = regs->dx;
    ubuf  = (char __user *)regs->si;
#else
#error "arch non supportée"
#endif

    if (!file || !file->f_inode){
        printk(KERN_INFO "[HOOK] no file");
        return 0;
    }

    // Filtrage par processus ciblé
    if (targeted_task && current->pid != targeted_task->pid){
        // printk(KERN_INFO "[HOOK] no targeted_task, %d, %d\n", current->pid, targeted_task->pid);
        return 0;
    }
    printk(KERN_INFO "[HOOK] targeted_task, %d, %d\n", current->pid, targeted_task->pid);

    // Filtrage par fichier ciblé
    if (targeted_file && file != targeted_file){
        char path[256];
        char * tmp = d_path(&file->f_path, path, sizeof(path));
        char path2[256];
        char * tmp2 = d_path(&targeted_file->f_path, path2, sizeof(path2));
        printk(KERN_INFO "[HOOK] final pb, %p, %d, %p, %d, %s, %s\n", file, file->f_inode, targeted_file, targeted_file->f_inode, tmp, tmp2);
        return 0;
    }

    #define MAX_DUMP 256
    char kbuf[MAX_DUMP + 1];
    size_t n = min(count, (size_t)MAX_DUMP);

    if (copy_from_user(kbuf, ubuf, n))
        return 0;

    kbuf[n] = '\0';

    printk(KERN_INFO "[KPROBE] PID=%d (%s) write inode=%lu bytes=%zu\n",
           current->pid,
           current->comm,
           file->f_inode->i_ino,
           count);
    print_hex_dump(KERN_INFO, "[DATA] ", DUMP_PREFIX_OFFSET,
               16, 1, kbuf, n, true);
    push_stream_data(kbuf, n);
    return 0;
}

int hook_file(struct file *file)
{
    int ret;

    if (kp_active)
        return 0;

    memset(&kp, 0, sizeof(kp));
    kp.symbol_name = "vfs_write";
    kp.pre_handler = handler_pre;

    ret = register_kprobe(&kp);
    if (ret < 0) {
        printk(KERN_ERR "register_kprobe failed: %d\n", ret);
        return ret;
    }

    kp_active = true;
    printk(KERN_INFO "[HOOK] kprobe activé\n");

    return 0;
}

void unhook_file(void)
{
    if (!kp_active)
        return;

    unregister_kprobe(&kp);
    kp_active = false;

    printk(KERN_INFO "[HOOK] kprobe désactivé\n");
}


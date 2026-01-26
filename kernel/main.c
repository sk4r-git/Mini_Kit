#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/fdtable.h>
#include <linux/delay.h>
#include <linux/kprobes.h>
#include <linux/uio.h>
#include <linux/kallsyms.h>
#include <asm/processor.h>
#include <asm/paravirt.h>

#include "../include/intercept.h"
#include "ioctl.h"
#include "my_kallsyms.h"

static const struct file_operations intercept_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = intercept_ioctl,
    .read = intercept_read,
};

static struct miscdevice intercept_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &intercept_fops,
};

/*
/sys/kernel/debug/kprobes/blacklist
reference les kprobes qu'on a pas le droit de mettre
on va faire en sorte que la fonction de check renvoie toujours true 
comme ça on pourra mettre des kprobes partout :D
*/
static void disable_wp(void)
{
    write_cr0(read_cr0() & (~0x10000));
}

static void enable_wp(void)
{
    write_cr0(read_cr0() | 0x10000);
}

unsigned char saved_blacklist_bytes[] = {0x00, 0x00, 0x00};

static int disable_blacklist_kprobes(void){
    unsigned long addr;
    unsigned char patch[] = { 0x31, 0xC0, 0xC3 }; // xor eax,eax ; ret

    addr = get_addresse_of_symbol_from_string("within_kprobe_blacklist");

    if (!addr) {
        printk(KERN_ERR "Impossible de trouver la fonction within_kprobe_blacklist\n");
        return -1;
    }

    printk(KERN_INFO "kprobe check @ %lx\n", addr);

    disable_wp();
    memcpy(saved_blacklist_bytes, (void *)addr, 3);
    memcpy((void *)addr, patch, 3);
    enable_wp();

    printk(KERN_INFO "Blacklist kprobes neutralisée 😈\n");

    return 0;
}

static int enable_blacklist_kprobes(void){
    unsigned long addr;

    addr = get_addresse_of_symbol_from_string("within_kprobe_blacklist");

    if (!addr) {
        printk(KERN_ERR "Impossible de trouver la fonction within_kprobe_blacklist\n");
        return -1;
    }

    printk(KERN_INFO "kprobe check @ %lx\n", addr);

    disable_wp();
    memcpy((void *)addr, saved_blacklist_bytes, 3);
    enable_wp();

    printk(KERN_INFO "Blacklist kprobes neutralisée 😈\n");

    return 0;
}

static int __init intercept_init(void)
{
    unsigned long addr = get_addresse_of_symbol_from_string("int80_emulation");
    // unsigned long addr_kb_bl = get_addresse_of_symbol_from_string("within_kprobe_blacklist");
    printk(KERN_INFO "addr of int80 emulation = %lx\n", addr);
    // printk(KERN_INFO "addr of kb_bl = %lx\n", addr_kb_bl);
    // bool (*within_kprobe_blacklist_fn)(unsigned long);
    // within_kprobe_blacklist_fn = (void *)addr_kb_bl;
    // printk(KERN_INFO "Blacklist: %d\n", within_kprobe_blacklist_fn(addr));
    // disable_blacklist_kprobes();
    // printk(KERN_INFO "Blacklist: %d\n", within_kprobe_blacklist_fn(addr));
    misc_register(&intercept_dev);
    printk(KERN_INFO "Module intercept chargé\n");
    return 0;
}

static void __exit intercept_exit(void)
{
    // unsigned long addr = get_addresse_of_symbol_from_string("int80_emulation");
    // unsigned long addr_kb_bl = get_addresse_of_symbol_from_string("within_kprobe_blacklistn");
    // bool (*within_kprobe_blacklist_fn)(unsigned long);
    // within_kprobe_blacklist_fn = (void *)addr_kb_bl;
    // printk(KERN_INFO "Blacklist: %d\n", within_kprobe_blacklist_fn(addr));
    // enable_blacklist_kprobes();
    // printk(KERN_INFO "Blacklist: %d\n", within_kprobe_blacklist_fn(addr));
    misc_deregister(&intercept_dev);
    printk(KERN_INFO "Module intercept déchargé\n");
}

module_init(intercept_init);
module_exit(intercept_exit);

MODULE_LICENSE("GPL");
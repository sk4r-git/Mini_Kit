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

#include "../include/intercept.h"
#include "ioctl.h"

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



static int __init intercept_init(void)
{
    misc_register(&intercept_dev);
    printk(KERN_INFO "Module intercept chargé\n");
    return 0;
}

static void __exit intercept_exit(void)
{
    misc_deregister(&intercept_dev);
    printk(KERN_INFO "Module intercept déchargé\n");
}

module_init(intercept_init);
module_exit(intercept_exit);

MODULE_LICENSE("GPL");
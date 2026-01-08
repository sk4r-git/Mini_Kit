#pragma once
#include <linux/fs.h>

long intercept_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
ssize_t intercept_read(struct file *f, char __user *buf, size_t len, loff_t *off);
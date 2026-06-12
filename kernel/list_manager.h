#pragma once
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/types.h>

bool file_blacklisted(struct inode *inode);
bool file_whitelisted(struct inode *inode);
int  file_add_b(dev_t dev, unsigned long ino);
int  file_add_w(dev_t dev, unsigned long ino);
int  file_del_b(dev_t dev, unsigned long ino);
int  file_del_w(dev_t dev, unsigned long ino);

bool proc_blacklisted(struct task_struct *t);
bool proc_whitelisted(struct task_struct *t);
int  proc_add_b(dev_t dev, unsigned long ino);
int  proc_add_w(dev_t dev, unsigned long ino);
int  proc_del_b(dev_t dev, unsigned long ino);
int  proc_del_w(dev_t dev, unsigned long ino);

bool should_hook(struct inode *inode, struct task_struct *task);

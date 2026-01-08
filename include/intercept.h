#pragma once
#include <linux/ioctl.h>

#define DEVICE_NAME "intercept"

#define IOCTL_A   _IOR('m', 1, char *)
#define IOCTL_AS  _IOR('m', 10, size_t)

#define IOCTL_B   _IOR('m', 2, int)
#define IOCTL_BT  _IOW('m', 20, int)
#define IOCTL_BS  _IOR('m', 21, size_t)

#define IOCTL_C   _IOWR('m', 3, int)
#define IOCTL_CT  _IOW('m', 30, int)
#define IOCTL_CI  _IOWR('m', 31, int)

#define IOCTL_D   _IOWR('m', 4, int)


#include <linux/sched.h>
#include <linux/fs.h>

extern struct task_struct *targeted_task;
extern struct file *targeted_file;

void push_stream_data(const char *buf, size_t len);
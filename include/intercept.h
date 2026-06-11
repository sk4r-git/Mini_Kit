#pragma once
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/fs.h>

#define DEVICE_NAME "intercept"

#define IOCTL_LOG _IOR('m', 0, int)

void push_stream_data(const char *buf, size_t len);
#pragma once
#include <linux/ioctl.h>
#include <linux/types.h>

#define DEVICE_NAME "intercept"

/* dev+ino : taille uniforme kernel(x86_64) et userspace */
struct intercept_proc_entry {
    unsigned long dev;
    unsigned long ino;
};

struct intercept_file_entry {
    unsigned long dev;
    unsigned long ino;
};

#define IOCTL_LOG         _IOR('m', 0, int)
#define IOCTL_BL_ADD_PROC _IOW('m', 1, struct intercept_proc_entry)
#define IOCTL_WL_ADD_PROC _IOW('m', 2, struct intercept_proc_entry)
#define IOCTL_BL_ADD_FILE _IOW('m', 3, struct intercept_file_entry)
#define IOCTL_WL_ADD_FILE _IOW('m', 4, struct intercept_file_entry)

void push_stream_data(const char *buf, size_t len);
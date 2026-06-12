#include "../include/intercept.h"
#include "hooks.h"
#include "ioctl.h"
#include "stream.h"
#include "list_manager.h"

long intercept_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case IOCTL_LOG:
            printk(KERN_INFO "Action LOG effectuée\n");
            return global_hook();

        case IOCTL_BL_ADD_PROC:
        case IOCTL_WL_ADD_PROC: {
            struct intercept_proc_entry e;
            if (copy_from_user(&e, (void __user *)arg, sizeof(e)))
                return -EFAULT;
            return (cmd == IOCTL_BL_ADD_PROC) ? proc_add_b((dev_t)e.dev, e.ino)
                                              : proc_add_w((dev_t)e.dev, e.ino);
        }
        case IOCTL_BL_ADD_FILE: {
            struct intercept_file_entry e;
            if (copy_from_user(&e, (void __user *)arg, sizeof(e)))
                return -EFAULT;
            return file_add_b((dev_t)e.dev, e.ino);
        }
        case IOCTL_WL_ADD_FILE: {
            struct intercept_file_entry e;
            if (copy_from_user(&e, (void __user *)arg, sizeof(e)))
                return -EFAULT;
            return file_add_w((dev_t)e.dev, e.ino);
        }
        default:
            return -EINVAL;
    }
}

ssize_t intercept_read(struct file *f, char __user *buf,
                       size_t len, loff_t *off)
{
    wait_event_interruptible(stream_wait, stream_len > 0);

    mutex_lock(&stream_lock);
    ssize_t ret = stream_len;
    if (copy_to_user(buf, stream_buf, stream_len))
        ret = -EFAULT;
    stream_len = 0;
    mutex_unlock(&stream_lock);

    return ret;
}

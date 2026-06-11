#include "../include/intercept.h"
#include "hooks.h"
#include "stream.h"

long intercept_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd) {
        case IOCTL_LOG:
            printk(KERN_INFO "Action LOG effectuée\n");
            return global_hook();
        default:
            printk(KERN_INFO "Commande inconnue\n");
            return -EINVAL;
    }    
    return 0;
}



ssize_t intercept_read(struct file *f, char __user *buf,
                 size_t len, loff_t *off)
{
    wait_event_interruptible(stream_wait, stream_len > 0);

    mutex_lock(&stream_lock);
    copy_to_user(buf, stream_buf, stream_len);
    size_t ret = stream_len;
    stream_len = 0;
    mutex_unlock(&stream_lock);

    return ret;
}
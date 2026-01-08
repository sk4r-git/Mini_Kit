#include "../include/intercept.h"
#include "infos.h"
#include "hooks.h"
#include "stream.h"

long intercept_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd) {
        case IOCTL_AS:
            printk(KERN_INFO "Action AS effectuée\n");
            return get_size_list_procs(arg);
        case IOCTL_A:
            printk(KERN_INFO "Action A effectuée\n");
            return list_procs(arg);

        case IOCTL_BT:
            printk(KERN_INFO "Action BT effectuée\n");
            return set_target(arg);
        case IOCTL_BS:
            printk(KERN_INFO "Action BS effectuée\n");
            return get_size_file_list(arg);
        case IOCTL_B:
            printk(KERN_INFO "Action B effectuée\n");
            return get_file_list(arg);

        case IOCTL_CT:
            printk(KERN_INFO "Action CT effectuée\n");
            return set_file_target(arg);
        case IOCTL_CI:
            printk(KERN_INFO "Action CI effectuée\n");
            return get_info_file(arg);
        case IOCTL_C:
            printk(KERN_INFO "Action C effectuée\n");
            int rien = hook_file(targeted_file);
            return 0;

        case IOCTL_D:
            printk(KERN_INFO "Action D effectuée\n");
            unhook_file();
            return 0;
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
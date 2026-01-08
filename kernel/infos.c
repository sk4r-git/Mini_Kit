#include "../include/intercept.h"


#include <linux/fs.h>
#include <linux/fdtable.h>
#include <linux/sched.h>
#include <linux/sched/task.h>


struct task_struct *targeted_task = NULL;
struct file *targeted_file = NULL;

int list_procs(unsigned long arg)
{
    struct task_struct *task;
    char * buf;
    size_t size = 0;
    int len;

    for_each_process(task) {
        size += snprintf(NULL, 0, "PID: %d, Nom: %s\n", task->pid, task->comm);
    }

    buf = kmalloc(size + 1, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    len = 0;
    for_each_process(task) {
        len += snprintf(buf + len, size - len + 1, "PID: %d, Nom: %s\n", task->pid, task->comm);
    }
    if (copy_to_user((char __user *)arg, buf, len + 1)) {
        kfree(buf);
        return -EFAULT;
    }

    kfree(buf);
    return 0;

}


int get_size_list_procs(unsigned long arg)
{
    struct task_struct *task;
    size_t size = 0;
    for_each_process(task) {
        size += snprintf(NULL, 0, "PID: %d, Nom: %s\n", task->pid, task->comm);
    }
    if (copy_to_user((char __user *)arg, &size, sizeof(size_t))) {
        return -EFAULT;
    }
    return 0;

}

int set_target(unsigned long arg)
{
    struct task_struct *task;
    for_each_process(task) {
        if (task->pid == arg){
            targeted_task = task;
            break;
        }
    }
    return 0;

}

int get_size_file_list(unsigned long arg)
{
    struct files_struct *files;
    struct fdtable *fdt;
    int i;
    files = targeted_task->files;
    if (!files)
        return 0;

    spin_lock(&files->file_lock);
    size_t size = 0;

    fdt = files_fdtable(files);
    for (i = 0; i < fdt->max_fds; i++) {
        struct file *f = fdt->fd[i];

        if (f != NULL) {
            char path[256];
            char *tmp;

            tmp = d_path(&f->f_path, path, sizeof(path));
            if (IS_ERR(tmp))
                size += snprintf(NULL, 0, "  FD %d: [unknown]\n", i);
            else
                size += snprintf(NULL, 0, "  FD %d: %s\n", i, tmp);
        }
    }

    spin_unlock(&files->file_lock);
    if (copy_to_user((char __user *)arg, &size, sizeof(size_t))) {
        return -EFAULT;
    }
    return 0;
}

int get_file_list(unsigned long arg)
{
    struct files_struct *files;
    struct fdtable *fdt;
    int i;
    char * buf;
    size_t size = 0;
    int len = 0;

    files = targeted_task->files;
    if (!files)
        return 0;

    spin_lock(&files->file_lock);


    fdt = files_fdtable(files);
    for (i = 0; i < fdt->max_fds; i++) {
        struct file *f = fdt->fd[i];

        if (f != NULL) {
            char path[256];
            char *tmp;

            tmp = d_path(&f->f_path, path, sizeof(path));
            if (IS_ERR(tmp))
                size += snprintf(NULL, 0, "  FD %d: [unknown]\n", i);
            else
                size += snprintf(NULL, 0, "  FD %d: %s\n", i, tmp);
        }
    }

    buf = kmalloc(size + 1, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    for (i = 0; i < fdt->max_fds; i++) {
        struct file *f = fdt->fd[i];

        if (f != NULL) {
            char path[256];
            char *tmp;

            tmp = d_path(&f->f_path, path, sizeof(path));
            if (IS_ERR(tmp))
                len += snprintf(buf + len, size-len+1, "  FD %d: [unknown]\n", i);
            else
                len += snprintf(buf + len, size-len+1, "  FD %d: %s\n", i, tmp);
        }
    }
    if (copy_to_user((char __user *)arg, buf, len + 1)) {
        kfree(buf);
        return -EFAULT;
    }

    kfree(buf);
    spin_unlock(&files->file_lock);
    return 0;
}


int set_file_target(unsigned long arg){
    struct files_struct *files;
    struct fdtable *fdt;
    char * buf;

    files = targeted_task->files;
    if (!files)
        return 0;

    spin_lock(&files->file_lock);

    fdt = files_fdtable(files);
    targeted_file = fdt->fd[arg];

    kfree(buf);
    spin_unlock(&files->file_lock);
    return 0;
}

int get_info_file(unsigned long arg){
    printk("PID=%d (%s), mode=%s%s, flags=0x%x\n",
            targeted_task->pid,
            targeted_task->comm,
            (targeted_file->f_mode & FMODE_READ) ? "R" : "",
            (targeted_file->f_mode & FMODE_WRITE) ? "W" : "",
            targeted_file->f_flags
    );
    return 0;
}
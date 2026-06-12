#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fs.h>
#include "list_manager.h"

struct black_file {
    dev_t            dev;
    unsigned long    ino;
    struct list_head list;
    struct rcu_head  rcu;
};

struct black_proc {
    struct rcu_head  rcu;
    struct list_head list;
    dev_t            dev;
    unsigned long    ino;
};

struct white_file {
    dev_t            dev;
    unsigned long    ino;
    struct list_head list;
    struct rcu_head  rcu;
};

struct white_proc {
    struct rcu_head  rcu;
    struct list_head list;
    dev_t            dev;
    unsigned long    ino;
};

static LIST_HEAD(black_files);
static LIST_HEAD(black_procs);
static DEFINE_SPINLOCK(black_wlock);

static LIST_HEAD(white_files);
static LIST_HEAD(white_procs);
static DEFINE_SPINLOCK(white_wlock);

bool file_blacklisted(struct inode *inode)
{
    struct black_file *e;
    bool hit = false;

    rcu_read_lock();
    list_for_each_entry_rcu(e, &black_files, list) {
        if (e->ino == inode->i_ino && e->dev == inode->i_sb->s_dev) {
            hit = true;
            break;
        }
    }
    rcu_read_unlock();
    return hit;
}

bool file_whitelisted(struct inode *inode)
{
    struct white_file *e;
    bool hit = false;

    rcu_read_lock();
    list_for_each_entry_rcu(e, &white_files, list) {
        if (e->ino == inode->i_ino && e->dev == inode->i_sb->s_dev) {
            hit = true;
            break;
        }
    }
    rcu_read_unlock();
    return hit;
}

int file_add_b(dev_t dev, unsigned long ino)
{
    struct black_file *e = kmalloc(sizeof(*e), GFP_KERNEL);
    if (!e)
        return -ENOMEM;
    e->dev = dev;
    e->ino = ino;

    spin_lock(&black_wlock);
    list_add_rcu(&e->list, &black_files);
    spin_unlock(&black_wlock);
    return 0;
}

int file_add_w(dev_t dev, unsigned long ino)
{
    struct white_file *e = kmalloc(sizeof(*e), GFP_KERNEL);
    if (!e)
        return -ENOMEM;
    e->dev = dev;
    e->ino = ino;

    spin_lock(&white_wlock);
    list_add_rcu(&e->list, &white_files);
    spin_unlock(&white_wlock);
    return 0;
}

int file_del_b(dev_t dev, unsigned long ino)
{
    struct black_file *e;

    spin_lock(&black_wlock);
    list_for_each_entry(e, &black_files, list) {
        if (e->ino == ino && e->dev == dev) {
            list_del_rcu(&e->list);
            spin_unlock(&black_wlock);
            kfree_rcu(e, rcu);
            return 0;
        }
    }
    spin_unlock(&black_wlock);
    return -ENOENT;
}

int file_del_w(dev_t dev, unsigned long ino)
{
    struct white_file *e;

    spin_lock(&white_wlock);
    list_for_each_entry(e, &white_files, list) {
        if (e->ino == ino && e->dev == dev) {
            list_del_rcu(&e->list);
            spin_unlock(&white_wlock);
            kfree_rcu(e, rcu);
            return 0;
        }
    }
    spin_unlock(&white_wlock);
    return -ENOENT;
}

/* Lit dev+ino de l'exe du task sous RCU. Retourne false si indisponible. */
static bool task_exe_ids(struct task_struct *t, dev_t *dev, unsigned long *ino)
{
    struct mm_struct *mm = t->mm;
    if (!mm)
        return false;

    rcu_read_lock();
    struct file *exe = rcu_dereference(mm->exe_file);
    if (!exe || !exe->f_inode) {
        rcu_read_unlock();
        return false;
    }
    *dev = exe->f_inode->i_sb->s_dev;
    *ino = exe->f_inode->i_ino;
    rcu_read_unlock();
    return true;
}

bool proc_blacklisted(struct task_struct *t)
{
    dev_t dev; unsigned long ino;
    if (!task_exe_ids(t, &dev, &ino))
        return false;

    struct black_proc *e;
    bool hit = false;

    rcu_read_lock();
    list_for_each_entry_rcu(e, &black_procs, list) {
        if (e->dev == dev && e->ino == ino) { hit = true; break; }
    }
    rcu_read_unlock();
    return hit;
}

bool proc_whitelisted(struct task_struct *t)
{
    dev_t dev; unsigned long ino;
    if (!task_exe_ids(t, &dev, &ino))
        return false;

    struct white_proc *e;
    bool hit = false;

    rcu_read_lock();
    list_for_each_entry_rcu(e, &white_procs, list) {
        if (e->dev == dev && e->ino == ino) { hit = true; break; }
    }
    rcu_read_unlock();
    return hit;
}

int proc_add_b(dev_t dev, unsigned long ino)
{
    struct black_proc *e, *it;

    printk(KERN_INFO "[LIST] proc_add_b dev=%u ino=%lu\n", (unsigned)dev, ino);

    e = kmalloc(sizeof(*e), GFP_KERNEL);
    if (!e)
        return -ENOMEM;
    e->dev = dev;
    e->ino = ino;

    spin_lock(&black_wlock);
    list_for_each_entry(it, &black_procs, list) {
        if (it->dev == dev && it->ino == ino) {
            spin_unlock(&black_wlock);
            kfree(e);
            return -EEXIST;
        }
    }
    list_add_rcu(&e->list, &black_procs);
    spin_unlock(&black_wlock);
    return 0;
}

int proc_add_w(dev_t dev, unsigned long ino)
{
    struct white_proc *e, *it;

    e = kmalloc(sizeof(*e), GFP_KERNEL);
    if (!e)
        return -ENOMEM;
    e->dev = dev;
    e->ino = ino;

    spin_lock(&white_wlock);
    list_for_each_entry(it, &white_procs, list) {
        if (it->dev == dev && it->ino == ino) {
            spin_unlock(&white_wlock);
            kfree(e);
            return -EEXIST;
        }
    }
    list_add_rcu(&e->list, &white_procs);
    spin_unlock(&white_wlock);
    return 0;
}

int proc_del_b(dev_t dev, unsigned long ino)
{
    struct black_proc *e;

    spin_lock(&black_wlock);
    list_for_each_entry(e, &black_procs, list) {
        if (e->dev == dev && e->ino == ino) {
            list_del_rcu(&e->list);
            spin_unlock(&black_wlock);
            kfree_rcu(e, rcu);
            return 0;
        }
    }
    spin_unlock(&black_wlock);
    return -ENOENT;
}

int proc_del_w(dev_t dev, unsigned long ino)
{
    struct white_proc *e;

    spin_lock(&white_wlock);
    list_for_each_entry(e, &white_procs, list) {
        if (e->dev == dev && e->ino == ino) {
            list_del_rcu(&e->list);
            spin_unlock(&white_wlock);
            kfree_rcu(e, rcu);
            return 0;
        }
    }
    spin_unlock(&white_wlock);
    return -ENOENT;
}

bool should_hook(struct inode *inode, struct task_struct *task)
{
    dev_t exe_dev = 0;
    unsigned long exe_ino = 0;
    bool has_exe = false;

    rcu_read_lock();

    if (task->mm) {
        struct file *exe = rcu_dereference(task->mm->exe_file);
        if (exe && exe->f_inode) {
            exe_dev = exe->f_inode->i_sb->s_dev;
            exe_ino = exe->f_inode->i_ino;
            has_exe = true;
            printk(KERN_INFO "[HOOK] exe dev=%u ino=%lu comm=%s\n",
                   (unsigned)exe_dev, exe_ino, task->comm);
        }
    }

    struct black_file *bf;
    list_for_each_entry_rcu(bf, &black_files, list) {
        if (bf->ino == inode->i_ino && bf->dev == inode->i_sb->s_dev)
            goto skip;
    }

    if (has_exe) {
        struct black_proc *bp;
        list_for_each_entry_rcu(bp, &black_procs, list) {
            if (bp->dev == exe_dev && bp->ino == exe_ino)
                goto skip;
        }
    }

    if (!list_empty(&white_files)) {
        bool hit = false;
        struct white_file *wf;
        list_for_each_entry_rcu(wf, &white_files, list) {
            if (wf->ino == inode->i_ino && wf->dev == inode->i_sb->s_dev) {
                hit = true; break;
            }
        }
        if (!hit) goto skip;
    }

    if (!list_empty(&white_procs) && has_exe) {
        bool hit = false;
        struct white_proc *wp;
        list_for_each_entry_rcu(wp, &white_procs, list) {
            if (wp->dev == exe_dev && wp->ino == exe_ino) {
                hit = true; break;
            }
        }
        if (!hit) goto skip;
    }

    rcu_read_unlock();
    return true;

skip:
    rcu_read_unlock();
    return false;
}

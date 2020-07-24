/*
 * Copyright (c) 2009-~ Helight.Xu
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License
 *
 * Author:       Helight.Xu<Helight.Xu@gmail.com>
 * Created Time: Mon 01 Feb 2010 04:46:06 PM HKT
 * File Name:    file.c
 *
 * Description:  
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define MAX_SIZE 1024

extern struct dentry *auditfs_create_file(const char *name, mode_t mode,
                                        struct dentry *parent, void *data,
                                        const struct file_operations *fops);
extern struct dentry *auditfs_create_dir(const char *name, struct dentry *parent);
extern void auditfs_remove(struct dentry *dentry);

static char buff[MAX_SIZE];

static ssize_t stat_open(struct inode *inode, struct file *file)
{
        return 0;
}

static ssize_t
stat_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
        int len = 0;
        long int p = *ppos;

        len = strlen(buff);
        if (p >= len - 1)
                return 0;
        if (copy_to_user(buf, (void *)buff, len))
                return -EFAULT;
        *ppos += len;
        return len;
}

static ssize_t
stat_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
        memset(buff, '\0', sizeof(buff));
        if (copy_from_user(buff, buf, count))
                return -EFAULT;
        return count;
}

static ssize_t stat_release(struct inode *inode, struct file *file)
{
        return 0;
}

const struct file_operations stat_ops = {
        .open =         stat_open,
        .read =         stat_read,
        .write =        stat_write,
        .release =      stat_release
};

static struct dentry *audit_stat;

static __init int init_audit_file(void)
{
	memset(buff, '\0', sizeof(buff));
	strcpy(buff, "helight!!!\n");
        audit_stat = auditfs_create_file("auditfs_stat", 0666, NULL, 0, &stat_ops);

        return 0;
}

static void __exit exit_audit_file(void)
{
        auditfs_remove(audit_stat);

        return;
}

module_init(init_audit_file);
module_exit(exit_audit_file);
MODULE_LICENSE("Dual BSD/GPL");


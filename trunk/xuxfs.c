#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/fs.h>
#include <linux/fsnotify.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/kobject.h>
#include <linux/magic.h>
#include <linux/mount.h>
//#include "xuxfs.h"



#define XUXFS_MAGIC 0x3966477
#define XUXFS_DIR   0x0001
#define PAGE_CACHE_MASK         PAGE_MASK
#define PAGE_MASK       (~(PAGE_SIZE-1))

extern const struct file_operations xuxfs_file_operations;

static struct vfsmount *xuxfs_mount;
static int xuxfs_mount_count;


struct super_block * xuxfs_sb = NULL;

static bool xuxfs_registered;


static struct inode *xuxfs_get_inode(struct super_block *sb, int mode, dev_t dev)
{
        struct inode *inode = new_inode(sb);

        if (inode) {
                inode->i_mode = mode;
                inode->i_uid = 0;
                inode->i_gid = 0;
                inode->i_blocks = 0;
                inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
                        inode->i_fop = &xuxfs_file_operations;
                }
        
        return inode;
}

static const struct super_operations sysfs_ops = {
        .statfs         = simple_statfs,
        .drop_inode     = generic_delete_inode,
};


static int xuxfs_mknod(struct inode *dir, struct dentry *dentry,
                         int mode, dev_t dev)
{
        struct inode *inode;
        int error = -EPERM;

        if (dentry->d_inode)
                return -EEXIST;

        inode = xuxfs_get_inode(dir->i_sb, mode, dev);
        if (inode) {
                d_instantiate(dentry, inode);
                dget(dentry);
                error = 0;
        }
        return error;
}

static int xuxfs_create(struct inode *dir, struct dentry *dentry, int mode)
{
        int res;

        mode = (mode & S_IALLUGO) | S_IFREG;
        res = xuxfs_mknod(dir, dentry, mode, 0);
        if (!res)
                fsnotify_create(dir, dentry);
        return res;
}

static inline int xuxfs_positive(struct dentry *dentry)
{
        return dentry->d_inode && !d_unhashed(dentry);
}


static int xuxfs_fill_super(struct super_block *sb, void *data, int silent)
{
        static struct tree_descr xuxfs_files[] = {{""}};

        return simple_fill_super(sb, XUXFS_MAGIC, xuxfs_files);
}




static int xux_get_sb(struct file_system_type *fs_type,
			int flags, const char *dev_name,
			void *data, struct vfsmount *mnt)
{
	return get_sb_single(fs_type, flags, data, xuxfs_fill_super, mnt);
}

static struct file_system_type xux_fs_type = {
	.owner =	THIS_MODULE,
	.name =		"xuxfs",
	.get_sb =	xux_get_sb,
	.kill_sb =	kill_litter_super,
};


static int xuxfs_create_by_name(const char *name, mode_t mode,
                                  struct dentry *parent,
                                  struct dentry **dentry)
{
        int error = 0;

        if (!parent) {
                if (xuxfs_mount && xuxfs_mount->mnt_sb) {
                        parent = xuxfs_mount->mnt_sb->s_root;
                }
        }
        if (!parent) {
                return -EFAULT;
        }

        *dentry = NULL;
        *dentry = lookup_one_len(name, parent, strlen(name));
			 error = xuxfs_create(parent->d_inode, *dentry, mode);
                dput(*dentry);

        return error;
}

static ssize_t default_read_file(struct file *file, char __user *buf,
                                 size_t count, loff_t *ppos)
{
        return 0;
}

static ssize_t default_write_file(struct file *file, const char __user *buf,
                                   size_t count, loff_t *ppos)
{
        return count;
}

static int default_open(struct inode *inode, struct file *file)
{
        if (inode->i_private)
                file->private_data = inode->i_private;

        return 0;
}

const struct file_operations xuxfs_file_operations = {
        .read =         default_read_file,
        .write =        default_write_file,
        .open =         default_open,
};


struct dentry *xuxfs_create_file(const char *name, mode_t mode,
           struct dentry *parent, void *data,
                                   const struct file_operations *fops)
{
        struct dentry *dentry = NULL;
        int error;


        error = simple_pin_fs(&xux_fs_type, &xuxfs_mount,
                              &xuxfs_mount_count);
        if (error)
                goto exit;

        error = xuxfs_create_by_name(name, mode, parent, &dentry);
        if (error) {
                dentry = NULL;
                simple_release_fs(&xuxfs_mount, &xuxfs_mount_count);
                goto exit;
        }

        if (dentry->d_inode) {
                if (data)
                        dentry->d_inode->i_private = data;
                if (fops)
                        dentry->d_inode->i_fop = fops;
        }
exit:
        return dentry;
}
EXPORT_SYMBOL_GPL(xuxfs_create_file);


static struct kobject *xux_kobj;

static int __init xuxfs_init(void)
{

	int retval;


	retval = register_filesystem(&xux_fs_type);
	if (retval)
		kobject_put(xux_kobj);
	else
		xuxfs_registered = true;

	return retval;
}

static void __exit xuxfs_exit(void)
{
	xuxfs_registered = false;

	simple_release_fs(&xuxfs_mount, &xuxfs_mount_count);
	unregister_filesystem(&xux_fs_type);
	kobject_put(xux_kobj);
}

module_init(xuxfs_init);
module_exit(xuxfs_exit);
MODULE_LICENSE("GPL");


#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/fs.h>
#include <linux/fsnotify.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/kobject.h>
#include <linux/magic.h>
#include <linux/mount.h>
//#include "myfs.h"



#define XUXFS_MAGIC 0x3966477
#define XUXFS_DIR   0x0001
#define PAGE_CACHE_MASK         PAGE_MASK
#define PAGE_MASK       (~(PAGE_SIZE-1))


static struct vfsmount *myfs_mount ;
static int myfs_mount_count;

struct super_block * myfs_sb = NULL;

static bool myfs_registered;

static const struct super_operations sysfs_ops = {
        .statfs         = simple_statfs,
        .drop_inode     = generic_delete_inode,
};

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

const struct file_operations myfs_file_operations = {
        .read =         default_read_file,
        .write =        default_write_file,
        .open =         default_open,
};



static struct inode *myfs_get_inode(struct super_block *sb, int mode, dev_t dev)
{
        struct inode *inode = new_inode(sb);

        if (inode) {
                inode->i_mode = mode;
                inode->i_uid = 0;
                inode->i_gid = 0;
                inode->i_blocks = 0;
                inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
                        inode->i_fop = &myfs_file_operations;
                }
        
        return inode;
}


static int myfs_mknod(struct inode *dir, struct dentry *dentry,
                         int mode, dev_t dev)
{
        struct inode *inode;
        int error = -EPERM;

        if (dentry->d_inode)
                return -EEXIST;

        inode = myfs_get_inode(dir->i_sb, mode, dev);
        if (inode) {
                d_instantiate(dentry, inode);
                dget(dentry);
                error = 0;
        }
        return error;
}

static int myfs_create(struct inode *dir, struct dentry *dentry, int mode)
{
        int res;

        mode = (mode & S_IALLUGO) | S_IFREG;
        res = myfs_mknod(dir, dentry, mode, 0);
        if (!res)
                fsnotify_create(dir, dentry);
        return res;
}

static inline int myfs_positive(struct dentry *dentry)
{
        return dentry->d_inode && !d_unhashed(dentry);
}


static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
        static struct tree_descr myfs_files[] = {{""}};

        return simple_fill_super(sb, XUXFS_MAGIC, myfs_files);
}




static int my_get_sb(struct file_system_type *fs_type,
			int flags, const char *dev_name,
			void *data, struct vfsmount *mnt)
{
	return get_sb_single(fs_type, flags, data, myfs_fill_super, mnt);
}

static struct file_system_type my_fs_type = {
	.owner =	THIS_MODULE,
	.name =		"myfs",
	.get_sb =	my_get_sb,
	.kill_sb =	kill_litter_super,
};


static int myfs_create_by_name(const char *name, mode_t mode,
                                  struct dentry *parent,
                                  struct dentry **dentry)
{
        int error = 0;

        if (!parent) {
                if (myfs_mount && myfs_mount->mnt_sb) {
                        parent = myfs_mount->mnt_sb->s_root;
                }
        }
        if (!parent) {
                return -EFAULT;
        }

        *dentry = NULL;
        *dentry = lookup_one_len(name, parent, strlen(name));
			 error = myfs_create(parent->d_inode, *dentry, mode);
                dput(*dentry);

        return error;
}



struct dentry *myfs_create_file(const char *name, mode_t mode,
           struct dentry *parent, void *data,
                                   const struct file_operations *fops)
{
        struct dentry *dentry = NULL;
        int error;


        error = simple_pin_fs(&my_fs_type, &myfs_mount,
                              &myfs_mount_count);
        if (error)
                goto exit;

        error = myfs_create_by_name(name, mode, parent, &dentry);
        if (error) {
                dentry = NULL;
                simple_release_fs(&myfs_mount, &myfs_mount_count);
                goto exit;
        }

        if (dentry->d_inode) {
                if (data)
                        dentry->d_inode->i_private = data;
                if (fops)
                        dentry->d_inode->i_fop = fops;
        }
exit:
        printk("\n%s\t%s", "infuction myfs_create_file", (fops!=NULL)?"yes":"no");
	return dentry;
}
EXPORT_SYMBOL_GPL(myfs_create_file);
struct dentry *myfs_create_dir(const char *name, struct dentry *parent)
{
        return myfs_create_file(name,
                                   S_IFDIR | S_IRWXU | S_IRUGO | S_IXUGO,
                                   parent, NULL, NULL);
}
EXPORT_SYMBOL_GPL(myfs_create_dir);


static int __init myfs_init(void)
{

	int retval;


	retval = register_filesystem(&my_fs_type);
       if (!retval) {
                myfs_mount = vfs_kern_mount(&my_fs_type,MS_KERNMOUNT,(&my_fs_type)->name,NULL);
                if (IS_ERR(myfs_mount)) {
                        printk(KERN_ERR "myfs: could not mount!\n");
                        retval= PTR_ERR(myfs_mount);
                        myfs_mount = NULL;
                        unregister_filesystem(&my_fs_type);
                        return retval;
                }
	}
	return 0;
}

static void __exit myfs_exit(void)
{
	myfs_registered = false;

	simple_release_fs(&myfs_mount, &myfs_mount_count);
	unregister_filesystem(&my_fs_type);
}

module_init(myfs_init);
module_exit(myfs_exit);
MODULE_LICENSE("GPL");


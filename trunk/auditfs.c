#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fsnotify.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/kobject.h>
#include <linux/magic.h>
#include <linux/mount.h>
#include <linux/pagemap.h>

#define XUXFS_MAGIC 0x3966477
#define XUXFS_DIR   0x0001
#define PAGE_CACHE_MASK         PAGE_MASK
#define PAGE_MASK       (~(PAGE_SIZE-1))

static struct vfsmount *auditfs_mount ;
static int auditfs_mount_count;
struct super_block * auditfs_sb = NULL;
static bool auditfs_registered = NULL;

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

const struct file_operations auditfs_file_operations = {
        .read =         default_read_file,
        .write =        default_write_file,
        .open =         default_open,
};

static struct inode *auditfs_get_inode(struct super_block *sb, int mode, dev_t dev)
{
        struct inode *inode = new_inode(sb);

        if (inode) {
                inode->i_mode = mode;
                inode->i_uid = 0;
                inode->i_gid = 0;
                inode->i_blocks = 0;
                inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
                inode->i_fop = &auditfs_file_operations;
                }
        
        return inode;
}

static int auditfs_mknod(struct inode *dir, struct dentry *dentry,
                         int mode, dev_t dev)
{
        struct inode *inode;
        int error = -EPERM;

        if (dentry->d_inode)
                return -EEXIST;

        inode = auditfs_get_inode(dir->i_sb, mode, dev);
        if (inode) {
                d_instantiate(dentry, inode);
                dget(dentry);
                error = 0;
        }
        return error;
}

static int auditfs_create(struct inode *dir, struct dentry *dentry, int mode)
{
        int res;

        mode = (mode & S_IALLUGO) | S_IFREG;
        res = auditfs_mknod(dir, dentry, mode, 0);
        if (!res)
                fsnotify_create(dir, dentry);
        return res;
}

static int auditfs_fill_super(struct super_block *sb, void *data, int silent)
{
        static struct tree_descr auditfs_files[] = {{""}};

        return simple_fill_super(sb, XUXFS_MAGIC, auditfs_files);
}

static int audit_get_sb(struct file_system_type *fs_type,
			int flags, const char *dev_name,
			void *data, struct vfsmount *mnt)
{
	return get_sb_single(fs_type, flags, data, auditfs_fill_super, mnt);
}

static struct file_system_type audit_fs_type = {
	.owner =	THIS_MODULE,
	.name =		"auditfs",
	.get_sb =	audit_get_sb,
	.kill_sb =	kill_litter_super,
};

static int auditfs_create_by_name(const char *name, mode_t mode,
                                  struct dentry *parent,
                                  struct dentry **dentry)
{
        int error = 0;

        if (!parent) {
                if (auditfs_mount && auditfs_mount->mnt_sb) {
                        parent = auditfs_mount->mnt_sb->s_root;
                }
        }
        if (!parent) {
                return -EFAULT;
        }

        *dentry = NULL;
        *dentry = lookup_one_len(name, parent, strlen(name));
			 error = auditfs_create(parent->d_inode, *dentry, mode);
                dput(*dentry);

        return error;
}

struct dentry *auditfs_create_file(const char *name, mode_t mode,
			           struct dentry *parent, void *data,
                                   const struct file_operations *fops)
{
        struct dentry *dentry = NULL;
        int error;


        error = simple_pin_fs(&audit_fs_type, &auditfs_mount, &auditfs_mount_count);
        if (error)
                goto exit;

        error = auditfs_create_by_name(name, mode, parent, &dentry);
        if (error) {
                dentry = NULL;
                simple_release_fs(&auditfs_mount, &auditfs_mount_count);
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
EXPORT_SYMBOL_GPL(auditfs_create_file);

struct dentry *auditfs_create_dir(const char *name, struct dentry *parent)
{
        return auditfs_create_file(name,
                                   S_IFDIR | S_IRWXU | S_IRUGO | S_IXUGO,
                                   parent, NULL, NULL);
}
EXPORT_SYMBOL_GPL(auditfs_create_dir);

static inline int auditfs_positive(struct dentry *dentry)
{
        return dentry->d_inode && !d_unhashed(dentry);
}

static void __auditfs_remove(struct dentry *dentry, struct dentry *parent)
{
        int ret = 0;

        if (auditfs_positive(dentry)) {
                if (dentry->d_inode) {
                        dget(dentry);
                        switch (dentry->d_inode->i_mode & S_IFMT) {
                        case S_IFDIR:
                                ret = simple_rmdir(parent->d_inode, dentry);
                                break;
                        case S_IFLNK:
                                kfree(dentry->d_inode->i_private);
                                /* fall through */
                        default:
                                simple_unlink(parent->d_inode, dentry);
                                break;
                        }
                        if (!ret)
                                d_delete(dentry);
                        dput(dentry);
                }
        }
}

void auditfs_remove(struct dentry *dentry)
{
        struct dentry *parent;

        if (!dentry)
                return;

        parent = dentry->d_parent;
        if (!parent || !parent->d_inode)
                return;

        mutex_lock(&parent->d_inode->i_mutex);
        __auditfs_remove(dentry, parent);
        mutex_unlock(&parent->d_inode->i_mutex);
        simple_release_fs(&auditfs_mount, &auditfs_mount_count);
}
EXPORT_SYMBOL_GPL(auditfs_remove);

static int __init auditfs_init(void)
{
	int ret = 0;

	ret = register_filesystem(&audit_fs_type);
	if (ret) 
		return ret;
	
        ret = simple_pin_fs(&audit_fs_type, &auditfs_mount, &auditfs_mount_count);
	if (ret) {
		ret= PTR_ERR(auditfs_mount);
		auditfs_mount = NULL;
		unregister_filesystem(&audit_fs_type);
		return ret;
	}
	return 0;
}

static void __exit auditfs_exit(void)
{
	auditfs_registered = false;

	simple_release_fs(&auditfs_mount, &auditfs_mount_count);
	unregister_filesystem(&audit_fs_type);
}

module_init(auditfs_init);
module_exit(auditfs_exit);
MODULE_LICENSE("Dual BSD/GPL");


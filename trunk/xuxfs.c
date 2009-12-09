#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/magic.h>

static struct vfsmount *xuxfs_mount;
static int xuxfs_mount_count;
static bool xuxfs_registered;
/*
static struct inode *debugfs_get_inode(struct super_block *sb, int mode, dev_t dev)
{
	struct inode *inode = new_inode(sb);

	if (inode) {
		inode->i_mode = mode;
		inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
		switch (mode & S_IFMT) {
		default:
			init_special_inode(inode, mode, dev);
			break;
		case S_IFREG:
			inode->i_fop = &debugfs_file_operations;
			break;
		case S_IFLNK:
			inode->i_op = &debugfs_link_operations;
			break;
		case S_IFDIR:
			inode->i_op = &simple_dir_inode_operations;
			inode->i_fop = &simple_dir_operations;
*/
			/* directory inodes start off with i_nlink == 2
			 * (for "." entry) */
/*			inc_nlink(inode);
			break;
		}
	}
	return inode; 
}
*/
static int xux_fill_super(struct super_block *sb, void *data, int silent)
{
	static struct tree_descr xux_files[] = {{""}};

	return simple_fill_super(sb, DEBUGFS_MAGIC, xux_files);
}

static int xux_get_sb(struct file_system_type *fs_type,
			int flags, const char *dev_name,
			void *data, struct vfsmount *mnt)
{
	return get_sb_single(fs_type, flags, data, xux_fill_super, mnt);
}

static struct file_system_type xux_fs_type = {
	.owner =	THIS_MODULE,
	.name =		"xuxfs",
	.get_sb =	xux_get_sb,
	.kill_sb =	kill_litter_super,
};

static struct kobject *xux_kobj;

static int __init xuxfs_init(void)
{
	int retval;

	xux_kobj = kobject_create_and_add("xuxfs", kernel_kobj);
	if (!xux_kobj)
		return -EINVAL;

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


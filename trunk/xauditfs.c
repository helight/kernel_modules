#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/magic.h>

static struct vfsmount *xauditfs_mount;
static int xauditfs_mount_count;
static bool xauditfs_registered;
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
static int xaudit_fill_super(struct super_block *sb, void *data, int silent)
{
	static struct tree_descr xaudit_files[] = {{""}};

	return simple_fill_super(sb, XAUDITFS_MAGIC, xaudit_files);
}

static int xaudit_get_sb(struct file_system_type *fs_type,
			int flags, const char *dev_name,
			void *data, struct vfsmount *mnt)
{
	return get_sb_single(fs_type, flags, data, xaudit_fill_super, mnt);
}

static struct file_system_type xaudit_fs_type = {
	.owner =	THIS_MODULE,
	.name =		"xauditfs",
	.get_sb =	xaudit_get_sb,
	.kill_sb =	kill_litter_super,
};


static int __init xauditfs_init(void)
{
	int retval;

	retval = register_filesystem(&xaudit_fs_type);
	if (!retval)
		xauditfs_registered = true;

	return retval;
}

static void __exit xauditfs_exit(void)
{
	xauditfs_registered = false;

	simple_release_fs(&xauditfs_mount, &xauditfs_mount_count);
	unregister_filesystem(&xaudit_fs_type);
}

module_init(xauditfs_init);
module_exit(xauditfs_exit);
MODULE_LICENSE("GPL");


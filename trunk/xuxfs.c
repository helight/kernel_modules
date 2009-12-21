#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/magic.h>




#define XUXFS_MAGIC 0x3966477
#define XUXFS_DIR   0x0001
#define PAGE_CACHE_SHIFT        12
#define PAGE_CACHE_SIZE         256
#define PAGE_CACHE_MASK         PAGE_MASK
#define PAGE_MASK       (~(PAGE_SIZE-1))


struct xuxfs_dirent {
        atomic_t                s_count;
        atomic_t                s_active;
        struct xuxfs_dirent     *s_parent;
        struct xuxfs_dirent     *s_sibling;
        const char              *s_name;

     /*   union {
                struct xuxfs_elem_dir           s_dir;
                struct xuxfs_elem_symlink       s_symlink;
                struct xuxfs_elem_attr          s_attr;
                struct xuxfs_elem_bin_attr      s_bin_attr;
        };*/

        unsigned int            s_flags;
        ino_t                   s_ino;
        umode_t                 s_mode;
        struct iattr            *s_iattr;
};


static struct vfsmount *xuxfs_mount;
struct super_block * xuxfs_sb = NULL;

static int xuxfs_mount_count;
static bool xuxfs_registered;

static const struct super_operations sysfs_ops = {
        .statfs         = simple_statfs,
        .drop_inode     = generic_delete_inode,
};

struct xuxfs_dirent xuxfs_root = {
        .s_name         = "",
        .s_count        = ATOMIC_INIT(1),
        .s_flags        = XUXFS_DIR,
        .s_mode         = S_IFDIR | S_IRWXU | S_IRUGO | S_IXUGO,
        .s_ino          = 1,
};

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


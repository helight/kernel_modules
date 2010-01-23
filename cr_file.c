
#include <linux/module.h>
#include <linux/fs.h>


extern struct dentry *auditfs_create_file(const char *name, mode_t mode,
					struct dentry *parent, void *data,
					const struct file_operations *fops);
extern struct dentry *auditfs_create_dir(const char *name, struct dentry *parent);

unsigned long __read_mostly     test_thresh;

static ssize_t default_read_file(struct file *file, char __user *buf,
                                 size_t count, loff_t *ppos)
{
        return -1;
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
static int auditfs_u8_set(void *data, u64 val)
{
        *(u8 *)data = val;
        return 0;
}
static int auditfs_u8_get(void *data, u64 *val)
{
        *val = *(u8 *)data;
        return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(fops_u8, auditfs_u8_get, auditfs_u8_set, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(fops_u8_ro, auditfs_u8_get, NULL, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(fops_u8_wo, NULL, auditfs_u8_set, "%llu\n");
struct dentry *auditfs_create_u8(const char *name, mode_t mode,
                                 struct dentry *parent, u8 *value)
{
        /* if there are no write bits set, make read only */
        if (!(mode & S_IWUGO))
                return auditfs_create_file(name, mode, parent, value, &fops_u8_ro);
        /* if there are no read bits set, make write only */
        if (!(mode & S_IRUGO))
                return auditfs_create_file(name, mode, parent, value, &fops_u8_wo);

        return auditfs_create_file(name, mode, parent, value, &fops_u8);
}
EXPORT_SYMBOL_GPL(auditfs_create_u8);
static u8 auditfs_test;


static struct dentry *d_audit;

static __init int my_init_auditfs(void)
{

	int error;
	if(!d_audit)
	 d_audit = auditfs_create_dir("audit", NULL);

        if (!kernel_kobj) {
                error = -ENOMEM;
                return error;
        }

	auditfs_create_u8("auditfs_test",0664,NULL,&auditfs_test);

	return 0;
}

static void __exit my_exit_auditfs(void)
{
	
	return;
}

module_init(my_init_auditfs);
module_exit(my_exit_auditfs);

MODULE_LICENSE("GPL");


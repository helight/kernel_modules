
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#define MAX_SIZE 1024


extern struct dentry *myfs_create_file(const char *name, mode_t mode,
					struct dentry *parent, void *data,
					const struct file_operations *fops);
extern struct dentry *myfs_create_dir(const char *name, struct dentry *parent);

static int my_open_file(struct inode *inode, struct file *file)
{
//	file->private_data = message;
}
static ssize_t my_read_file(struct file *file, char __user *buf,
                                 size_t count, loff_t *ppos)
{
	int p = *ppos;
	if (p>=14)
		return 0; 	
      const char message[]="come from kernel!";
	copy_to_user(buf, (void *)message, count);
	printk("\nthis function be called\n");
	return count;
}

static ssize_t my_write_file(struct file *file, const char __user *buf, 
                                   size_t count, loff_t *ppos)
{
        return count;
}
struct file_operations fops = {
	.open  = my_open_file,
	.read  = my_read_file,
	.write = my_write_file,
};
struct file_operations fops_ro = {
	.open  = my_open_file,
	.read  = my_read_file,
};
struct file_operations fops_wo = {
	.open  = my_open_file,
	.write = my_write_file,
};
struct dentry *myfs_create_u8(const char *name, mode_t mode,
                                 struct dentry *parent, u8 *value)
{
        /* if there are no write bits set, make read only */
        if (!(mode & S_IWUGO))
                return myfs_create_file(name, mode, parent, value, &fops_ro);
        /* if there are no read bits set, make write only */
        if (!(mode & S_IRUGO))
                return myfs_create_file(name, mode, parent, value, &fops_wo);

        return myfs_create_file(name, mode, parent, value, &fops);
}
EXPORT_SYMBOL_GPL(myfs_create_u8);
static u8 myfs_test;


static struct dentry *d_my;

static __init int my_init_myfs(void)
{

	if(!d_my)
	 d_my = myfs_create_dir("my", NULL);

	myfs_create_u8("myfs_test",0664,NULL,&myfs_test);

	return 0;
}

static void __exit my_exit_myfs(void)
{
	return;
}

module_init(my_init_myfs);
module_exit(my_exit_myfs);

MODULE_LICENSE("GPL");



#include <linux/module.h>
#include <linux/fs.h>
extern struct dentry *auditfs_create_file(const char *name, mode_t mode,
					struct dentry *parent, void *data,
					const struct file_operations *fops);

unsigned long __read_mostly     test_thresh;

int test_open_generic(struct inode *inode, struct file *filp)
{

        filp->private_data = inode->i_private;
        return 0;
}

static const struct file_operations test_mark_fops = {
        .open           = test_open_generic,
//        .write          = test_mark_write,
};


static __init int my_init_auditfs(void)
{
         struct dentry *d_tracer;

        struct dentry *entry = NULL;


        entry = auditfs_create_file("auditfs_enabled", 0644, d_tracer,&test_thresh , &test_mark_fops);
        if (!entry)
                pr_warning("Could not create auditfs 'auditfs_enabled' entry\n");

        entry = auditfs_create_file("auditfs_options", 0644, d_tracer,
                                    &test_thresh, &test_mark_fops);
        if (!entry)
                pr_warning("Could not create auditfs 'auditfs_options' entry\n");

	return 0;
}

static void __exit my_exit_auditfs(void)
{
	return;
}

module_init(my_init_auditfs);
module_exit(my_exit_auditfs);

MODULE_LICENSE("GPL");


/*
 * fs/xuxfs/xuxfs.h - xuxfs internal header file
 *
 * Copyright (c) 2001-3 Patrick Mochel
 * Copyright (c) 2007 SUSE Linux Products GmbH
 * Copyright (c) 2007 Tejun Heo <teheo@suse.de>
 *
 * This file is released under the GPLv2.
 */

struct xuxfs_open_dirent;

/* type-specific structures for xuxfs_dirent->s_* union members */
struct xuxfs_elem_dir {
	struct kobject		*kobj;
	/* children list starts here and goes through sd->s_sibling */
	struct xuxfs_dirent	*children;
};

struct xuxfs_elem_symlink {
	struct xuxfs_dirent	*target_sd;
};

struct xuxfs_elem_attr {
	struct attribute	*attr;
	struct xuxfs_open_dirent *open;
};

struct xuxfs_elem_bin_attr {
	struct bin_attribute	*bin_attr;
};

/*
 * xuxfs_dirent - the building block of xuxfs hierarchy.  Each and
 * every xuxfs node is represented by single xuxfs_dirent.
 *
 * As long as s_count reference is held, the xuxfs_dirent itself is
 * accessible.  Dereferencing s_elem or any other outer entity
 * requires s_active reference.
 */
struct xuxfs_dirent {
	atomic_t		s_count;
	atomic_t		s_active;
	struct xuxfs_dirent	*s_parent;
	struct xuxfs_dirent	*s_sibling;
	const char		*s_name;

	union {
		struct xuxfs_elem_dir		s_dir;
		struct xuxfs_elem_symlink	s_symlink;
		struct xuxfs_elem_attr		s_attr;
		struct xuxfs_elem_bin_attr	s_bin_attr;
	};

	unsigned int		s_flags;
	ino_t			s_ino;
	umode_t			s_mode;
	struct iattr		*s_iattr;
};

#define SD_DEACTIVATED_BIAS		INT_MIN

#define SYSFS_TYPE_MASK			0x00ff
#define SYSFS_DIR			0x0001
#define SYSFS_KOBJ_ATTR			0x0002
#define SYSFS_KOBJ_BIN_ATTR		0x0004
#define SYSFS_KOBJ_LINK			0x0008
#define SYSFS_COPY_NAME			(SYSFS_DIR | SYSFS_KOBJ_LINK)

#define SYSFS_FLAG_MASK			~SYSFS_TYPE_MASK
#define SYSFS_FLAG_REMOVED		0x0200

static inline unsigned int xuxfs_type(struct xuxfs_dirent *sd)
{
	return sd->s_flags & SYSFS_TYPE_MASK;
}

/*
 * Context structure to be used while adding/removing nodes.
 */
struct xuxfs_addrm_cxt {
	struct xuxfs_dirent	*parent_sd;
	struct inode		*parent_inode;
	struct xuxfs_dirent	*removed;
	int			cnt;
};

/*
 * mount.c
 */
extern struct xuxfs_dirent xuxfs_root;
extern struct super_block *xuxfs_sb;
extern struct kmem_cache *xuxfs_dir_cachep;

/*
 * dir.c
 */
extern struct mutex xuxfs_mutex;
extern struct mutex xuxfs_rename_mutex;
extern spinlock_t xuxfs_assoc_lock;

extern const struct file_operations xuxfs_dir_operations;
extern const struct inode_operations xuxfs_dir_inode_operations;

struct dentry *xuxfs_get_dentry(struct xuxfs_dirent *sd);
struct xuxfs_dirent *xuxfs_get_active_two(struct xuxfs_dirent *sd);
void xuxfs_put_active_two(struct xuxfs_dirent *sd);
void xuxfs_addrm_start(struct xuxfs_addrm_cxt *acxt,
		       struct xuxfs_dirent *parent_sd);
int xuxfs_add_one(struct xuxfs_addrm_cxt *acxt, struct xuxfs_dirent *sd);
void xuxfs_remove_one(struct xuxfs_addrm_cxt *acxt, struct xuxfs_dirent *sd);
void xuxfs_addrm_finish(struct xuxfs_addrm_cxt *acxt);

struct xuxfs_dirent *xuxfs_find_dirent(struct xuxfs_dirent *parent_sd,
				       const unsigned char *name);
struct xuxfs_dirent *xuxfs_get_dirent(struct xuxfs_dirent *parent_sd,
				      const unsigned char *name);
struct xuxfs_dirent *xuxfs_new_dirent(const char *name, umode_t mode, int type);

void release_xuxfs_dirent(struct xuxfs_dirent *sd);

int xuxfs_create_subdir(struct kobject *kobj, const char *name,
			struct xuxfs_dirent **p_sd);
void xuxfs_remove_subdir(struct xuxfs_dirent *sd);

static inline struct xuxfs_dirent *xuxfs_get(struct xuxfs_dirent *sd)
{
	if (sd) {
		WARN_ON(!atomic_read(&sd->s_count));
		atomic_inc(&sd->s_count);
	}
	return sd;
}

static inline void xuxfs_put(struct xuxfs_dirent *sd)
{
	if (sd && atomic_dec_and_test(&sd->s_count))
		release_xuxfs_dirent(sd);
}

/*
 * inode.c
 */
struct inode *xuxfs_get_inode(struct xuxfs_dirent *sd);
int xuxfs_setattr(struct dentry *dentry, struct iattr *iattr);
int xuxfs_hash_and_remove(struct xuxfs_dirent *dir_sd, const char *name);
int xuxfs_inode_init(void);

/*
 * file.c
 */
extern const struct file_operations xuxfs_file_operations;

int xuxfs_add_file(struct xuxfs_dirent *dir_sd,
		   const struct attribute *attr, int type);

/*
 * bin.c
 */
extern const struct file_operations bin_fops;

/*
 * symlink.c
 */
extern const struct inode_operations xuxfs_symlink_inode_operations;

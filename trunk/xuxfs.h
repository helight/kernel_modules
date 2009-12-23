
struct xuxfs_dirent {
	atomic_t		s_count;
	atomic_t		s_active;
	struct xuxfs_dirent	*s_parent;
	struct xuxfs_dirent	*s_sibling;
	const char		*s_name;


	unsigned int		s_flags;
	ino_t			s_ino;
	umode_t			s_mode;
	struct iattr		*s_iattr;
};



/*
 * file.c
 */
extern const struct file_operations xuxfs_file_operations;

int xuxfs_add_file(struct xuxfs_dirent *dir_sd,
		   const struct attribute *attr, int type);

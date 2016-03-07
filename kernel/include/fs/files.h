struct stat {
	uint32_t	st_dev;		/* device containing file */
	int32_t		st_ino;		/* inode number */
	int32_t		st_mode;	/* protection bits */
	int32_t		st_nlink;	/* hard links */
	int32_t		st_uid;		/* uid of owner */
	int32_t		st_gid;		/* group id of owner */
	int32_t		st_rdev;	/* device ID (if special) */
	uint32_t	st_size;	/* total size, bytes */
};

int32_t close(uint32_t fd);
int32_t open(const char *pathname, uint32_t flags, uint32_t mode);
int32_t read(uint32_t fd, void *buf, uint32_t count);
int32_t write(uint32_t fd, void *buf, uint32_t count);
int32_t stat(const char *pathname, struct stat *buf);

void fd_table_init(void);


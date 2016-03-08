struct timespec {
	uint32_t seconds;
	uint32_t ns;
};

struct stat {
	uint32_t	st_dev;		/* device containing file */
	int32_t		st_ino;		/* inode number */
	int32_t		st_mode;	/* protection bits */
	int32_t		st_nlink;	/* hard links */
	int32_t		st_uid;		/* uid of owner */
	int32_t		st_gid;		/* group id of owner */
	int32_t		st_rdev;	/* device ID (if special) */
	uint32_t	st_size;	/* total size, bytes */
	uint32_t	st_blksize;     /* I/O blocksize */
	uint32_t	st_blocks;      /* number of blocks allocated */
	struct timespec	st_atim;	/* access time */
	struct timespec	st_mtim;	/* modification time */
	struct timespec	st_ctim;	/* status change time */
};

struct statfs {
	uint32_t f_type;	/* Filesystem type */
	uint32_t f_bsize;	/* Filesystem blocksize */
	uint32_t f_blocks;	/* Total blocks */
	uint32_t f_bfree;	/* Free blocks */
	uint32_t f_bavail;	/* Free blocks avail to user */
	uint32_t f_files;	/* Total file nodes */
	uint32_t f_ffree;	/* Free file nodes */
	uint32_t f_fsid;	/* Filesystem id */
	uint32_t f_namelen;	/* Max filename length */
};

struct superblock_t {
	uint32_t size;
	uint32_t free;
};

int32_t close(uint32_t fd);
int32_t open(const char *pathname, uint32_t flags, uint32_t mode);
int32_t read(uint32_t fd, void *buf, uint32_t count);
int32_t write(uint32_t fd, void *buf, uint32_t count);
int32_t stat(const char *pathname, struct stat *buf);
int32_t statfs(const char *path, struct statfs *buf);
int32_t mount(const char *source, const char *target,
	const char *filesystemtype, uint32_t mountflags,
	const void *data);

void fd_table_init(void);


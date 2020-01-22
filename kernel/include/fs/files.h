/* We do things in hex in vmwos, none of this octal nonsense */
#define S_IFMT		0xf000	/* mask			*/
#define S_IFSOCK	0xc000	/* socket		*/
#define S_IFLNK		0xa000	/* symbolic link	*/
#define S_IFREG		0x8000	/* regular file		*/
#define S_IFBLK		0x6000	/* block device		*/
#define S_IFDIR		0x4000	/* directory		*/
#define S_IFCHR		0x2000	/* character device	*/
#define S_IFIFO		0x1000	/* FIFO			*/


struct vmwos_stat {
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
	uint32_t	st_atime;	/* access time */
	uint32_t	st_mtime;	/* modification time */
	uint32_t	st_ctime;	/* status change time */
};

struct vmwos_statfs {
	uint32_t f_type;	/* Filesystem type */
	uint32_t f_bsize;	/* Filesystem blocksize */
	uint32_t f_blocks;	/* Total blocks */
	uint32_t f_bfree;	/* Free blocks */
	uint32_t f_bavail;	/* Free blocks avail to user */
	uint32_t f_files;	/* Total file nodes */
	uint32_t f_ffree;	/* Free file nodes */
	uint32_t f_fsid;	/* Filesystem id */
	uint32_t f_namelen;	/* Max filename length */
	uint32_t f_frsize;	/* Fragment size */
	uint32_t f_flags;	/* Flags */
	uint32_t padding[5];

};

struct superblock_t {
	uint32_t size;
	uint32_t free;
};

struct vmwos_dirent {
	uint32_t        d_ino;
	uint32_t        d_off;
	uint32_t        d_reclen;
	char            d_name[];
};


int32_t close_syscall(uint32_t fd);
int32_t open_syscall(const char *pathname, uint32_t flags, uint32_t mode);
int32_t read_syscall(uint32_t fd, void *buf, uint32_t count);
int32_t write_syscall(uint32_t fd, void *buf, uint32_t count);
int32_t stat_syscall(const char *pathname, struct vmwos_stat *buf);
int32_t statfs_syscall(const char *path, struct vmwos_statfs *buf);
int32_t mount_syscall(const char *source, const char *target,
	const char *filesystemtype, uint32_t mountflags,
	const void *data);
int32_t getdents_syscall(uint32_t fd, struct vmwos_dirent *dirp, uint32_t count);
int32_t chdir_syscall(const char *pathname);
char *getcwd_syscall(char *buf, size_t size);
int64_t llseek_syscall(uint32_t fd, int64_t offset, int32_t whence);


void file_objects_init(void);
int32_t get_inode(const char *pathname);



/* We do things in hex in vmwos, none of this octal nonsense */
#define S_IFMT		0xf000	/* mask			*/
#define S_IFSOCK	0xc000	/* socket		*/
#define S_IFLNK		0xa000	/* symbolic link	*/
#define S_IFREG		0x8000	/* regular file		*/
#define S_IFBLK		0x6000	/* block device		*/
#define S_IFDIR		0x4000	/* directory		*/
#define S_IFCHR		0x2000	/* character device	*/
#define S_IFIFO		0x1000	/* FIFO			*/


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
	uint32_t	st_atime;	/* access time */
	uint32_t	st_mtime;	/* modification time */
	uint32_t	st_ctime;	/* status change time */
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


int32_t close(uint32_t fd);
int32_t open(const char *pathname, uint32_t flags, uint32_t mode);
int32_t read(uint32_t fd, void *buf, uint32_t count);
int32_t write(uint32_t fd, void *buf, uint32_t count);
int32_t stat(const char *pathname, struct stat *buf);
int32_t statfs(const char *path, struct statfs *buf);
int32_t mount(const char *source, const char *target,
	const char *filesystemtype, uint32_t mountflags,
	const void *data);
int32_t getdents(uint32_t fd, struct vmwos_dirent *dirp, uint32_t count);

void fd_table_init(void);

int32_t get_inode(const char *pathname);

int32_t chdir(const char *pathname);
char *getcwd(char *buf, size_t size);

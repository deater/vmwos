#define NUM_INODES	64

struct inode_type {
	uint32_t	device;		/* device containing file */
	int32_t		number;		/* inode number */
	int32_t		count;		/* total users of this */
	int32_t		mode;		/* protection bits */
	int32_t		hard_links;	/* hard links */
	int32_t		uid;		/* uid of owner */
	int32_t		gid;		/* group id of owner */
	int32_t		rdev;		/* device ID */
	uint64_t	size;		/* total size, bytes */
	uint32_t	blocksize;	/* I/O blocksize */
	uint32_t	blocks;		/* number of blocks allocated */
	uint64_t	atime;		/* access time */
	uint64_t	mtime;		/* modification time */
	uint64_t	ctime;		/* status change time */
	struct superblock_type *sb;	/* associated superblock */
};

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
	uint64_t	st_size;	/* total size, bytes */
	uint32_t	st_blksize;     /* I/O blocksize */
	uint32_t	st_blocks;      /* number of blocks allocated */
	uint64_t	st_atime;	/* access time */
	uint64_t	st_mtime;	/* modification time */
	uint64_t	st_ctime;	/* status change time */
};


int32_t stat_syscall(const char *pathname, struct vmwos_stat *buf);
int32_t chmod_syscall(const char *pathname, int32_t mode);

struct inode_type *inode_allocate(void);
int32_t inode_lookup_and_alloc(const char *pathname, struct inode_type **inode);
int32_t inode_free(struct inode_type *inode);

const char *split_filename(const char *start_ptr, char *name, int len);
const char *split_pathname(char *fullpath,int len);

int32_t truncate_inode(struct inode_type *inode, int64_t size);
int32_t truncate64_syscall(const char *path, uint64_t size);

int32_t unlink_syscall(const char *pathname);

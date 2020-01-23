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


int32_t stat_syscall(const char *pathname, struct vmwos_stat *buf);
int32_t get_inode(const char *pathname);

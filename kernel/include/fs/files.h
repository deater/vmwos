#define MAX_FILENAME_SIZE	256
#define	MAX_PATH_LEN		1024
#define MAX_SUBDIR_DEPTH	16

#define MAX_FD_PER_PROC 8
#define MAX_OPEN_FILES  64

struct file_object_operations;

struct file_object {
        uint64_t file_offset;				/* current offset */
	struct file_object_operations *file_ops;	/* file_ops struct */
        struct inode_type *inode;			/* inode */
        uint32_t count;					/* # users of file */
	uint32_t flags;					/* flags used at open */
	char name[MAX_PATH_LEN];			/* name of file */
};

struct vmwos_dirent {
	uint32_t        d_ino;
	uint32_t        d_off;
	uint32_t        d_reclen;
	char            d_name[];
};

struct superblock_type;

struct file_object_operations {
        int32_t (*read) (struct inode_type *, char *, uint32_t, uint64_t *);
        int32_t (*write) (struct inode_type *,
			const char *, uint32_t, uint64_t *);
        int64_t (*llseek) (struct file_object *, int64_t, int32_t);
        int32_t (*getdents) (struct inode_type *,
			uint64_t *, void *, uint32_t);
        int32_t (*ioctl) (struct file_object *, uint32_t, uint32_t);
        int32_t (*open) (int32_t *, struct file_object *);
//        int (*flush) (struct file *);
};

struct inode_type *file_get_inode(int32_t which);
int32_t open_file_object(struct file_object **file,
			const char *pathname, uint32_t flags, uint32_t mode);
int32_t file_object_free(struct file_object *file);



int32_t close_syscall(uint32_t fd);

#define O_RW_MASK	0x0003

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_CREAT         0x0040
#define O_EXCL          0x0080
#define O_NOCTTY        0x0100
#define O_TRUNC         0x0200
#define O_APPEND        0x0400
#define O_NONBLOCK      0x0800

int32_t open_syscall(const char *pathname, uint32_t flags, uint32_t mode);
int32_t read_syscall(uint32_t fd, void *buf, uint32_t count);
int32_t write_syscall(uint32_t fd, void *buf, uint32_t count);
int32_t getdents_syscall(uint32_t fd, struct vmwos_dirent *dirp, uint32_t count);
int32_t chdir_syscall(const char *pathname);
char *getcwd_syscall(char *buf, size_t size);
int64_t llseek_syscall(uint32_t fd, int64_t offset, int32_t whence);

void file_objects_init(void);
struct file_object *file_special(int which);


#define SEEK_SET	0	/* Seek from beginning of file */
#define SEEK_CUR	1	/* Seek from current position  */
#define SEEK_END	2	/* Seek from end of file       */


int64_t llseek_generic(struct file_object *file,
                int64_t offset, int32_t whence);

int32_t ftruncate64_syscall(int32_t fd, uint64_t size);

#define F_DUPFD		0	/* Duplicate fd    */
#define F_GETFD		1	/* Get fd flags    */
#define F_SETFD		2	/* Set fd flags    */
#define F_GETFL		3	/* Get file status */
#define F_SETFL		4	/* Set file status */

int32_t fcntl_syscall(uint32_t fd, int32_t cmd, uint32_t third);

void files_increment_count(struct file_object *file);

int32_t ioctl_syscall(uint32_t fd, int32_t cmd,
                                        uint32_t third, uint32_t fourth);


int32_t dup2_syscall(uint32_t oldfd, int32_t newfd);

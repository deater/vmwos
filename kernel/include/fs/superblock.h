#define MAX_MOUNTS	8

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

struct superblock_type;

struct superblock_operations {
	int32_t (*make_inode) (struct inode_type *, struct inode_type **);
	int32_t (*destroy_inode) (struct inode_type *);
//	void (*read_inode) (struct inode *);
//	void (*dirty_inode) (struct inode *);
	int32_t (*link_inode) (struct inode_type *, const char *);
	int32_t (*unlink_inode) (struct inode_type *);
	void (*write_inode) (struct inode_type *);
	int32_t (*truncate_inode) (struct inode_type *, uint64_t);
//	void (*put_inode) (struct inode *);
//	void (*drop_inode) (struct inode *);
//	void (*delete_inode) (struct inode *);
//	void (*put_super) (struct super_block *);
	void (*write_superblock) (struct superblock_type *);
//	int (*sync_fs) (struct super_block *, int);
	int32_t (*statfs) (struct superblock_type *, struct vmwos_statfs *);
	int32_t (*lookup_inode) (struct inode_type *, const char *);
	int32_t (*setup_fileops) (struct file_object *);
//	int (*remount_fs) (struct super_block *, int *, char *);
//	void (*clear_inode) (struct inode *);
//	void (*umount_begin) (struct super_block *);
};

struct superblock_type {
	int32_t valid;
	uint32_t blocks;
	uint32_t blocks_free;
	uint32_t block_size;
	int32_t root_dir;
	char mountpoint[MAX_PATH_LEN];
	struct superblock_operations sb_ops;	/* superblock operations */
	struct block_dev_type *block;
	void *private;
};


struct superblock_type *superblock_allocate(void);
struct superblock_type *superblock_lookup(const char *path);

int32_t statfs_syscall(const char *path, struct vmwos_statfs *buf);
int32_t mount_syscall(const char *source, const char *target,
	const char *filesystemtype, uint32_t mountflags,
	const void *data);

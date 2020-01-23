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

int32_t statfs_syscall(const char *path, struct vmwos_statfs *buf);
int32_t mount_syscall(const char *source, const char *target,
	const char *filesystemtype, uint32_t mountflags,
	const void *data);

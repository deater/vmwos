struct vmwos_statfs {
        uint32_t f_type;        /* Filesystem type */
        uint32_t f_bsize;       /* Filesystem blocksize */
        uint32_t f_blocks;      /* Total blocks */
        uint32_t f_bfree;       /* Free blocks */
        uint32_t f_bavail;      /* Free blocks avail to user */
        uint32_t f_files;       /* Total file nodes */
        uint32_t f_ffree;       /* Free file nodes */
        uint32_t f_fsid;        /* Filesystem id */
        uint32_t f_namelen;     /* Max filename length */
        uint32_t f_frsize;      /* Fragment size */
        uint32_t f_flags;       /* Flags */
        uint32_t padding[5];

};



int32_t close_syscall(uint32_t fd);
int32_t open_syscall(const char *pathname, uint32_t flags, uint32_t mode);
int32_t read_syscall(uint32_t fd, void *buf, uint32_t count);
int32_t write_syscall(uint32_t fd, void *buf, uint32_t count);
int32_t stat_syscall(const char *pathname, struct stat *buf);
int32_t statfs_syscall(const char *path, struct vmwos_statfs *buf);
int32_t mount_syscall(const char *source, const char *target,
        const char *filesystemtype, uint32_t mountflags,
        const void *data);
int32_t getdents_syscall(uint32_t fd, struct vmwos_dirent *dirp, uint32_t count);

int test_glue_setup(void);



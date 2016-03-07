int32_t close(uint32_t fd);
int32_t open(const char *pathname, uint32_t flags, uint32_t mode);
int32_t read(uint32_t fd, void *buf, uint32_t count);
int32_t write(uint32_t fd, void *buf, uint32_t count);

void fd_table_init(void);


uint32_t close(uint32_t fd);
uint32_t open(const char *pathname, uint32_t flags, uint32_t mode);
uint32_t read(uint32_t fd, void *buf, uint32_t count);
uint32_t write(uint32_t fd, void *buf, uint32_t count);

void fd_table_init(void);


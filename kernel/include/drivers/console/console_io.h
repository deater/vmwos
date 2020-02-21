#define CONSOLE_MAJOR	1

int console_write(const void *buf, size_t count);
int console_read(void *buf, size_t count, int non_blocking);
int console_insert_char(int ch);

void console_enable_locking(void);

struct char_dev_type *console_init(void);

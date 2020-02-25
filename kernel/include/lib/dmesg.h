#define SYSLOG_ACTION_READ_ALL          3
#define SYSLOG_ACTION_SIZE_BUFFER       10

int32_t dmesg_syscall(int32_t cmd, char *buf);


#define STDIN	0
#define STDOUT	1
#define STDERR	2

/* Same as Linux for compatibility */
//#define SYSCALL_EXIT		1
//#define SYSCALL_FORK		2
#define SYSCALL_READ		3
#define SYSCALL_WRITE		4
//#define SYSCALL_OPEN		5
//#define SYSCALL_CLOSE		6
//#define SYSCALL_EXECVE	11
#define SYSCALL_TIME		13
#define SYSCALL_GETPID		20
#define SYSCALL_IOCTL		54
#define SYSCALL_REBOOT		88
#define SYSCALL_NANOSLEEP	162

/* VMW syscalls */
#define SYSCALL_BLINK		8192
#define SYSCALL_SETFONT		8193
#define SYSCALL_GRADIENT	8194
#define SYSCALL_TB1		8195
#define SYSCALL_RUN		8196
#define SYSCALL_STOP		8197
#define SYSCALL_TEMPERATURE	8198

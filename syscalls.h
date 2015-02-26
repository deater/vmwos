static inline uint32_t syscall0(int which) {

	uint32_t result;

	asm volatile ("mov r7, %[which]\n"
			"swi 0\n"
			"mov %[result], r0\n"
		:	[result] "=r" (result)
                : 	[which] "r" (which)
                : 	"r7" );

	return result;

}

static inline uint32_t syscall1(int arg0, int which) {

	uint32_t result;

	asm volatile (  "mov r0, %[arg0]\n"
			"mov r7, %[which]\n"
			"swi 0\n"
			"mov %[result], r0\n"
		:	[result] "=r" (result)
                : 	[arg0] "r" (arg0),
			[which] "r" (which)
                : 	"r0", "r7" );

	return result;

}

static inline uint32_t syscall3(int arg0, int arg1, int arg2, int which) {

	uint32_t result;

	asm volatile (  "mov r0, %[arg0]\n"
			"mov r1, %[arg1]\n"
			"mov r2, %[arg2]\n"
			"mov r7, %[which]\n"
			"swi 0\n"
			"mov %[result], r0\n"
		:	[result] "=r" (result)
                : 	[arg0] "r" (arg0),
			[arg1] "r" (arg1),
			[arg2] "r" (arg2),
			[which] "r" (which)
                : 	"r0", "r1", "r2", "r7" );

	return result;

}



#define STDIN	0
#define STDOUT	1
#define STDERR	2

/* Same as Linux for compatibility */
#define SYSCALL_EXIT	1
#define SYSCALL_FORK	2
#define SYSCALL_READ	3
#define SYSCALL_WRITE	4
#define SYSCALL_OPEN	5
#define SYSCALL_CLOSE	6
#define SYSCALL_EXECVE	11

#define SYSCALL_BLINK	8192


#include <stddef.h>
#include <stdint.h>

#include "syscalls.h"
#include "vlibc.h"

/* From Linux kernel, arch/arm/include/uapi/asm/unistd.h */
/* That is auto-generated so not always there */
/* On a real pi2 look in /usr/include/arm-linux-gnueabihf/asm/unistd.h */
/* /usr/include/arm-linux-gnueabihf/asm/unistd-common.h */

#define __NR_exit		1
#define __NR_read		3
#define __NR_write		4
#define __NR_open		5
#define __NR_close		6
#define __NR_waitpid		7
#define __NR_link		9
#define __NR_unlink		10
#define __NR_execve		11
#define __NR_chdir		12
#define __NR_time		13
#define __NR_chmod		15
#define __NR_getpid		20
#define __NR_times		43
#define __NR_brk		45
#define __NR_ioctl		54
#define __NR_fcntl		55
#define __NR_dup2		63
#define __NR_reboot		88
#define __NR_mmap		90
#define __NR_munmap		91
#define __NR_statfs		99
#define __NR_dmesg		103	/* Linux calls this syslog() */
#define __NR_stat		106
#define __NR_fstat		108
#define __NR_sysinfo		116
#define	__NR_uname		122
#define __NR__llseek		140
#define __NR_getdents		141
#define __NR_nanosleep  	162
#define __NR_getcwd		183
#define __NR_vfork		190
#define __NR_truncate64		193
#define __NR_ftruncate64	194
#define __NR_clock_gettime	263
#define __NR_statfs64		266
#define __NR_getcpu		345

int32_t errno=0;

static int32_t update_errno(int32_t value) {

	if ((value<0) && (value>-MAX_ERRNO)) {
		errno=-value;
		return -1;
	}
	return value;
}

/* 1 */
int32_t exit(int32_t status) {

	register long r7 __asm__("r7") = __NR_exit;
	register long r0 __asm__("r0") = status;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return update_errno(r0);
}

/* 3 */
int32_t read(int fd, void *buf, size_t count) {

	register long r7 __asm__("r7") = __NR_read;
	register long r0 __asm__("r0") = fd;
	register long r1 __asm__("r1") = (long)buf;
	register long r2 __asm__("r2") = count;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return update_errno(r0);
}

/* 4 */
int32_t write(int fd, const void *buf, uint32_t size) {

	register long r7 __asm__("r7") = __NR_write;
	register long r0 __asm__("r0") = fd;
	register long r1 __asm__("r1") = (long)buf;
	register long r2 __asm__("r2") = size;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return update_errno(r0);
}

/* 5 */
int32_t open_direct(const char *filename, uint32_t flags, uint32_t mode) {

	register long r7 __asm__("r7") = __NR_open;
	register long r0 __asm__("r0") = (long)filename;
	register long r1 __asm__("r1") = flags;
	register long r2 __asm__("r2") = mode;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return update_errno(r0);
}

/* 6 */
int32_t close(uint32_t fd) {

	register long r7 __asm__("r7") = __NR_close;
	register long r0 __asm__("r0") = fd;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return update_errno(r0);
}

/* 7 */
int32_t waitpid(int32_t pid, int32_t *wstatus, int32_t options) {

	register long r7 __asm__("r7") = __NR_waitpid;
	register long r0 __asm__("r0") = (long)pid;
	register long r1 __asm__("r1") = (long)wstatus;
	register long r2 __asm__("r2") = (long)options;


	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	/* returns -1 on failure */
	return r0;

}

/* 10 */
int32_t unlink(const char *filename) {

	register long r7 __asm__("r7") = __NR_unlink;
	register long r0 __asm__("r0") = (long)filename;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return update_errno(r0);
}


/* 11 */
int32_t execve(const char *filename, char *const argv[],
		char *const envp[]) {

	register long r7 __asm__("r7") = __NR_execve;
	register long r0 __asm__("r0") = (long)filename;
	register long r1 __asm__("r1") = (long)argv;
	register long r2 __asm__("r2") = (long)envp;


	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return update_errno(r0);

}

/* 12 */
int32_t chdir(const char *path) {

	register long r7 __asm__("r7") = __NR_chdir;
	register long r0 __asm__("r0") = (long)path;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return update_errno(r0);
}

/* 15 */
int32_t chmod(const char *path, int32_t mode) {

	register long r7 __asm__("r7") = __NR_chmod;
	register long r0 __asm__("r0") = (long)path;
	register long r1 __asm__("r1") = (long)mode;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1)
		: "memory");

	return update_errno(r0);
}

/* 20 */
int32_t getpid(void) {

	register long r7 __asm__("r7") = __NR_getpid;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7) /* input */
		: "memory");

	/* Always successful */
	return r0;

}

/* 43 */
int32_t times(struct tms *buf) {

	register long r7 __asm__("r7") = __NR_times;
	register long r0 __asm__("r0") = (long)buf;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return update_errno(r0);
}

/* 63 */
int32_t dup2(int32_t oldfd, int32_t newfd) {

	register long r7 __asm__("r7") = __NR_dup2;
	register long r0 __asm__("r0") = (long)oldfd;
	register long r1 __asm__("r1") = (long)newfd;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1)
		: "memory");

	return update_errno(r0);
}

/* 103 */
int32_t dmesg(int32_t cmd, char *buf) {

	register long r7 __asm__("r7") = __NR_dmesg;
	register long r0 __asm__("r0") = (long)cmd;
	register long r1 __asm__("r1") = (long)buf;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1)
		: "memory");

	return update_errno(r0);
}

/* 106 */
int32_t stat(const char *pathname, struct stat *buf) {

	register long r7 __asm__("r7") = __NR_stat;
	register long r0 __asm__("r0") = (long)pathname;
	register long r1 __asm__("r1") = (long)buf;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1)
		: "memory");

	return r0;
}

/* 116 */
int32_t sysinfo(struct sysinfo *buf) {

	register long r7 __asm__("r7") = __NR_sysinfo;
	register long r0 __asm__("r0") = (long)buf;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return r0;
}

/* 122 */
int32_t uname(struct utsname *buf) {

	register long r7 __asm__("r7") = __NR_uname;
	register long r0 __asm__("r0") = (long)buf;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return r0;
}

/* 141 */
int32_t getdents(uint32_t fd, struct vmwos_dirent *dirp, uint32_t count) {

	register long r7 __asm__("r7") = __NR_getdents;
	register long r0 __asm__("r0") = (long)fd;
	register long r1 __asm__("r1") = (long)dirp;
	register long r2 __asm__("r2") = (long)count;


	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return r0;

}

/* 162 */
int32_t nanosleep(const struct timespec *req, struct timespec *rem) {

	register long r7 __asm__("r7") = __NR_nanosleep;
	register long r0 __asm__("r0") = (long)req;
	register long r1 __asm__("r1") = (long)rem;


	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1)
		: "memory");

	return r0;

}

/* 183 */
char *getcwd(char *buf, uint32_t size) {

	register long r7 __asm__("r7") = __NR_getcwd;
	register long r0 __asm__("r0") = (long)buf;
	register long r1 __asm__("r1") = (long)size;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1)
		: "memory");

	return (char *)r0;
}

/* 190 */
int32_t vfork(void) {

	register long r7 __asm__("r7") = __NR_vfork;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7) /* input */
		: "memory");

	return r0;

}

/* 54 */
int32_t ioctl4(int fd, unsigned long request, unsigned long req2, unsigned long req3) {

	register long r7 __asm__("r7") = __NR_ioctl;
	register long r0 __asm__("r0") = fd;
	register long r1 __asm__("r1") = request;
	register long r2 __asm__("r2") = req2;
	register long r3 __asm__("r3") = req3;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2), "r"(r3)
		: "memory");

	return r0;
}


int32_t sys_time(void) {

	register long r7 __asm__("r7") = __NR_time;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7) /* input */
		: "memory");

	return r0;

}

int32_t clock_gettime(uint32_t clk_id, struct timespec *t) {

	register long r7 __asm__("r7") = __NR_clock_gettime;
	register long r0 __asm__("r0") = clk_id;
	register long r1 __asm__("r1") = (long)t;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1)
		: "memory");

	return r0;
}


int32_t sys_reboot(void) {

	register long r7 __asm__("r7") = __NR_reboot;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7) /* input */
		: "memory");

	return r0;

}

/* 45 */
void *brk(void *address) {

	register long r7 __asm__("r7") = __NR_brk;
	register long r0 __asm__("r0") = (long)address;

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7), "0"(r0) /* input */
		: "memory");

	return (void *)r0;

}

/* 55 */
int32_t fcntl_direct(int32_t fd, int32_t cmd, int32_t paramater) {

	register long r7 __asm__("r7") = __NR_fcntl;
	register long r0 __asm__("r0") = (long)fd;
	register long r1 __asm__("r1") = (long)cmd;
	register long r2 __asm__("r2") = (long)paramater;

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2) /* input */
		: "memory");

        return 0;
}

/* 90 */
void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, int offset) {

	register long r7 __asm__("r7") = __NR_mmap;
	register long r0 __asm__("r0") = (long)addr;
	register long r1 __asm__("r1") = (long)length;
	register long r2 __asm__("r2") = (long)prot;
	register long r3 __asm__("r3") = (long)flags;
	register long r4 __asm__("r4") = (long)fd;
	register long r5 __asm__("r5") = (long)offset;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5)
		: "memory");

	return (void *)r0;
}

/* 91 */
int munmap(void *addr, size_t length) {

	register long r7 __asm__("r7") = __NR_munmap;
	register long r0 __asm__("r0") = (long)addr;
	register long r1 __asm__("r1") = (long)length;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1)
		: "memory");

	return r0;
}


/* 99 */
int statfs(const char *path, struct statfs *buf) {

	register long r7 __asm__("r7") = __NR_statfs;
	register long r0 __asm__("r0") = (long)path;
	register long r1 __asm__("r1") = (long)buf;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1)
		: "memory");

	return r0;
}

/* 140 */
/* AKA _llseek */
int32_t lseek64(int32_t fd, uint32_t offset_high,
		uint32_t offset_low, uint64_t *result,
		uint32_t whence) {

	register long r7 __asm__("r7") = __NR__llseek;
	register long r0 __asm__("r0") = (long)fd;
	register long r1 __asm__("r1") = (long)offset_high;
	register long r2 __asm__("r2") = (long)offset_low;
	register long r3 __asm__("r3") = (long)result;
	register long r4 __asm__("r4") = (long)whence;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4)
		: "memory");

	return r0;

}

/* 193 */

/* note, 64-bit values need to be aligned in even registers like this?*/
/* assuming we are using something similar to EABI here */
int32_t truncate(const char *path, int64_t length) {

	register long r7 __asm__("r7") = __NR_truncate64;
	register long r0 __asm__("r0") = (long)path;
	register long r1 __asm__("r1") = 0;
	register long r2 __asm__("r2") = length&0xffffffff;
	register long r3 __asm__("r3") = length>>32;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2), "r"(r3)
		: "memory");

	return update_errno(r0);
}

/* 194 */

/* note, 64-bit values need to be aligned in even registers like this?*/
/* assuming we are using something similar to EABI here */
int32_t ftruncate(int32_t fd, int64_t length) {

	register long r7 __asm__("r7") = __NR_ftruncate64;
	register long r0 __asm__("r0") = fd;
	register long r1 __asm__("r1") = 0;
	register long r2 __asm__("r2") = length&0xffffffff;
	register long r3 __asm__("r3") = length>>32;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2), "r"(r3)
		: "memory");

	return update_errno(r0);
}


/* 345 */
int getcpu(uint32_t *cpu, uint32_t *node, void *tcache) {

	register long r7 __asm__("r7") = __NR_getcpu;
	register long r0 __asm__("r0") = (long)cpu;
	register long r1 __asm__("r1") = (long)node;
	register long r2 __asm__("r2") = (long)tcache;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return r0;
}

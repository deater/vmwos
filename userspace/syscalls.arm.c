#include <stddef.h>
#include <stdint.h>

#include "syscalls.h"

#define __NR_read	3
#define __NR_write	4
#define __NR_time	13
#define __NR_getpid	20
#define __NR_ioctl	54
#define __NR_reboot	88
#define __NR_nanosleep  162

uint32_t read(int fd, void *buf, size_t count) {

	register long r7 __asm__("r7") = __NR_read;
	register long r0 __asm__("r0") = fd;
	register long r1 __asm__("r1") = (long)buf;
	register long r2 __asm__("r2") = count;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return r0;
}


uint32_t write(int fd, const void *buf, uint32_t size) {

	register long r7 __asm__("r7") = __NR_write;
	register long r0 __asm__("r0") = fd;
	register long r1 __asm__("r1") = (long)buf;
	register long r2 __asm__("r2") = size;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return r0;
}


int getpid(void) {

	register long r7 __asm__("r7") = __NR_getpid;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7) /* input */
		: "memory");

	return r0;

}


int nanosleep(const struct timespec *req, struct timespec *rem) {

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

int ioctl3(int fd, unsigned long request, unsigned long req2) {

	register long r7 __asm__("r7") = __NR_ioctl;
	register long r0 __asm__("r0") = fd;
	register long r1 __asm__("r1") = request;
	register long r2 __asm__("r2") = req2;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return r0;
}

int ioctl4(int fd, unsigned long request, unsigned long req2, unsigned long req3) {

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


int sys_time(void) {

	register long r7 __asm__("r7") = __NR_time;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7) /* input */
		: "memory");

	return r0;

}


int sys_reboot(void) {

	register long r7 __asm__("r7") = __NR_reboot;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7) /* input */
		: "memory");

	return r0;

}

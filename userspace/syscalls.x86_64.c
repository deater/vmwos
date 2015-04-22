#include <stddef.h>
#include <stdint.h>

#include "syscalls.h"

#define __NR_read	0
#define __NR_write	1
#define __NR_ioctl	16
#define __NR_nanosleep  35

uint32_t read(int fd, void *buf, size_t count) {

	uint32_t ret;
	asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(__NR_read), "D"(fd), "S"(buf), "d"(count)
			: "cc", "rcx", "r11", "memory"
	);
	return ret;
}


uint32_t write(int fd, const void *buf, uint32_t size) {

	uint32_t ret;
	asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(__NR_write), "D"(fd), "S"(buf), "d"(size)
			: "cc", "rcx", "r11", "memory"
	);
	return ret;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {

	uint32_t ret;
	asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(__NR_nanosleep), "D"(req), "S"(rem)
			: "cc", "rcx", "r11", "memory"
	);
	return ret;

}

int ioctl3(int fd, unsigned long request, unsigned long req2) {

	uint32_t ret;
	asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(__NR_ioctl), "D"(fd), "S"(request), "d"(req2)
			: "cc", "rcx", "r11", "memory"
	);

	return ret;
}

int ioctl4(int fd, unsigned long request, unsigned long req2, unsigned long req3) {

	uint32_t ret;
	register long r10 asm("r10") = req3;

	asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(__NR_ioctl), "D"(fd), "S"(request), "d"(req2), "r"(r10)
			: "cc", "rcx", "r11", "memory"
	);

	return ret;
}

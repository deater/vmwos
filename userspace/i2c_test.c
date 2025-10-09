#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <errno.h>
#include <string.h>
#endif

uint32_t vmwos_i2c_test(void) {

	register long r7 __asm__("r7") = __NR_i2c_test;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7) /* input */
		: "memory");

	return r0;
}


int main(int argc, char **argv) {

	printf("Testing i2c\n");


	vmwos_i2c_test();

	return 0;
}

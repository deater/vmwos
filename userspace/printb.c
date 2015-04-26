#include <stddef.h>
#include <stdint.h>
#include "syscalls.h"
#include "vlibc.h"

int main(int argc, char **argv) {

	while(1) {
		printf("B");
		asm volatile(
			"mov r1,#65536\n"
			"a_loop:\n"
			"subs	r1,r1,#1\n"
			"bne	a_loop\n"
			:::);
	}
}

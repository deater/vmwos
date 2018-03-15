/* Prints B for 10s */
/* Half of the traditional duo used to show multi-tasking is working */

#include <stddef.h>
#include <stdint.h>
#include "syscalls.h"
#include "vlibc.h"

#define RUNTIME	10

int main(int argc, char **argv) {

	int start_time,current_time;
	struct tms buf;

	start_time=time(NULL);

	while(1) {
		printf("B");
		asm volatile(
			"mov r1,#65536\n"
			"a_loop:\n"
			"subs	r1,r1,#1\n"
			"bne	a_loop\n"
			:::);
		current_time=time(NULL);
		if (current_time-start_time>RUNTIME) break;
	}

	times(&buf);

	printf("\nTime running B: "
		"Wallclock: %d seconds, User: %d jiffies = %d seconds\n",
		current_time-start_time,buf.tms_utime,buf.tms_utime/64);

	return 0;
}

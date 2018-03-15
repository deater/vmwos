/* Prints A for 5s */
/* Half of the traditional duo used to show multi-tasking is working */

#include <stddef.h>
#include <stdint.h>
#include "syscalls.h"
#include "vlibc.h"

#define RUNTIME	5

int main(int argc, char **argv) {

	int start_time,current_time;
	struct tms buf;

	start_time=time(NULL);

	while(1) {
		printf("A");
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

	printf("\nTime running A: "
		"Wallclock: %d seconds, User: %d seconds, Running %d%% of time\n",
		current_time-start_time,buf.tms_utime/64,
		(buf.tms_utime/64)*100U/(current_time-start_time));

	return 0;
}

/* Prints A for 5s */
/* Half of the traditional duo used to show multi-tasking is working */

#include <stddef.h>
#include <stdint.h>
#include "syscalls.h"
#include "vlibc.h"

/* In microseconds */
#define RUNTIME	5*1000000


static void print_fixed(uint32_t value, int divider) {

	int i=0,f=0;

	if (divider==1000000) {
		i=value/1000000;
		f=(value%1000000)/1000;
		printf("%d.%03d",i,f);
	}
	else if (divider==1000) {
		i=value/1000;
		f=value%1000;
		printf("%d.%03d",i,f);
	}
	else if (divider==64) {
		i=value/64;
		f=((value%64)*1000)/64;
		printf("%d.%03d",i,f);
	}
	else {

	}

}

int main(int argc, char **argv) {

	int64_t start_time,current_time;
	struct tms buf;
	struct timespec t;

	clock_gettime(CLOCK_REALTIME,&t);
	start_time=(t.tv_sec*1000000ULL)+t.tv_nsec/1000;

	while(1) {
		printf("A");

		asm volatile(
			"mov r1,#6553600\n"
			"a_loop:\n"
			"subs	r1,r1,#1\n"
			"bne	a_loop\n"
			:::);
		clock_gettime(CLOCK_REALTIME,&t);
		current_time=(t.tv_sec*1000000ULL)+t.tv_nsec/1000;
		if (current_time-start_time>RUNTIME) break;
	}

	times(&buf);

	int wallclock_ms,user_ms,percent=0;
	uint32_t cpu;

	getcpu(&cpu,NULL,NULL);

	printf("\nTime running A: CPU: %d, Wallclock: ",cpu);
	print_fixed((current_time-start_time),1000000);
	printf(" seconds, User: ");
	print_fixed(buf.tms_utime,64);
	printf(" seconds, Running ");

	wallclock_ms=(current_time-start_time);
	wallclock_ms/=1000;
	user_ms=(buf.tms_utime*1000)/64;

	if (wallclock_ms!=0) {
		percent=(100*user_ms)/wallclock_ms;
	}

	printf("%d%% of time\n",percent);

	return 0;
}

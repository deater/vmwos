#include <stdio.h>
#include <string.h>
#include <sys/time.h>


#define MAX_SIZE (16*1024*1024) // 16MB, 2^24
char mem_buffer[MAX_SIZE];

#define ITERATIONS	128

void print_size(double size) {

	if (size<1024) {
		printf("%.1lf bytes",size);
	}
	else if (size<1024*1024) {
		printf("%.1lf kB",size/1024.0);
	}
	else if (size<1024*1024*1024) {
		printf("%.1lf MB",size/(1024.0*1024.0));
	}
	else {
		printf("%.1lf GB",size/(1024.0*1024.0*1024.0));
	}

}

int main(int argc, char **argv) {

	int i,j;
	long long size;
	struct timeval before,after;
	double seconds,total_size;

	for(i=0;i<=24;i++) {

		size=1<<i;

		gettimeofday(&before,NULL);

		for(j=0;j<ITERATIONS;j++) {
			memset(mem_buffer,0,size);
		}

		gettimeofday(&after,NULL);

		seconds=(after.tv_sec-before.tv_sec);
		seconds+=(after.tv_usec-before.tv_usec)/1000000.0;

		total_size=size*ITERATIONS;
#if 0
		print_size(size);
		printf(" * %d = ",ITERATIONS);

		print_size(total_size);

		printf("in %lfs = %lf MB/s\n",seconds, 
			total_size/1024.0/1024.0/seconds);
#endif


		printf("%lld\t%lf\t(* ",size,total_size/1024.0/1024.0/seconds);
		print_size(size);
		printf(", %d iterations, %lfs *)\n",ITERATIONS,seconds);

	}

	return 0;
}

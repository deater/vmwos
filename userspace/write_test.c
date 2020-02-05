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
#include <fcntl.h>
#include <unistd.h>
#endif

#define BUF_SIZE 128

//static int debug=1;

int main(int argc, char **argv) {

	int output_fd,result;

	printf("argc=%d, argv[0]=%s\n",argc,argv[0]);

	if (argc<2) {
		printf("Error: need file name\n");
		return -1;
	}

	output_fd=open(argv[1],O_WRONLY,0);
	if (output_fd<0) {
		printf("Couldn't open %s: %s\n",argv[1],strerror(errno));
		return 0;
	}

	result=write(output_fd,"BLAH BLAH\n",10);
	printf("Write result=%d %s\n",result,strerror(errno));

	result=close(output_fd);
	printf("Close result=%d\n",result);

	return 0;
}

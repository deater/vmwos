/* cp -- copy a file */

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

#define BUF_SIZE 256

static int debug=0;

static int cp(int in_fd, int out_fd) {

	char buffer[BUF_SIZE];
	int result;
	int64_t pos;

	while(1) {
		result=read(in_fd,buffer,BUF_SIZE);
		if (result<=0) {
			if (debug) {
				printf("\nFinish with result=%d\n",result);
				pos=lseek(in_fd,0,SEEK_CUR);
				printf("\nFinish file pos=%lld\n",pos);
			}
			break;
		}
		result=write(out_fd,buffer,result);
		if (result<=0) {
			printf("cp: Unexpected write error %d\n",result);
			break;
		}
	}

	return 0;
}


int main(int argc, char **argv) {

	int input_fd;
	int output_fd;	/* stdout */

	if (argc<3) {
		printf("\nUSAGE: cp src dest\n\n");
	}
	else {
		input_fd=open(argv[1],O_RDONLY,0);
		if (input_fd<0) {
			printf("Error opening input file %s (%d)\n",
				argv[1],input_fd);
			return -1;
		}

		output_fd=open(argv[2],O_WRONLY|O_CREAT,0660);
		if (output_fd<0) {
			printf("Error opening output file %s (%d)\n",
				argv[2],output_fd);
			return -1;
		}

		cp(input_fd,output_fd);
		close(input_fd);
		close(output_fd);
	}

	return 0;
}

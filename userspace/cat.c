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

static int debug=0;

static int cat(int in_fd, int out_fd) {

	char buffer[BUF_SIZE];
	int result;
	int64_t pos;

	while(1) {
		result=read(in_fd,buffer,BUF_SIZE);
		if (result<=0) {
			if (debug) {
				printf("\nFinish with result=%d\n",result);
				pos=lseek(in_fd,0,SEEK_CUR);
				printf("Finish file pos=%lld\n",pos);
			}
			break;
		}
		result=write(out_fd,buffer,result);
		if (result<=0) {
			printf("cat: Unexpected write error %d\n",result);
			break;
		}
	}

	return 0;
}


int main(int argc, char **argv) {

	int i;

	int input_fd;
	int output_fd=1;	/* stdout */

	if (argc<2) {
		input_fd=0;	/* stdin */
		cat(input_fd,output_fd);
	}
	else {
		for(i=1;i<argc;i++) {
			input_fd=open(argv[i],O_RDONLY,0);
			if (input_fd<0) {
				printf("Error opening %s\n",argv[i]);
				return -1;
			}
			cat(input_fd,output_fd);
			close(input_fd);
		}
	}

	return 0;
}

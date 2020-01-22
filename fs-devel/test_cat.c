#include <stddef.h>
#include <stdint.h>

#include "test_glue.h"

#include <stdio.h>

#if 0
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define BUF_SIZE 128

static int debug=1;

static int cat(int in_fd, int out_fd) {

	char buffer[BUF_SIZE];
	int result;

	while(1) {
		result=read_syscall(in_fd,buffer,BUF_SIZE);
		if (result<=0) break;
		result=write_syscall(out_fd,buffer,result);
		if (result<=0) break;
	}

	return 0;
}


int main(int argc, char **argv) {

	int i;

	int input_fd;
	int output_fd=1;	/* stdout */

	test_glue_setup();

	if (argc<2) {
		input_fd=0;	/* stdin */
		cat(input_fd,output_fd);
	}
	else {
		for(i=1;i<argc;i++) {
			input_fd=open_syscall(argv[i],O_RDONLY,0);
			if (debug) printf("Opened with fd %d\n",input_fd);
			if (input_fd<0) {
				printf("Error opening %s\n",argv[i]);
				return -1;
			}
			cat(input_fd,output_fd);
			close_syscall(input_fd);
		}
	}

	return 0;
}

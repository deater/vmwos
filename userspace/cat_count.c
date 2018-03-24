/* Prints only X chars of a file */
/* Used when debugging loser ANSI animation problems */

#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define BUF_SIZE 8192

static char buffer[BUF_SIZE];

#if 0
int32_t atoiX(char *string) {

        int result=0;
	char *ptr;

	ptr=string;

	while(*ptr!=0) {
		result*=10;
		result+=(*ptr)-'0';
		ptr++;
	}

        return result;
}
#endif



int main(int argc, char **argv) {

	int input_fd;
	int output_fd=1;	/* stdout */
	int count;
	int result;

	if (argc!=3) {
		printf("Usage:\n\t%s FILENAME count\n\n",argv[0]);
		return -1;
	}

	count=atoi(argv[2]);
	printf("Only catting %d bytes\n",count);

	input_fd=open(argv[1],O_RDONLY,0);
	if (input_fd<0) {
		printf("Error opening %s\n",argv[1]);
		return -1;
	}

//	while(1) {
		result=read(input_fd,buffer,count);
		if (result>0) {
			result=write(output_fd,buffer,result);
		}
//	}

	close(input_fd);

	return 0;
}

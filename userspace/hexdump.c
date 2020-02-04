/* Simple hexdump program */

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

#define BUF_SIZE 16

static int hexdump(int in_fd) {

	char buffer[BUF_SIZE];
	int result,i;
	int32_t pos=0;

	while(1) {
		result=read(in_fd,buffer,BUF_SIZE);

		if (result<0) {
			printf("Error reading\n");
			break;
		}

		if (result==0) {
			printf("\n");
			printf("Total size=%d\n",pos);
			break;
		}

		for(i=0;i<BUF_SIZE;i++) {
			if (i%16==0) printf("%08x: ",pos+i);
			if (i%16==8) printf(" ");
			if (i<result) printf("%02x ",buffer[i]&0xff);
			else printf("   ");
		}

		printf("  |");
		for(i=0;i<BUF_SIZE;i++) {

			if (i>=result) break;

			if ((buffer[i]>=' ') && (buffer[i]<127)) {
				printf("%c",buffer[i]);
			}
			else {
				printf(".");
			}
		}
		printf("|\n");

		pos+=result;
	}

	return 0;
}


int main(int argc, char **argv) {

	int input_fd;

	if (argc<2) {
		printf("Usage: hexdump input\n\n");
		return -1;
	}

	input_fd=open(argv[1],O_RDONLY,0);
	if (input_fd<0) {
		printf("Error opening %s\n",argv[1]);
		return -1;
	}
	hexdump(input_fd);
	close(input_fd);

	return 0;
}

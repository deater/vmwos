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

static int hexdump(int in_fd, int bytes) {

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

		if ((bytes>0) && (pos>=bytes)) break;
	}

	return 0;
}

static void usage(void) {
	printf("\nUsage: hexdump [-n size] input\n\n");
	exit(1);
}

int main(int argc, char **argv) {

	int input_fd;
	int c;
	int bytes=-1;

	opterr=0;
	while ((c=getopt(argc,argv,"hn:"))!=-1) {
		switch(c) {
			case 'n':
				bytes=atoi(optarg);
				break;
			case 'h':
                        default:
				usage();
				break;
		}
	}

	if (optind>=argc) {
		usage();
		return -1;
	}

	input_fd=open(argv[optind],O_RDONLY,0);
	if (input_fd<0) {
		printf("Error opening %s\n",argv[optind]);
		return -1;
	}
	hexdump(input_fd,bytes);
	close(input_fd);

	return 0;
}

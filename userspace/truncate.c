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
#include <stdlib.h>
#include <getopt.h>
#endif

//static int debug=1;

static void usage(void) {
	printf("\nUsage: truncate [-s size] FILENAME\n\n");
	exit(1);
}

int main(int argc, char **argv) {

	int c,size=0,result;
	int use_ftruncate=0,fd;

	opterr=0;
	while ((c=getopt(argc,argv,"fhs:"))!=-1) {
		switch(c) {
			case 'f':
				use_ftruncate=1;
				break;
			case 's':
				size=atoi(optarg);
				break;
			case 'h':
			default:
				usage();
				break;
		}
	}

	if (optind<argc) {

		if (use_ftruncate) {
			printf("Ftruncating %s to size %d\n",argv[optind],size);

			fd=open(argv[optind],O_WRONLY,0);
			if (fd<0) {
				printf("Error opening: %s\n",strerror(errno));
			}
			else {
				result=ftruncate(fd,size);
				if (result<0) {
					printf("Error truncating: %d %s\n",
						errno,strerror(errno));
				}
				close(fd);
			}
		}
		else {
			printf("Truncating %s to size %d\n",argv[optind],size);

			result=truncate(argv[optind],size);
			if (result<0) {
				printf("Error truncating: %d %s\n",
					errno,strerror(errno));
			}
		}
	}
	else {
		usage();
	}
	return 0;
}

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

	opterr=0;
	while ((c=getopt(argc,argv,"hs:"))!=-1) {
		switch(c) {
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
		printf("Truncating %s to size %d\n",argv[optind],size);

		result=truncate(argv[optind],size);
		if (result<0) {
			printf("Error truncating: %s\n",strerror(errno));
		}
	}
	else {
		usage();
	}
	return 0;
}

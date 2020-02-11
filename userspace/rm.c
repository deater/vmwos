/* rm -- remove file */

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
	printf("\nUsage: rm FILENAME ...\n\n");
	exit(1);
}

int main(int argc, char **argv) {

	int result;
	int c;

	opterr=0;
	while ((c=getopt(argc,argv,"h"))!=-1) {
		switch(c) {
			case 'h':
			default:
				usage();
				break;
		}
	}

	while (optind<argc) {

		printf("Attempting to unlink %s\n",argv[optind]);

		result=unlink(argv[optind]);
		if (result<0) {
			printf("Error unlinking %s: %s\n",
					argv[optind],strerror(errno));
			break;
		}
		optind++;
	}

	return 0;
}

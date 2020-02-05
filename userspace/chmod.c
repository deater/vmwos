/* chmod -- change mode of file */

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

static int debug=1;

int32_t parse_mode(char *mode) {

	int32_t result=0,i;

	/* urgh octal */
	if (mode[0]=='0') {
		for(i=0;i<strlen(mode);i++) {
			result*=8;
			result+=mode[i]-'0';
		}
	}
	else {
		

	}

	if (debug) printf("Parsed %s as %x\n",mode,result);

	return result;
}

int main(int argc, char **argv) {

	int32_t mode=0;

	if (argc<3) {
		printf("\nUsage: chmod mode filename\n\n");
		return -1;
	}

	mode=parse_mode(argv[1]);

	chmod(argv[2],mode);

	return 0;
}

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

int32_t parse_mode(char *mode,int32_t old_mode) {

	int32_t result=0,i,octal=0;

	if (debug) printf("Old mode: %x\n",old_mode);

	/* urgh octal */
	if (mode[0]=='0') {
		for(i=0;i<strlen(mode);i++) {
			octal*=8;
			octal+=mode[i]-'0';
		}
		result=(old_mode&0xfffffe00)|(octal&0x1ff);
	}
	else if (mode[0]=='-') {
		if (mode[1]=='r') {
			result=old_mode&=~0444;
		}
		else if (mode[1]=='w') {
			result=old_mode&=~0222;
		}
		else if (mode[1]=='x') {
			result=old_mode&=~0111;
		}
		else {
			printf("Unsupported mode %c\n",mode[1]);
			exit(-1);
		}
	}
	else if (mode[0]=='+') {
		if (mode[1]=='r') {
			result=old_mode|=0444;
		}
		else if (mode[1]=='w') {
			result=old_mode|=0222;
		}
		else if (mode[1]=='x') {
			result=old_mode|=0111;
		}
		else {
			printf("Unsupported mode %c\n",mode[1]);
			exit(-1);
		}
	}
	else {
		printf("Unknown mode %s\n",mode);
		exit(-1);
	}

	if (debug) printf("New mode %x\n",result);

	return result;
}

int main(int argc, char **argv) {

	int32_t new_mode=0,result,old_mode;
	struct stat stat_buf;

	if (argc<3) {
		printf("\nUsage: chmod mode filename\n\n");
		return -1;
	}

	result=stat(argv[2],&stat_buf);
	if (result<0) {
		printf("Error stating %s: %s\n",argv[2],strerror(errno));
	}

	old_mode=stat_buf.st_mode;

	new_mode=parse_mode(argv[1],old_mode);

	chmod(argv[2],new_mode);

	return 0;
}

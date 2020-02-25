/* dmesg -- dump the kernel log */

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

#define BUF_SIZE 4096

static char dmesg_buffer[BUF_SIZE+1];

//static int debug=0;

int main(int argc, char **argv) {

	int result;
	int size;

	size=dmesg(SYSLOG_ACTION_SIZE_BUFFER,NULL);
	if (size!=BUF_SIZE) {
		printf("Warning, buffer not big enough %d > %d\n",
			size,BUF_SIZE);
		return -1;
	}

	result=dmesg(SYSLOG_ACTION_READ_ALL,dmesg_buffer);
	if (result<0) {
		printf("Error reading from kernel\n");
		return -1;
	}

	write(STDOUT_FILENO,dmesg_buffer,result);

	printf("\n");

	return 0;
}

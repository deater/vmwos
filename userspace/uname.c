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
#include <sys/utsname.h>
#endif

int main(int argc, char **argv) {

	struct utsname buf;

	uname(&buf);

	printf("%s %s %s %s\n",
		buf.sysname,
		buf.nodename,
		buf.release,
		buf.machine);

	return 0;
}

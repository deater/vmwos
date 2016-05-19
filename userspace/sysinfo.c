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
#include <sys/sysinfo.h>
#endif

int main(int argc, char **argv) {

	struct sysinfo buf;
	struct utsname ubuf;
	char timestring[256];

	sysinfo(&buf);
        uname(&ubuf);

	printf("\n");

	printf("%s version %s system info:\n",ubuf.sysname,ubuf.release);

	printf("\t%d/%d Processes Ready\n",
		buf.procs_ready,buf.procs);
	printf("\t%dk/%dk RAM free\n",
		buf.free_ram/1024,buf.total_ram/1024);
	printf("\tUp %s, ",
		time_pretty(buf.uptime,timestring,256));
	printf("Idle for %s\n",
		time_pretty(buf.idle_time,timestring,256));

	return 0;
}

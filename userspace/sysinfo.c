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

	sysinfo(&buf);

	printf("%d/%d Processes Ready\n",
		buf.procs_ready,buf.procs);
	printf("%dk/%dk RAM free\n",
		buf.free_ram/1024,buf.total_ram/1024);
	printf("Up %ds, Idle for %ds\n",
		buf.uptime,buf.idle_time);

	return 0;
}

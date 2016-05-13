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

	struct sysinfo_t buf;

	get_sysinfo(&buf);

	printf("%d/%d Processes Ready\n",
		buf.processes_ready,buf.total_processes);
	printf("%d/%d RAM free\n",
		buf.free_ram,buf.total_ram);
	printf("Up %ds, Idle for %ds\n",
		buf.uptime,buf.idletime);

	return 0;
}

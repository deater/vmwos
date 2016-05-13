#include <stdint.h>

#include "sysinfo.h"


int32_t get_sysinfo(struct sysinfo_t *buf) {

	buf->total_processes=0;
	buf->processes_ready=0;
	buf->total_ram=0;
	buf->free_ram=0;
	buf->uptime=0;
	buf->idletime=0;

	return 0;
}

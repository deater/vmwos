#include <stdint.h>

#include "sysinfo.h"
#include "time.h"
#include "memory.h"
#include "lib/memset.h"

int32_t sysinfo(struct sysinfo *buf) {

	memset(buf,0,sizeof(struct sysinfo));

	buf->uptime=ticks_since_boot()/TIMER_HZ;
	/* no loadavg info */
	buf->total_ram=memory_total;
	buf->free_ram=memory_total_free();
	/* no shared/buffer/swap */
	buf->procs=0;
	buf->procs_ready=0;
	buf->idle_time=0;

	return 0;
}

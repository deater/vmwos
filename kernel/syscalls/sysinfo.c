#include <stdint.h>

#include "lib/memset.h"
#include "lib/smp.h"

#include "syscalls/sysinfo.h"
#include "time/time.h"
#include "memory/memory.h"
#include "processes/process.h"

int32_t sysinfo(struct sysinfo *buf) {

	int32_t total_proc_count,ready_proc_count;

	memset(buf,0,sizeof(struct sysinfo));

	/* Seconds since boot */
	buf->uptime=ticks_since_boot()/TIMER_HZ;

	/* no loadavg info */

	/* Total RAM */
	buf->total_ram=memory_get_total();

	/* Free RAM */
	buf->free_ram=memory_total_free();

	/* no shared/buffer/swap */

	/* On Linux procs is only 16-bits.  On little endian */
	/* shouldn't matter */
	total_proc_count=process_get_totals(
			PROCESS_STATUS_READY,&ready_proc_count);
	buf->procs=total_proc_count;

	/* VMWos specific fields */

	/* Ready processes */
	buf->procs_ready=ready_proc_count;

	/* Idle time */
	buf->idle_time=(proc_first->user_time)/TIMER_HZ;

	return 0;
}

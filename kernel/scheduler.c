#include <stddef.h>
#include <stdint.h>

#include "exec.h"
#include "memory.h"
#include "scheduler.h"
#include "process.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"


void schedule(long *pcb) {

	int i;

	if (!userspace_started) return;

	i=current_process;

	/* save current process state */

	process_save(i,pcb);

	/* find next available process */
	/* Should we have an idle process (process 0) */
	/* That is special cased and just runs wfi?   */

	while(1) {
		i++;
		if (i>=MAX_PROCESSES) i=0;
		if ((process[i].valid) &&
			(process[i].status=PROCESS_STATUS_READY)) break;
	}

	/* switch to new process */

	/* ARM documentation says we can put a */
	/* clrex instruction here */
	/* to avoid false positives in the mutex handling? */
	/* if there's any aliasing between new and old process? */

	/* IRQ stack */
	process_run(i,pcb[17]);

}

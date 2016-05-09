#include <stddef.h>
#include <stdint.h>

#include "exec.h"
#include "memory.h"
#include "scheduler.h"
#include "process.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"


void schedule(long *irq_stack) {

	int i;

	if (!userspace_started) return;

	i=current_process;

	/* save current process state */

	process_save(i,irq_stack);

	/* find next available process */
	/* Should we have an idle process (process 0) */
	/* That is special cased and just runs wfi?   */

	while(1) {
		i++;

		/* wrap around if off end */
		if (i>=MAX_PROCESSES) i=1;

		/* if valid and ready, then run it */
		if ((process[i].valid) &&
			(process[i].status==PROCESS_STATUS_READY)) break;

		/* Nothing was ready, run idle task */
		if (i==current_process) {
			i=0;
			break;
		}

	}

	/* switch to new process */
	if (i!=current_process) {
		printk("Switching from %d to %d\n",current_process,i);
		process_run(i,irq_stack);
	}

	/* ARM documentation says we can put a */
	/* clrex instruction here */
	/* to avoid false positives in the mutex handling? */
	/* if there's any aliasing between new and old process? */

}

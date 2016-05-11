#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"

#include "process.h"

int32_t waitpid(int32_t pid, int32_t *wstatus, int32_t options) {

	struct process_control_block_type *proc;

	proc=process_lookup(pid);

	if (proc->status!=PROCESS_STATUS_EXITED) {
		printk("Waiting on pid %d\n",pid);
//		process[current_process].status=PROCESS_STATUS_SLEEPING;
	}

	if (wstatus!=NULL) *wstatus=proc->exit_value;

	/* Kill the zombie now? */

	/* Reschedule? */
	return pid;
}



#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"

#include "processes/process.h"
#include "processes/scheduler.h"
#include "processes/wait.h"

static int debug=1;

struct wait_queue_t waitpid_wait_queue = {
	NULL
};


int32_t waitpid_done(void) {
	/* Wake anyone waiting for exit */
	if (debug) printk("WAITPID: waking all waiters\n");

	wait_queue_wake(&waitpid_wait_queue);

	return 0;
}

int32_t waitpid(int32_t pid, int32_t *wstatus, int32_t options) {

	struct process_control_block_type *proc;

	proc=process_lookup(pid);

	if (debug) printk("WAITPID: Waiting on pid %d\n",pid);

	while (proc->status!=PROCESS_STATUS_EXITED) {
		wait_queue_add(&waitpid_wait_queue,current_process);
		//schedule();
	}

	if (wstatus!=NULL) *wstatus=proc->exit_value;

	/* Kill the zombie now? */

	/* Reschedule? */
	return pid;
}



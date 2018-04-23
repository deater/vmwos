#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/smp.h"

#include "processes/process.h"
#include "processes/scheduler.h"
#include "processes/waitpid.h"

static int debug=0;

void exit(int32_t status) {

	if (debug) printk("Process %d exiting\n",current_proc[0]->pid);

//	dump_saved_user_state(current_proc[0];
//	dump_saved_kernel_state(current_proc[0]);

        current_proc[0]->status=PROCESS_STATUS_EXITED;
	current_proc[0]->exit_value=status;

	/* Free resources now? No wait until waitpid() */
	/* process_destroy(current_proc[0]); */

	/* Wake anyone waiting in waitpid */
	waitpid_done();

	/* Reschedule? */
	schedule();
}



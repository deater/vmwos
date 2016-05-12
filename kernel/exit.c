#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"

#include "process.h"

void exit(int32_t status) {

	printk("Process %d exiting\n",current_process->pid);

//	dump_saved_user_state(current_process);
//	dump_saved_kernel_state(current_process);

        current_process->status=PROCESS_STATUS_EXITED;
	current_process->exit_value=status;

	/* Free resources now? */
	/* process_destroy(current_process); */

	/* Reschedule? */
	schedule();
}



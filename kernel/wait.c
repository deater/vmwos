#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"

#include "process.h"

int32_t waitpid(int32_t pid, int32_t *wstatus, int32_t options) {

	if (wstatus!=NULL) *wstatus=process[current_process].exit_value;

	/* Kill the zombie now? */

	/* Reschedule? */
	return pid;
}



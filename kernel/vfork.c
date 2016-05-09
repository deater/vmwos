#include <stddef.h>
#include <stdint.h>

#include "process.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"


int32_t vfork(void) {

	int32_t child,parent,i;

	parent=current_process;

	/* create new child process */
	child=process_create();
	if (child<0) {
		return child;
	}

	printk("vfork: created child %d\n",child);


	printk("vfork: copying register state from parent (%d -> %d)\n",
		parent,child);
	/* copy register state from parent */
	for(i=0;i<15;i++) {
		process[child].reg_state.r[i]=process[parent].reg_state.r[i];
	}
	/* Set r0 to be 0 so it looks like our syscall returned 0 */
	process[child].reg_state.r[0]=0;

	/* set stack/data to NULL */
	/* otherwise we might try to free parent's on exit() */
	process[child].stack=NULL;
	process[child].text=NULL;

	printk("vfork: put parent %d to sleep\n",parent);
	/* put parent to sleep */
	process[parent].status=PROCESS_STATUS_SLEEPING;

	printk("vfork: wake child %d\n",child);
	process[child].status=PROCESS_STATUS_READY;
	process[child].running=1;

	/* call the scheduler ???*/

	return child;
}

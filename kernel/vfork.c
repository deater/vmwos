#include <stddef.h>
#include <stdint.h>

#include "process.h"
#include "scheduler.h"

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

	printk("vfork: synching current parent state\n");

	/* Make sure on return the parent gets the syscall result */
	((long *)swi_handler_stack)[2]=child;

	process_save(parent,(long *)swi_handler_stack);

	printk("vfork: copying register state from parent (%d -> %d)\n",
		parent,child);
	/* copy register state from parent */
	for(i=0;i<15;i++) {
		process[child].reg_state.r[i]=process[parent].reg_state.r[i];
	}
	process[child].reg_state.spsr=process[parent].reg_state.spsr;
	process[child].reg_state.lr=process[parent].reg_state.lr;

	process[child].parent=parent;

	/* set stack/data to NULL */
	/* otherwise we might try to free parent's on exit() */
	process[child].stack=NULL;
	process[child].text=NULL;

	printk("vfork: put parent %d to sleep\n",parent);
	/* put parent to sleep */
	process[parent].status=PROCESS_STATUS_SLEEPING;

	printk("vfork: wake child %d\n",child);
	process[child].status=PROCESS_STATUS_READY;

	return child;
}

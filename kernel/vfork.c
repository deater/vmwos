#include <stddef.h>
#include <stdint.h>

#include "process.h"
#include "scheduler.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"


int32_t vfork(void) {

	int32_t i;
	struct process_control_block_type *child,*parent;

	parent=current_process;

	/* create new child process */
	child=process_create();
	if (child==NULL) {
		printk("kernel: error forking\n");
		return -1;
	}

	printk("vfork: created child %d\n",child->pid);

	printk("vfork: synching current parent state\n");

	/* Make sure on return the parent gets the syscall result */
	((long *)swi_handler_stack)[2]=child->pid;

	process_save(parent,(long *)swi_handler_stack);

	printk("vfork: copying register state from parent (%d -> %d)\n",
		parent->pid,child->pid);
	/* copy register state from parent */
	for(i=0;i<15;i++) {
		child->reg_state.r[i]=parent->reg_state.r[i];
	}
	child->reg_state.spsr=parent->reg_state.spsr;
	child->reg_state.lr=parent->reg_state.lr;

	child->parent=parent;

	/* set stack/data to NULL */
	/* otherwise we might try to free parent's on exit() */
	child->stack=NULL;
	child->text=NULL;

	printk("vfork: put parent %d to sleep\n",parent->pid);
	/* put parent to sleep */
	parent->status=PROCESS_STATUS_SLEEPING;

	printk("vfork: wake child %d\n",child->pid);
	child->status=PROCESS_STATUS_READY;

	return child->pid;
}

#include <stddef.h>
#include <stdint.h>

#include "process.h"
#include "scheduler.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"


int32_t vfork(void) {

	int32_t child_pid;
	struct process_control_block_type *child,*parent,*prev,*next;
	uint32_t new_stack;

	parent=current_process;

	/* create new child process */
	child=process_create();
	if (child==NULL) {
		printk("kernel: error forking\n");
		return -1;
	}

	/* Some state we don't want to duplicate */
	child_pid=child->pid;
	prev=child->prev;
	next=child->next;

	/* Copy process info over, including kernel stack */
	memcpy(child,parent,sizeof(struct process_control_block_type));

	child->pid=child_pid;
	child->prev=prev;
	child->next=next;

	printk("vfork: created child %d\n",child_pid);

	printk("vfork: synching current parent state\n");

	/* Make sure on return the parent gets the syscall result */
	//((long *)swi_handler_stack)[2]=child->pid;

//	process_save(parent);

//	printk("vfork: copying register state from parent (%d -> %d)\n",
//		parent->pid,child_pid);
	/* copy register state from parent */
//	for(i=0;i<15;i++) {
//		child->reg_state.r[i]=parent->reg_state.r[i];
//	}
//	child->reg_state.spsr=parent->reg_state.spsr;
	//child->reg_state.lr=parent->reg_state.lr;

	child->parent=parent;

	/* set stack/data to NULL */
	/* otherwise we might try to free parent's on exit() */
	child->stack=NULL;
	child->text=NULL;

	printk("vfork: put parent %d to sleep\n",parent->pid);
	/* put parent to sleep */
	parent->status=PROCESS_STATUS_SLEEPING;

	printk("vfork: wake child %d\n",child_pid);
	child->status=PROCESS_STATUS_READY;

	/* Update saved process state of child */
	/* It makes a copy of parent, but with SP pointing to child kernel stack */
	{
		register long r13 asm ("r13");
		long old_stack_offset;

		old_stack_offset=r13-(long)parent;
		new_stack=(long)child+old_stack_offset;
	}

	process_save(child,new_stack);


	if (current_process==parent) {
		schedule();
		return child_pid;
	}

	/* In child, return 0 */

	return 0;
}

#include <stddef.h>
#include <stdint.h>

#include "processes/process.h"
#include "processes/scheduler.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"

static int debug=0;

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

	if (debug) printk("vfork: created child %d\n",child_pid);

	child->parent=parent;

	/* set stack/data to NULL */
	/* otherwise we might try to free parent's on exit() */
	child->stack=NULL;
	child->text=NULL;

	if (debug) printk("vfork: put parent %d to sleep\n",parent->pid);
	/* put parent to sleep */
	parent->status=PROCESS_STATUS_SLEEPING;

	if (debug) printk("vfork: wake child %d\n",child_pid);
	child->status=PROCESS_STATUS_READY;

	/* Update saved process state of child */
	/* It makes a copy of parent, but with SP pointing to child kernel stack */

	register long r13 asm ("r13");
	long old_stack_offset;

	old_stack_offset=r13-(long)parent;
	new_stack=(long)child+old_stack_offset;

	if (debug) printk("vfork: parent=%x child=%x r13=%x old_stack_offset %x new_stack %x\n",
		(long)parent,(long)child,r13,old_stack_offset,new_stack);

	process_save(child,new_stack);

	if (current_process==parent) {
		if (debug) printk("parent status %d, ready=%d sleep=%d running schedule\n",
				parent->status,
				PROCESS_STATUS_READY,PROCESS_STATUS_SLEEPING);
		schedule();
		if (debug) printk("vfork: returning in parent, status=%d\n",
				parent->status);
		return child_pid;
	}

	/* In child, return 0 */
	if (debug) printk("vfork: returning in child\n");

	return 0;
}

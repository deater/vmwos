#include <stddef.h>
#include <stdint.h>

#include "exec.h"
#include "memory.h"
#include "process.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"


int userspace_started=0;
static int avail_pid=0;

struct process_control_block_type *proc_first=NULL;
struct process_control_block_type *current_process=NULL;

struct process_control_block_type *process_lookup(int32_t pid) {

	struct process_control_block_type *result=NULL;

	result=proc_first;
	while(result!=NULL) {
		if (result->pid==pid) return result;

		result=result->next;
	}

	return result;

}

static int32_t process_insert(struct process_control_block_type *proc) {

	struct process_control_block_type *last;

	if (proc_first==NULL) {
		proc_first=proc;
		proc->next=NULL;
		return 0;
	}

	last=proc_first;
	while(last->next!=NULL) {
		last=last->next;
	}
	last->next=proc;
	proc->next=NULL;

	return 0;
}

struct process_control_block_type *process_create(void) {

	int i;
	struct process_control_block_type *new_proc;

	new_proc=memory_allocate(sizeof(struct process_control_block_type));
	if (new_proc==NULL) {
		printk("process_create: out of memory\n");
		return NULL;
	}

	/* Insert into linked list */
	process_insert(new_proc);

	/* Set up initial conditions */
	new_proc->running=0;
	new_proc->status=PROCESS_STATUS_SLEEPING;
	new_proc->time=0;

	/* LOCK */
	/* FIXME: what happens when we rollover */
	new_proc->pid=avail_pid;
	avail_pid++;
	/* UNLOCK */

	new_proc->stack=NULL;
	new_proc->text=NULL;
	new_proc->stacksize=0;
	new_proc->textsize=0;

	/* Clear out registers */
	for(i=0;i<14;i++) new_proc->reg_state.r[i]=0;

	/* Setup the default SPSR */
	/* USER mode (0x10) */
	/* We don't mask 0x80 or 0x40 (IRQ or FIQ) */
	new_proc->reg_state.spsr=0x10;

	/* Clear out LR (saved pc) */
	/* exec should set this for us */
	new_proc->reg_state.lr=0;

	return new_proc;
}

int32_t process_destroy(struct process_control_block_type *proc) {

	/* close open files */
	/* TODO */

	/* free memory */
	if (proc->stack) {
		memory_free(proc->stack,proc->stacksize);
	}
	if (proc->text) {
		memory_free(proc->text,proc->textsize);
	}
	/* mark as no longer valid */
	proc->valid=0;

	/* remove from linked list? */

	return 0;
}

#if 0
int32_t process_load(char *name, int type, char *data, int size, int stack_size) {

	char *binary_start=NULL;
	char *stack_start=NULL;
	int32_t which;

	which=process_create();
	if (which<0) {
		return which;
	}

	if (type==PROCESS_FROM_DISK) {
		execve(name,NULL,NULL);
	}
	else if (type==PROCESS_FROM_RAM) {
		/* Allocate Memory */
        	binary_start=(char *)memory_allocate(size);
        	stack_start=(char *)memory_allocate(stack_size);

		/* Load executable */
		//printk("Copying %d bytes from %x to %x\n",size,data,binary_start);
        	memcpy(binary_start,data,size);
	}
	else {
		printk("Unknown process type!\n");
		process_destroy(which);
		return -1;
	}

	return which;
}
#endif

int32_t process_run(struct process_control_block_type *proc, long *irq_stack) {

	int i;
	long return_pc,our_spsr;
	static int start_debug=0;

//	printk("Resetting IRQ stack to %x\n",irq_stack);

	return_pc=proc->reg_state.lr;
	our_spsr=proc->reg_state.spsr;

//	if (which==2) start_debug=1;
	if (start_debug) {
//		printk("IRQ stack=%x\n",(long)irq_stack);
		printk("Attempting to run proc %x (%s pid=%d): "
			"PC=%x SPSR=%x stack=%x\n",
			(long)proc,
			proc->name,proc->pid,
			return_pc,our_spsr,
			proc->reg_state.r[13]);
	}

	irq_stack[0]=our_spsr;
	irq_stack[1]=return_pc;
	for(i=0;i<15;i++) {
		irq_stack[2+i]=proc->reg_state.r[i];
	}

	proc->running=1;
	current_process=proc;

#if 0
	/* restore user registers */
	/* update status */
	/* jump to saved user PC */



	asm volatile(
		"mov r0, %[our_sp]\n"
		"msr SPSR_cxsf, %[our_spsr]\n"
		"mov lr, %[return_pc]\n"
		"mov sp,%[irq_stack]\n"
		"ldmia r0, {r0 - lr}^\n"	/* the ^ means load user regs */
		"nop\n"
		/* Need to reset IRQ stack here or we leak */
		"movs pc, lr\n"			/* movs with pc changes mode */
		: /* output */
		:	[our_sp] "r"(our_sp),
			[return_pc] "r"(return_pc),
			[irq_stack] "r"(irq_stack),
			[our_spsr] "r"(our_spsr) /* input */
		: "lr", "sp", "r0", "memory" /* clobbers */
			);





	/* set user stack */
        asm volatile(
                "msr CPSR_c, #0xDF\n" /* System mode, like user but privldg */
                "mov sp, %[stack]\n"
                "msr CPSR_c, #0xD3\n" /* Back to Supervisor mode */
                : /* output */
                : [stack] "r"(shell_stack) /* input */
                : "sp", "memory");      /* clobbers */

        /* enter userspace */

        asm volatile(
                "mov r0, #0x10\n"
                "msr SPSR, r0\n"
                "mov lr, %[shell]\n"
                "movs pc,lr\n"
                : /* output */
                : [shell] "r"(shell_address) /* input */
                : "r0", "lr", "memory");        /* clobbers */
#endif

	return 0;
}

int32_t process_save(struct process_control_block_type *proc, long *irq_stack) {

	int i;

	/* No longer running */
	proc->running=0;

	proc->reg_state.spsr=irq_stack[0];
	proc->reg_state.lr=irq_stack[1];
//	printk("save: SPSR=%x PC=%x IRQ_STACK=%x ",
//		irq_stack[0],irq_stack[1],
//		irq_stack);

	for(i=0;i<15;i++) {
		proc->reg_state.r[i]=irq_stack[i+2];
//		printk("r%d=%x ",i,irq_stack[i+2]);
	}
//	printk("\n");

	return 0;
}

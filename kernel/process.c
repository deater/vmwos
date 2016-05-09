#include <stddef.h>
#include <stdint.h>

#include "exec.h"
#include "memory.h"
#include "process.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"


int userspace_started=0;
int current_process=0;
struct process_control_block_type process[MAX_PROCESSES];
static int avail_pid=0;


int32_t process_table_init(void) {

	int i;

	current_process=0;
	for(i=0;i<MAX_PROCESSES;i++) {
		process[i].valid=0;
	}

	return 0;
}

static int32_t process_find_free(void) {

	int32_t i,which=0;

	/* LOCK */

	/* Find free process */
	for(i=0;i<MAX_PROCESSES;i++) {
		if (!process[i].valid) break;
	}

	if (i==MAX_PROCESSES) {
		printk("ERROR: No free process slot!\n");
		which=-1;
	}
	else {
		process[i].valid=1;
		which=i;
	}

	/* UNLOCK */

	return which;
}

int32_t process_create(void) {

	int32_t which;
	int i;

	which=process_find_free();
	if (which<0) {
		return which;
	}

	/* Set up initial conditions */
	process[which].running=0;
	process[which].status=PROCESS_STATUS_SLEEPING;
	process[which].time=0;

	/* LOCK */
	/* FIXME: what happens when we rollover */
	process[which].pid=avail_pid;
	avail_pid++;
	/* UNLOCK */

	process[which].stack=NULL;
	process[which].text=NULL;
	process[which].stacksize=0;
	process[which].textsize=0;

	/* Clear out registers */
	for(i=0;i<14;i++) process[which].reg_state.r[i]=0;

	/* Setup the default SPSR */
	/* USER mode (0x10) */
	/* We don't mask 0x80 or 0x40 (IRQ or FIQ) */
	process[which].reg_state.spsr=0x10;

	/* Clear out LR (saved pc) */
	/* exec should set this for us */
	process[which].reg_state.lr=0;

	return which;
}

int32_t process_destroy(int32_t which) {

	/* close open files */
	/* TODO */

	/* free memory */
	if (process[which].stack) {
		memory_free(process[which].stack,
			process[which].stacksize);
	}
	if (process[which].text) {
		memory_free(process[which].text,
			process[which].textsize);
	}
	/* mark as no longer valid */
	process[which].valid=0;

	return 0;
}

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

int32_t process_run(int which, long *irq_stack) {

	int i;
	long return_pc,our_spsr;
	static int start_debug=0;

//	printk("Resetting IRQ stack to %x\n",irq_stack);

	return_pc=process[which].reg_state.lr;
	our_spsr=process[which].reg_state.spsr;

//	if (which==2) start_debug=1;
	if (start_debug) {
//		printk("IRQ stack=%x\n",(long)irq_stack);
		printk("Attempting to run proc %d (%s pid=%d): "
			"PC=%x SPSR=%x stack=%x\n",
			which, process[which].name,process[which].pid,
			return_pc,our_spsr,
			process[which].reg_state.r[13]);
	}

	irq_stack[0]=our_spsr;
	irq_stack[1]=return_pc;
	for(i=0;i<15;i++) {
		irq_stack[2+i]=process[which].reg_state.r[i];
	}

	process[which].running=1;
	current_process=which;

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

int32_t process_save(int which, long *irq_stack) {

	int i;

	/* No longer running */
	process[which].running=0;

	process[which].reg_state.spsr=irq_stack[0];
	process[which].reg_state.lr=irq_stack[1];
//	printk("save: SPSR=%x PC=%x IRQ_STACK=%x ",
//		irq_stack[0],irq_stack[1],
//		irq_stack);

	for(i=0;i<15;i++) {
		process[which].reg_state.r[i]=irq_stack[i+2];
//		printk("r%d=%x ",i,irq_stack[i+2]);
	}
//	printk("\n");

	return 0;
}

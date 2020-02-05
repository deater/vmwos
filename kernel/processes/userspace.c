#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/smp.h"

#include "memory/memory.h"

#include "processes/idle_task.h"
#include "processes/process.h"

#include "syscalls/exec.h"

#include "time/time.h"

#include "fs/files.h"

static int debug=0;

void enter_userspace(void) {

	/* enter userspace */

	long shell_address=current_proc[0]->user_state.pc;
	long cp_address=(long)(&current_proc[0]);

	asm volatile(
		"mov	lr, %[shell]\n"
		"mov	sp, %[cp]\n"
		"ldr	sp,[sp]\n"
		"mov	r0, #0x10\n"	/* Userspace, IRQ enabled */
		"msr	SPSR_cxsf, r0\n"
		"movs	pc,lr\n"
		: /* output */
		: [shell] "r"(shell_address),
		  [cp] "r"(cp_address) /* input */
		: "r0", "lr", "memory");        /* clobbers */
}

void start_userspace(char *init_filename) {

	struct process_control_block_type *init_process;
	int32_t result;

	if (debug) printk("Starting init...\n");

	init_process=process_create();
	current_proc[0]=init_process;
	/* Should this be NULL instead? */
	init_process->parent=init_process;

	/* Set current working directory */
	strncpy(init_process->current_dir,"/",MAX_PATH_LEN);

	/* Set up stdin / stdout / stderr */
	init_process->files[0]=file_special(0);
	init_process->files[1]=file_special(1);
	init_process->files[2]=file_special(2);

	if (debug) printk("Execing init...\n");

	result=execve(init_filename,NULL,NULL);
	if (result<0) {
		goto error_init;
	}

	printk("\nEntering userspace by starting process %d (%s)!\n",
		init_process->pid,init_process->name);

	/* Mark idle and init as ready */
	init_process->status=PROCESS_STATUS_READY;

	long *shell_stack=(long *)init_process->user_state.r[13];

	asm volatile(
                "msr CPSR_c, #0xDF\n" /* System mode, like user but privldg */
                "mov sp, %[stack]\n"
                "msr CPSR_c, #0xD3\n" /* Back to Supervisor mode */
					/* with interrupts disabled */
                : /* output */
                : [stack] "r"(shell_stack) /* input */
                : "sp", "memory");      /* clobbers */

	enter_userspace();

error_init:

	/* We should not get here */

	return;

}

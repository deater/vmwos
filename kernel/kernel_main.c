#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"

#include "boot/hardware_detect.h"

#include "drivers/drivers.h"
#include "drivers/block/ramdisk.h"
#include "drivers/serial/serial.h"

#include "fs/files.h"

#include "memory/memory.h"

#include "processes/idle_task.h"
#include "processes/process.h"

#include "syscalls/exec.h"

#include "time/time.h"

#include "arch/arm1176/arm1176-mmu.h"

/* Initrd hack */
#include "../userspace/initrd.h"

#include "version.h"

/* For memory benchmark */
#define BENCH_SIZE (1024*1024)
//uint8_t benchmark[BENCH_SIZE];

void enter_userspace(void) {

	/* enter userspace */

	long shell_address=current_process->user_state.pc;

	asm volatile(
		"mov	lr, %[shell]\n"
		"ldr	sp,=current_process\n"
		"ldr	sp,[sp]\n"
		"mov	r0, #0x10\n"	/* Userspace, IRQ enabled */
		"msr	SPSR_cxsf, r0\n"
		"movs	pc,lr\n"
		: /* output */
		: [shell] "r"(shell_address) /* input */
		: "r0", "lr", "memory");        /* clobbers */
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t r2,
		uint32_t memory_kernel) {

	struct process_control_block_type *init_process,*idle_process;
	int32_t result;

	(void) r0;	/* Ignore boot method */

//	early_debug_init();
//	early_debug_dump_memory(0x14d764,4096);


	/*******************/
	/* Detect Hardware */
	/*******************/

	result=hardware_detect((uint32_t *)r2);

	/*****************************/
	/* Initialize Serial Console */
	/*****************************/

	/* Serial console is most important so do that first */
	serial_init(SERIAL_UART_PL011);
	serial_printk("\n\n\nUsing pl011-uart\n");

	/************************/
	/* Boot messages!	*/
	/************************/

	printk("From bootloader: r0=%x r1=%x r2=%x\n",
		r0,r1,r2);
	printk("\nBooting VMWos...\n");

	/* Print boot message */
	printk("\033[0;41m   \033[42m \033[44m   \033[42m \033[44m   \033[0m VMW OS\n");
	printk(" \033[0;41m \033[42m   \033[44m \033[42m   \033[44m \033[0m  Version %s\n\n",VERSION);

	printk("Detected hardware:\n");

	/* Print model info */
	hardware_print_model(r1);

	/* Print command line */
	hardware_print_commandline();

	/**************************/
	/* Init Device Drivers	  */
	/**************************/

	drivers_init_all();

	/**************************/
	/* Init Memory Hierarchy  */
	/**************************/

	memory_hierarchy_init(memory_kernel);

	/************************/
	/* Other init		*/
	/************************/

	/* Init the file descriptor table */
	fd_table_init();

	/* Initialize the ramdisk */
	ramdisk_init(initrd_image,sizeof(initrd_image));

	/* Mount the ramdisk */
	mount("/dev/ramdisk","/","romfs",0,NULL);

	/* Create idle thread */
	idle_process=process_create();
	idle_process->text=(void *)&idle_task;
	idle_process->user_state.pc=(long)&idle_task;
	idle_process->running=1;
	idle_process->total_time=0;
	idle_process->start_time=ticks_since_boot();
	idle_process->last_scheduled=idle_process->start_time;
	strncpy(idle_process->name,"idle",5);
	idle_process->kernel_state.r[14]=(long)enter_userspace;
	printk("Created idle thread: %d\n",idle_process->pid);
	//dump_saved_user_state(idle_process);
	//dump_saved_kernel_state(idle_process);

	/* Enter our "init" process*/
	init_process=process_create();
	current_process=init_process;
	/* Should this be NULL instead? */
	init_process->parent=init_process;
	init_process->current_dir=get_inode("/");
	result=execve("shell",NULL,NULL);
	if (result<0) {
		goto error_init;
	}

	printk("\nEntering userspace by starting process %d (%s)!\n",
		init_process->pid,init_process->name);

	/* Mark idle and init as ready */
	idle_process->status=PROCESS_STATUS_READY;
	init_process->status=PROCESS_STATUS_READY;

	long *shell_stack=(long *)init_process->user_state.r[13];



#if 0
	/* Setup userspace to point to process 1 */
	/* process_run(r1,&stack); */
	asm volatile(
		"sub sp,sp,#64\n"	/* Put place for reg state on stack*/
		"mov r0,#1\n"		/* Run process 1 */
		"mov r1,sp\n"		/* point to stack */
		"bl process_run\n"
		: /* output */
		: /* input */
		: "sp", "memory");      /* clobbers */


#endif

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

	/* we should never get here */
	printk("Error starting init!\n");



	while(1) {

		/* Loop Forever */
		/* Should probably execute a wfi instruction */
		/* In theory only here for HZ until scheduler kicks in */
	}

}

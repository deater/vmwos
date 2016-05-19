#include <stddef.h>
#include <stdint.h>

#include "drivers/block/ramdisk.h"
#include "drivers/serial/pl011_uart.h"
#include "lib/printk.h"
#include "boot/atags.h"
#include "drivers/led/led.h"
#include "delay.h"
#include "drivers/timer/timer.h"
#include "interrupts.h"
#include "bcm2835_periph.h"
#include "mmio.h"
#include "memory.h"
#include "syscalls.h"
#include "hardware.h"
#include "drivers/framebuffer/framebuffer.h"
#include "drivers/framebuffer/framebuffer_console.h"
#include "lib/string.h"
#include "process.h"
#include "scheduler.h"
#include "idle_task.h"
#include "drivers/keyboard/ps2-keyboard.h"
#include "syscalls/time.h"
#include "lib/div.h"
#include "arch/arm1176/arm1176-mmu.h"
#include "arch/arm1176/arm1176-pmu.h"
#include "drivers/thermal/thermal.h"
#include "fs/files.h"
#include "fs/romfs/romfs.h"
#include "lib/memset.h"
#include "lib/memory_benchmark.h"
#include "drivers/random/bcm2835_rng.h"
#include "syscalls/exec.h"
#include "panic.h"

/* Initrd hack */
#include "../userspace/initrd.h"

#include "version.h"

/* default, this is over-ridden later */
uint32_t hardware_type=RPI_MODEL_B;

/* For memory benchmark */
#define BENCH_SIZE (1024*1024)
uint8_t benchmark[BENCH_SIZE];

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




void kernel_main(uint32_t r0, uint32_t r1, uint32_t *atags,
		uint32_t memory_kernel) {

	struct process_control_block_type *init_process,*idle_process;
	struct atag_info_t atag_info;
	uint32_t framebuffer_width=800,framebuffer_height=600;
	uint32_t temperature;
	int32_t result;

	(void) r0;	/* Ignore boot method */

	/* Initialize Software Structures */

	/* Detect Hardware */
	atags_detect(atags,&atag_info);
	hardware_type=atag_info.hardware_type;

	/* Initialize Hardware */

	/* Serial console is most important so do that first */
	uart_init();

	/* Enable HW random number generator */
	bcm2835_rng_init();

	/************************/
	/* Boot message!	*/
	/************************/

	printk("From bootloader: r0=%x r1=%x r2=%x\n",
		r0,r1,(uint32_t)atags);
	printk("\nBooting VMWos...\n");



	/**************************/
	/* Device Drivers	  */
	/**************************/

	/* Set up ACT LED */
	led_init();

	/* Set up timer */
	timer_init();

	/* Set up keyboard */
	ps2_keyboard_init();

	/* Enable the Framebuffer */
	if (atag_info.framebuffer_x!=0) {
		framebuffer_width=atag_info.framebuffer_x;
	}
	if (atag_info.framebuffer_y!=0) {
		framebuffer_height=atag_info.framebuffer_y;
	}

	framebuffer_init(framebuffer_width,framebuffer_height,24);
	framebuffer_console_init();

#if 0
	/* Delay to allow time for serial port to settle */
	/* So we can actually see the output on the terminal */
	delay(0x3f0000);

	printk("\nWaiting for serial port to be ready (press any key)\n");
	uart_getc();
#endif

	uart_enable_interrupts();

	/* Enable Interrupts */
//	enable_interrupts();


	/* Clear screen */
	printk("\n\033[2J\n\n");

	/* Print boot message */
	printk("\033[0;41m   \033[42m \033[44m   \033[42m \033[44m   \033[0m VMW OS\n");
	printk(" \033[0;41m \033[42m   \033[44m \033[42m   \033[44m \033[0m  Version %s\n\n",VERSION);

	/* Print hardware version */
	printk("Hardware version: %x ",r1);
	if (r1==0xc42) printk("(Raspberry Pi)");
	else printk("(Unknown Hardware)");
	printk("\n");

	printk("Detected Model ");
	switch(hardware_type) {
		case RPI_MODEL_A:	printk("A"); break;
		case RPI_MODEL_APLUS:	printk("A+"); break;
		case RPI_MODEL_B:	printk("B"); break;
		case RPI_MODEL_BPLUS:	printk("B+"); break;
		case RPI_MODEL_B2:	printk("B2"); break;
		case RPI_COMPUTE_NODE:	printk("Compute Node"); break;
		default:		printk("Unknown %x",hardware_type); break;
	}
	printk("\n");

	/* Check temperature */
	temperature=thermal_read();
	printk("CPU Temperature: %dC, %dF\n",
		temperature/1000,
		((temperature*9)/5000)+32);

	/* Print ATAGS */
	atags_dump(atags);

	printk("\n");

	/* Get amount of RAM from ATAGs */
	memory_total=atag_info.ramsize;

	/* Init memory subsystem */
	memory_init(memory_total,memory_kernel);

	/* Start HW Perf Counters */
	arm1176_init_pmu();

	/* Setup Memory Hierarchy */
#if 0
	memset_benchmark(memory_total);
#else
	/* Enable L1 i-cache */
	printk("Enabling L1 icache\n");
	enable_l1_icache();

	/* Enable branch predictor */
	printk("Enabling branch predictor\n");
	enable_branch_predictor();

	/* Enable L1 d-cache */
	printk("Enabling MMU with 1:1 Virt/Phys page mapping\n");
	enable_mmu(0,memory_total,memory_kernel);
	printk("Enabling L1 dcache\n");
	enable_l1_dcache();
#endif

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

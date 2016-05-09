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
#include "time.h"
#include "lib/div.h"
#include "arch/arm1176/arm1176-mmu.h"
#include "arch/arm1176/arm1176-pmu.h"
#include "drivers/thermal/thermal.h"
#include "fs/files.h"
#include "fs/romfs/romfs.h"
#include "lib/memset.h"
#include "lib/memory_benchmark.h"
#include "drivers/random/bcm2835_rng.h"
#include "exec.h"

/* Initrd hack */
#include "../userspace/initrd.h"


#define VERSION 12

/* default, this is over-ridden later */
uint32_t hardware_type=RPI_MODEL_B;

/* For memory benchmark */
#define BENCH_SIZE (1024*1024)
uint8_t benchmark[BENCH_SIZE];

void kernel_main(uint32_t r0, uint32_t r1, uint32_t *atags,
		uint32_t memory_kernel) {

	unsigned int memory_total;
	int init_process,idle_process;
	struct atag_info_t atag_info;
	uint32_t framebuffer_width=800,framebuffer_height=600;
	uint32_t temperature;

	(void) r0;	/* Ignore boot method */

	/* Initialize Software Structures */
	process_table_init();

	/* Detect Hardware */
	atags_detect(atags,&atag_info);
	hardware_type=atag_info.hardware_type;

	/* Initialize Hardware */

	/* Serial console is most important so do that first */
	uart_init();

	/* Enable HW random number generator */
	bcm2835_rng_init();

	/* Enable Interrupts */
	enable_interrupts();

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


	/* Clear screen */
	printk("\n\033[2J\n\n");

	/* Print boot message */
	printk("\033[0;41m   \033[42m \033[44m   \033[42m \033[44m   \033[0m VMW OS\n");
	printk(" \033[0;41m \033[42m   \033[44m \033[42m   \033[44m \033[0m  Version 0.%d\n\n",VERSION);

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
	enable_mmu(0,memory_total);
	printk("Enabling L1 dcache\n");
	enable_l1_dcache();
#endif

	/* Init the file descriptor table */
	fd_table_init();

	/* Initialize the ramdisk */
	ramdisk_init(initrd_image,sizeof(initrd_image));

	/* Mount the ramdisk */
	mount("/dev/ramdisk","/","romfs",0,NULL);

#if 0
	/* Load the idle thread */
	idle_process=process_load("idle",PROCESS_FROM_RAM,
				(char *)&idle_task,8,4096);

	init_process=process_load("shell",PROCESS_FROM_DISK,
				NULL,0,8192);

	process_load("printa",PROCESS_FROM_DISK,
				NULL,0,8192);

	process_load("printb",PROCESS_FROM_DISK,
				NULL,0,8192);
#endif

	/* Create idle thread */
	printk("Loading the idle thread\n");
	idle_process=process_create();
	process[idle_process].text=(void *)&idle_task;
	process[idle_process].reg_state.lr=(long)&idle_task;
	process[idle_process].running=1;

	/* Enter our "init" process*/
	init_process=process_create();
	current_process=1;
	execve("shell",NULL,NULL);
	printk("\nEntering userspace by starting process %d!\n",
		init_process);

	process[idle_process].status=PROCESS_STATUS_READY;
	process[init_process].status=PROCESS_STATUS_READY;;

	userspace_started=1;

	/* run init and restore stack as we won't return */
	process_run(init_process,0x8000);

	/* we should never get here */

	while(1) {

		/* Loop Forever */
		/* Should probably execute a wfi instruction */
	}

}

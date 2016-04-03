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
#include "scheduler.h"
#include "idle_task.h"
#include "drivers/keyboard/ps2-keyboard.h"
#include "time.h"
#include "mmu.h"
#include "lib/div.h"
#include "drivers/thermal/thermal.h"
#include "fs/files.h"
#include "fs/romfs/romfs.h"

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
	processes_init();

	/* Detect Hardware */
	atags_detect(atags,&atag_info);
	hardware_type=atag_info.hardware_type;

	/* Initialize Hardware */

	/* Serial console is most important so do that first */
	uart_init();

	/* Enable Interrupts */
	enable_interrupts();

	/************************/
	/* Boot message!	*/
	/************************/

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

	/* Delay to allow time for serial port to settle */
	/* So we can actually see the output on the terminal */
	delay(0x3f0000);

	printk("\nWaiting for serial port to be ready (press any key)\n");
	uart_getc();

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

	/* Init the MMU */
	printk("Initializing MMU and caches\n");

	enable_l1_icache();
	enable_branch_predictor();
	enable_mmu(0, memory_total);
	enable_l1_dcache();

	/* Init the file descriptor table */
	fd_table_init();

	/* Initialize the ramdisk */
	ramdisk_init(initrd_image,sizeof(initrd_image));

	/* Mount the ramdisk */
	mount("/dev/ramdisk","/","romfs",0,NULL);

	/* Memory Benchmark */
#if 1
	{
		int i;
		uint32_t before,after;

		before=ticks_since_boot();

		for(i=0;i<16;i++) {
			memset(benchmark,0,BENCH_SIZE);
		}
		after=ticks_since_boot();

		printk("MEMSPEED: %d MB took %d ticks %dkB/s\n",
			16, (after-before),
			div32(16*1024,(((after-before)*1000)/64)));
	}
#endif

	/* Load the idle thread */
	idle_process=load_process("idle",PROCESS_FROM_RAM,
				(char *)&idle_task,8,4096);

	init_process=load_process("shell",PROCESS_FROM_DISK,
				NULL,0,8192);

	load_process("printa",PROCESS_FROM_DISK,
				NULL,0,8192);

	load_process("printb",PROCESS_FROM_DISK,
				NULL,0,8192);


	/* Enter our "init" process*/
	printk("\nEntering userspace by starting process %d!\n",
		init_process);

	process[idle_process].ready=1;
	process[init_process].ready=1;

	userspace_started=1;

	/* run init and restore stack as we won't return */
	run_process(init_process,0x8000);

	/* we should never get here */

	while(1) {

		/* Loop Forever */
		/* Should probably execute a wfi instruction */
	}

}

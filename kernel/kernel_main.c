#include <stddef.h>
#include <stdint.h>
#include "serial.h"
#include "printk.h"
#include "atags.h"
#include "led.h"
#include "delay.h"
#include "timer.h"
#include "interrupts.h"
#include "bcm2835_periph.h"
#include "mmio.h"
#include "memory.h"
#include "syscalls.h"
#include "hardware.h"
#include "framebuffer.h"
#include "framebuffer_console.h"
#include "string.h"
#include "scheduler.h"
#include "idle_task.h"

#define VERSION 10

/* default, this is over-ridden later */
int hardware_type=RPI_MODEL_B;

void kernel_main(uint32_t r0, uint32_t r1, uint32_t *atags,
		uint32_t memory_kernel) {

	unsigned int memory_total;
	int init_process,idle_process;

	(void) r0;	/* Ignore boot method */

	/* Initialize Software Structures */
	processes_init();

	/* Detect Hardware */
	atags_detect(atags);

	/* Initialize Hardware */
	uart_init();
	led_init();
	timer_init();

	/* Enable Interrupts */
	enable_interrupts();

	printk("\r\nBooting VMWos...\r\n");

	/* Enable the Framebuffer */
	framebuffer_init(800,600,24);
	framebuffer_console_init();


	/* Delay to allow time for serial port to settle */
	/* So we can actually see the output on the terminal */
	delay(0x3f0000);

	printk("\r\nWaiting for serial port to be ready (press any key)\r\n");
	uart_getc();

	/* Clear screen */
	printk("\n\r\033[2J\n\r\n\r");

	/* Print boot message */
	printk("\033[0;41m   \033[42m \033[44m   \033[42m \033[44m   \033[0m VMW OS\r\n");
	printk(" \033[0;41m \033[42m   \033[44m \033[42m   \033[44m \033[0m  Version 0.%d\r\n\r\n",VERSION);

	/* Print hardware version */
	printk("Hardware version: %x ",r1);
	if (r1==0xc42) printk("(Raspberry Pi)");
	else printk("(Unknown Hardware)");
	printk("\r\n");

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
	printk("\r\n");

	/* Print ATAGS */
	atags_dump(atags);

	printk("\r\n");

	/* Get amount of RAM from ATAGs */
	memory_total=atags_detect_ram(atags);

	/* Init memory subsystem */
	memory_init(memory_total,memory_kernel);

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
	printk("\r\nEntering userspace by starting process %d!\r\n",
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

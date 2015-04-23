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

#define VERSION 10

/* default, this is over-ridden later */
int hardware_type=RPI_MODEL_B;

#include "shell.h"

void kernel_main(uint32_t r0, uint32_t r1, uint32_t *atags,
		uint32_t memory_kernel) {

	unsigned int memory_total;
	int which_process;

	(void) r0;	/* Ignore boot method */

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


	/* Enter our "shell" */
	printk("\r\nEntering userspace!\r\n");

	which_process=load_process(shell_binary, sizeof(shell_binary),
				DEFAULT_STACK_SIZE);

	run_process(which_process);

#if 0

	char *shell_address,*shell_stack;

	/* Load the shell */
	shell_address=(char *)memory_allocate(8192);
	shell_stack=(char *)memory_allocate(4096);

	printk("Allocated 8kB at %x and stack at %x\r\n",
		shell_address,shell_stack);

	memcpy(shell_address,shell_binary,sizeof(shell_binary));

	/* Grows down */
	shell_stack+=4092;

	/* jump to the shell */

	/* set user stack */
	asm volatile(
		"msr CPSR_c, #0xDF\n" /* System mode, like user but privldg */
		"mov sp, %[stack]\n"
		"msr CPSR_c, #0xD3\n" /* Back to Supervisor mode */
                : /* output */
                : [stack] "r"(shell_stack) /* input */
                : "sp", "memory");	/* clobbers */

	/* enter userspace */

	asm volatile(
                "mov r0, #0x10\n"
		"msr SPSR, r0\n"
		"mov lr, %[shell]\n"
		"movs pc,lr\n"
                : /* output */
                : [shell] "r"(shell_address) /* input */
                : "r0", "lr", "memory");	/* clobbers */

#endif


	//shell();

	while(1) {

		/* Loop Forever */
		/* Should probably execute a wfi instruction */
	}

}

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

#define VERSION 10

#if 0

static int parse_input(char *string) {

	int result=0;

	if ((string[0]=='l') && (string[1]=='e') && (string[2]=='d')) {
		if (string[5]=='n') led_on();
		if (string[5]=='f') led_off();
	}
	else if ((string[0]=='t') && (string[1]=='i') && (string[2]=='m')) {
		printk("\r\nTimer: %d %d\r\n",mmio_read(TIMER_VALUE),0);
	}
	else if ((string[0]=='w') && (string[1]=='r') && (string[2]=='i')) {
		result=syscall3(STDOUT,
				(int)"WRITE SYSCALL TEST\r\n",
				20,
				SYSCALL_WRITE);
		printk("\r\nSyscall returned %d\r\n",result);
	}
	else if ((string[0]=='o') && (string[1]=='n')) {
		result=syscall1(1,SYSCALL_BLINK);
	}
	else if ((string[0]=='o') && (string[1]=='f')) {
		result=syscall1(0,SYSCALL_BLINK);
	}
	else if (string[0]=='f') {
		framebuffer_console_setfont(string[1]-'0');
	}

	else if ((string[0]=='g') && (string[1]=='r')) {

		int x;

		for(x=0;x<800;x++) {
			framebuffer_vline( (x*256)/800, 0, 599, x);
		}
		framebuffer_push();

	}
	else if ((string[0]=='t') && (string[1]=='b')) {
		framebuffer_tb1();
	}
	else {
		printk("\r\nUnknown commmand!");
	}

	return result;
}

#endif

/* default, this is over-ridden later */
int hardware_type=RPI_MODEL_B;

void kernel_main(uint32_t r0, uint32_t r1, uint32_t *atags,
		uint32_t memory_kernel) {

//	char input_string[256];
//	int input_pointer;
//	int ch;

	unsigned int memory_total;

	(void) r0;	/* Ignore boot method */

	/* Detect Hardware */
	atags_detect(atags);

	/* Initialize Hardware */
	uart_init();
	led_init();
	timer_init();
	framebuffer_init(800,600,24);
	framebuffer_console_init();

	/* Enable Interrupts */
	enable_interrupts();

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

	/* Test memory allocation */
	printk("Allocated 8kB at %x\r\n",
		memory_allocate(8192));


	/* Enter our "shell" */
	printk("\r\nEntering userspace!\r\n");

	while(1) {

#if 0
	while (1) {
		input_pointer=0;
		printk("] ");

		while(1) {
			ch=uart_getc();
			if ((ch=='\n') || (ch=='\r')) {
				input_string[input_pointer]=0;
				parse_input(input_string);
				printk("\r\n");
				break;
			}

			input_string[input_pointer]=ch;
			input_pointer++;
			printk("%c",ch);
//			uart_putc(ch);
		}
	}
#endif

	}

}

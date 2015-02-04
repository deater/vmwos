#include <stddef.h>
#include <stdint.h>
#include "serial.h"
#include "printk.h"
#include "atags.h"
#include "led.h"
#include "delay.h"

#define VERSION 2

static int parse_input(char *string) {

	if ((string[0]=='l') && (string[1]=='e') && (string[2]=='d')) {
		if (string[5]=='n') led_on();
		if (string[5]=='f') led_off();
	}
	else {
		printk("\r\nUnknown commmand!");
	}

	return 0;
}


void kernel_main(uint32_t r0, uint32_t r1, uint32_t *atags) {

	char input_string[256];
	int input_pointer;
	int ch;

	(void) r0;	/* Ignore boot method */

	/* Initialize Hardware */
	uart_init();
	led_init();

	delay(0x3f0000);

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

	/* Print ATAGS */
	dump_atags(atags);

	/* Enter our "shell" */
	printk("\r\nReady!\r\n");

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
			uart_putc(ch);
		}
	}
}

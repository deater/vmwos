/* This code attempts to drive directly to the serial port */
/* To allow debugging even when proper support not available */

#include <stddef.h>
#include <stdint.h>

#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/bcm2835/bcm2835_periph.h"

#include "drivers/gpio/gpio.h"
#include "drivers/console/console_io.h"
#include "interrupts/interrupts.h"
#include "lib/printk.h"
#include "lib/delay.h"

#include "drivers/serial/serial.h"
#include "drivers/serial/pl011_uart.h"

uint32_t early_debug_init(void) {

	uint32_t old;

	/* Disable UART */
	bcm2835_write(UART0_CR, 0x0);

	/* Setup GPIO pins 14 and 15 */
//	gpio_request(14,"uart_tx");
//	gpio_request(15,"uart_rx");


	/* Set GPIO14 and GPIO15 to be pl011 TX, so ALT0        */
	/* ALT0 is binary 100 (0x4)                             */
	old=bcm2835_read(GPIO_GPFSEL1);
	old &= ~(0x7 << 12);
	old |= (4<<12);

	old &= ~(0x7 << 15);
	old |= (4<<15);
	bcm2835_write(GPIO_GPFSEL1,old);


	/* Disable the pull up/down on pins 14 and 15 */
	/* See the Peripheral Manual for more info */
	/* Configure to disable pull up/down and delay for 150 cycles */
	bcm2835_write(GPIO_GPPUD, GPIO_GPPUD_DISABLE);
	delay(150);

	/* Pass the disable clock to GPIO pins 14 and 15 and delay*/
	bcm2835_write(GPIO_GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);

	/* Write 0 to GPPUDCLK0 to make it take effect */
	bcm2835_write(GPIO_GPPUDCLK0, 0x0);

	/* Clear pending interrupts. */
	bcm2835_write(UART0_ICR, 0x7FF);

	/* Set integer & fractional part of baud rate. */
	/* Divider = UART_CLOCK/(16 * Baud)            */
	/* Fraction part register = (Fractional part * 64) + 0.5 */
	/* UART_CLOCK = 3000000; Baud = 115200.        */

	/* Divider = 3000000 / (16 * 115200) = 1.627   */
	/* Integer part = 1 */
	/* Fractional part register = (.627 * 64) + 0.5 = 40.6 = 40 */

	/* On Pi3 and (all pis with newer firmware) UART_CLOCK is 48MHz */
	bcm2835_write(UART0_IBRD, 26);
	bcm2835_write(UART0_FBRD, 3);

	/* Enable FIFO */
	/* Set 8N1 (8 bits of data, no parity, 1 stop bit */
	bcm2835_write(UART0_LCRH, UART0_LCRH_FEN | UART0_LCRH_WLEN_8BIT);

	/* Mask all interrupts. */
	/* URGH to mask them "off" write a 0, not a 1 :( */
	bcm2835_write(UART0_IMSC, 0);

	/* Enable UART0, receive, and transmit */
	bcm2835_write(UART0_CR, UART0_CR_UARTEN |
				UART0_CR_TXE |
				UART0_CR_RXE);

	return 0;
}

void early_debug_putc(unsigned char byte) {

	/* Check Flags Register */
	/* And wait until FIFO not full */
	while ( bcm2835_read(UART0_FR) & UART0_FR_TXFF ) {
	}

	/* Write our data byte out to the data register */
	bcm2835_write(UART0_DR, byte);
}

int32_t early_debug_write(const char* buffer, size_t size) {

	size_t i;

//	if (!pl011_uart_initialized) return 0;

	for ( i = 0; i < size; i++ ) {
		/* Terminal emulators expect \r\n     */
		/* But we save space by only using \n */
//		if (buffer[i]=='\n') pl011_uart_putc('\r');

		pl011_uart_putc(buffer[i]);
	}
	return i;
}


static void early_debug_put_hex_char(unsigned char x) {

	unsigned char temp;

	temp=((x>>4)&0xf);
	if (temp<10) temp+='0';
	else temp+=('A'-10);
	early_debug_putc(temp);

	temp=(x&0xf);
	if (temp<10) temp+='0';
	else temp+=('A'-10);
	early_debug_putc(temp);

}

void early_debug_dump_memory(uint32_t address, uint32_t size) {

	unsigned char *ptr;
	int i;

	ptr=(unsigned char *)address;

	for(i=0;i<size;i++) {
		if ((i&0xf)==0) {
			early_debug_putc('\r');
			early_debug_putc('\n');
		}
		early_debug_put_hex_char(*ptr);
		ptr++;
		early_debug_putc(' ');

	}

}


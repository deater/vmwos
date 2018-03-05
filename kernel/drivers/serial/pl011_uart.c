/* This is code to drive the BCM2835 PL011 UART				*/
/* As decribed in Chapter 13 of the BCM2835 ARM Peripherals Manual	*/

/* The code assumes you have a serial connector with TX/RX connected	*/
/* To GPIO pins 14 and 15						*/

#include <stddef.h>
#include <stdint.h>

#include "drivers/gpio/gpio.h"
#include "drivers/console/console_io.h"
#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/serial/serial.h"
#include "drivers/serial/pl011_uart.h"

#include "lib/delay.h"
#include "lib/printk.h"

#include "interrupts/interrupts.h"

static int pl011_uart_initialized=0;

uint32_t pl011_uart_init(struct serial_type *serial) {

	uint32_t old;

	/* Make this configurable? */
	serial->baud=115200;
	serial->bits=8;
	serial->stop=1;
	serial->parity=SERIAL_PARITY_NONE;

	/* Set up function pointers */
	serial->uart_enable_interrupts=pl011_uart_enable_interrupts;
	serial->uart_putc=pl011_uart_putc;
	serial->uart_getc=pl011_uart_getc;
	serial->uart_getc_noblock=pl011_uart_getc_noblock;
	serial->uart_interrupt_handler=pl011_uart_interrupt_handler;

	/* Disable UART */
	bcm2835_write(UART0_CR, 0x0);

	/* Setup GPIO pins 14 and 15 */
	gpio_request(14,"uart_tx");
	gpio_request(15,"uart_rx");

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
//	bcm2835_write(UART0_IBRD, 1);
//	bcm2835_write(UART0_FBRD, 40);

	/* On Pi3 (and all other Pis with recent firmware) */
	/* UART clock is now 48MHz */

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
// Memory barrier?
//{
//uint32_t dest = 0;
//__asm__ __volatile__("mcr p15,0,%0,c7,c10,5" :"=&r"(dest) : : "memory");
//}

	pl011_uart_initialized=1;

	return 0;
}

void pl011_uart_enable_interrupts(void) {
	uint32_t old;

	/* clear pending interrupts */
	bcm2835_write(UART0_ICR, 0x7FF);

	/* Set RX fifo length to 1 */
	old=bcm2835_read(UART0_IFLS);
	old&=~(0x7<<3);		/* set to 1/8 RX FIFO */
	bcm2835_write(UART0_IFLS,old);

	/* Enable receive interrupts */
	old=bcm2835_read(UART0_IMSC);
	printk("uart: previous IMSC: %x\n",old);
	old|=UART0_IMSC_RTIM;
	printk("uart: new IMSC: %x\n",old);
	bcm2835_write(UART0_IMSC,old);
	irq_enable(57);
}

void pl011_uart_putc(unsigned char byte) {

	/* Check Flags Register */
	/* And wait until FIFO not full */
	while ( bcm2835_read(UART0_FR) & UART0_FR_TXFF ) {
	}

	/* Write our data byte out to the data register */
	bcm2835_write(UART0_DR, byte);
}

int32_t pl011_uart_getc(void) {

	/* Check Flags Register */
	/* Wait until Receive FIFO is not empty */
	while ( bcm2835_read(UART0_FR) & UART0_FR_RXFE ) {
	}

	/* Read and return the received data */
	/* Note we are ignoring the top 4 error bits */

	return bcm2835_read(UART0_DR);
}

int32_t pl011_uart_getc_noblock(void) {

	/* Check Flags Register */

	/* Return -1 if Receive FIFO is empty */
	if ( bcm2835_read(UART0_FR) & UART0_FR_RXFE ) {
		return -1;
	}

	/* Read and return the received data */
	/* Note we are ignoring the top 4 error bits */

	return (bcm2835_read(UART0_DR))&0xff;
}

int32_t pl011_uart_interrupt_handler(void) {

	uint32_t ascii;

	/* read byte */
	while(1) {
		ascii=pl011_uart_getc_noblock();
		if (ascii==-1) break;

		/* Send to console */
		console_insert_char(ascii);
	}

	/* Clear receive interrupt */

	bcm2835_write(UART0_ICR,UART0_ICR_RTIC);

	return 0;
}

void pl011_uart_putc_extra(unsigned char byte,unsigned int extra) {

	/* Check Flags Register */
	/* And wait until FIFO not full */
	while ( bcm2835_read(UART0_FR) & UART0_FR_TXFF ) {
	}

	/* Write our data byte out to the data register */
	bcm2835_write(UART0_DR, byte+extra-extra);
}

#if 1

int32_t pl011_write(const char* buffer, size_t size) {

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

uint32_t old_pl011_uart_init(void) {

	/* Disable UART */
	bcm2835_write(UART0_CR, 0x0);

	/* Setup GPIO pins 14 and 15 */
	gpio_request(14,"uart_tx");
	gpio_request(15,"uart_rx");

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
	bcm2835_write(UART0_IBRD, 1);
	bcm2835_write(UART0_FBRD, 40);

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

	pl011_uart_initialized=1;

	return 0;
}

#endif

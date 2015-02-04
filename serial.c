#include <stddef.h>
#include <stdint.h>
#include "bcm2835_periph.h"
#include "mmio.h"
#include "delay.h"


void uart_init(void) {

	/* Disable UART */
	mmio_write(UART0_CR, 0x0);

	/* Setup GPIO pins 14 and 15 */

	/* Disable the pull up/down on pins 14 and 15 */
	/* See the Peripheral Manual for more info */
	/* Configure to disable pull up/down and delay for 150 cycles */
	mmio_write(GPIO_GPPUD, GPIO_GPPUD_DISABLE);
	delay(150);

	/* Pass the disable clock to GPIO pins 14 and 15 and delay*/
	mmio_write(GPIO_GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);

	/* Write 0 to GPPUDCLK0 to make it take effect */
	mmio_write(GPIO_GPPUDCLK0, 0x0);

	/* Clear pending interrupts. */
	mmio_write(UART0_ICR, 0x7FF);

	/* Set integer & fractional part of baud rate. */
	/* Divider = UART_CLOCK/(16 * Baud)            */
	/* Fraction part register = (Fractional part * 64) + 0.5 */
	/* UART_CLOCK = 3000000; Baud = 115200.        */

	/* Divider = 3000000 / (16 * 115200) = 1.627   */
	/* Integer part = 1 */
	/* Fractional part register = (.627 * 64) + 0.5 = 40.6 = 40 */
	mmio_write(UART0_IBRD, 1);
	mmio_write(UART0_FBRD, 40);

	/* Enable FIFO */
	/* And 8N1 (8 bits of data, no parity, 1 stop bit */
	mmio_write(UART0_LCRH, UART0_LCRH_FEN | UART0_LCRH_WLEN_8BIT);

	/* Mask all interrupts. */
	mmio_write(UART0_IMSC, UART0_IMSC_CTS | UART0_IMSC_RX |
				UART0_IMSC_TX | UART0_IMSC_RT |
				UART0_IMSC_FE | UART0_IMSC_PE |
				UART0_IMSC_BE | UART0_IMSC_OE);

	/* Enable UART0, receive, and transmit */
	mmio_write(UART0_CR, UART0_CR_UARTEN |
				UART0_CR_TXE |
				UART0_CR_RXE);
}

void uart_putc(unsigned char byte) {

	/* Check Flags Register */
	/* And wait until FIFO not full */
	while ( mmio_read(UART0_FR) & UART0_FR_TXFF ) {
	}

	/* Write our data byte out to the data register */
	mmio_write(UART0_DR, byte);
}

unsigned char uart_getc(void) {

	/* Check Flags Register */
	/* Wait until Receive FIFO is not empty */
	while ( mmio_read(UART0_FR) & UART0_FR_RXFE ) {
	}

	/* Read and return the received data */
	/* Note we are ignoring the top 4 error bits */

	return mmio_read(UART0_DR);
}

/* write a series of bytes to the serial port */
void uart_write(const unsigned char* buffer, size_t size) {

	size_t i;

	for ( i = 0; i < size; i++ ) {
		uart_putc(buffer[i]);
	}
}

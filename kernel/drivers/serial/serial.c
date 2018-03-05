/* This code tries to be a generic serial driver */

#include <stddef.h>
#include <stdint.h>

#include "drivers/serial/serial.h"
#include "drivers/serial/pl011_uart.h"
#include "drivers/serial/mini_uart.h"

#include "lib/errors.h"

static struct serial_type serial;

uint32_t serial_init(uint32_t type) {

	uint32_t result=0;

	if (type==SERIAL_UART_PL011) {
		result=pl011_uart_init(&serial);
	}
	else if (type==SERIAL_UART_MINI) {
		result=mini_uart_init(&serial);
	}

	if (result!=0) {
		serial.initialized=0;
		return -ENODEV;
	}

	serial.initialized=1;

	return 0;
}


/* write a series of bytes to the serial port */
uint32_t serial_write(const char* buffer, size_t size) {

	size_t i;

	if (!serial.initialized) return 0;

	for ( i = 0; i < size; i++ ) {
		/* Terminal emulators expect \r\n     */
		/* But we save space by only using \n */
		if (buffer[i]=='\n') serial.uart_putc('\r');

		serial.uart_putc(buffer[i]);
	}
	return i;
}

int32_t serial_interrupt_handler(void) {

	return serial.uart_interrupt_handler();

}

void serial_enable_interrupts(void) {

	serial.uart_enable_interrupts();

}

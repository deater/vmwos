#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "serial.h"
#include "framebuffer_console.h"

int console_write(const void *buf, size_t count) {

	int result;

	/* Write to framebuffer */
	result=framebuffer_console_write(buf, count);

	/* Write to UART */
	result=uart_write(buf, count);

	return result;

}


int console_read(const void *buf, size_t count) {

	int i;
	unsigned char *buffer=buf;

	/* Read from UART */
	for(i=0;i<count;i++) {
		buffer[i]=uart_getc();
	}

	return i;

}

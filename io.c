#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "serial.h"
#include "framebuffer_console.h"

int write(int fd, const void *buf, size_t count) {

	int result;

	/* Only handle stdout for now */
	if (fd!=1) return -1;

	/* Write to framebuffer */
	result=framebuffer_console_write(buf, count);

	/* Write to UART */
	result=uart_write(buf, count);

	return result;

}

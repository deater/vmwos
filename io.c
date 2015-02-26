#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "serial.h"

int write(int fd, const void *buf, size_t count) {

	int result;

	/* Only handle stdout for now */
	if (fd!=1) return -1;

	result=uart_write(buf, count);

	return result;

}

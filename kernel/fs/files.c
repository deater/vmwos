#include <stdint.h>
#include <stddef.h>

#include "errors.h"

#include "drivers/console/console_io.h"
#include "fs/files.h"
#include "lib/printk.h"

uint32_t close(uint32_t fd) {

	return ENOSYS;

}


uint32_t open(const char *pathname, uint32_t flags, uint32_t mode) {

	return ENOSYS;

}

uint32_t read(uint32_t fd, void *buf, uint32_t count) {

	uint32_t result;

	if (fd==0) {
		result=console_read(buf,count);
	}
	else {
		printk("Attempting to read from unsupported fd %d\n",fd);
		result=-1;
	}
	return result;
}

uint32_t write(uint32_t fd, void *buf, uint32_t count) {

	uint32_t result;

	if ((fd==1) || (fd==2)) {
		result = console_write(buf, count);
	}
	else {
		printk("Attempting to write unsupported fd %d\n",fd);
		result=-1;
	}
	return result;
}

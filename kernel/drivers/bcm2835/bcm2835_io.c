#include <stdint.h>

#include "mmio.h"

static uint32_t io_base=0x20000000;

void bcm2835_init(int type) {

	return;
}

void bcm2835_write(uint32_t address, uint32_t data) {
	mmio_write(io_base+address, data);
}

uint32_t bcm2835_read(uint32_t address) {
	return mmio_read(io_base+address);
}

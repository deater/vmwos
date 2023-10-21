#include <stdint.h>

#include "lib/mmio.h"

#include "boot/hardware_detect.h"

void bcm2835_write(uint32_t address, uint32_t data) {
	mmio_write(io_base+address, data);
}

uint32_t bcm2835_read(uint32_t address) {
	return mmio_read(io_base+address);
}

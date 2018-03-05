#include <stdint.h>

#include "lib/mmio.h"

#if defined(ARMV7)
#define			IO_BASE		0x3f000000
#elif defined(ARM1176)
#define			IO_BASE		0x20000000
#else
#error "UNKNOWN ARCHITECTURE"
#endif

void bcm2835_write(uint32_t address, uint32_t data) {
	mmio_write(IO_BASE+address, data);
}

uint32_t bcm2835_read(uint32_t address) {
	return mmio_read(IO_BASE+address);
}

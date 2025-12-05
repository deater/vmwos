#include <stdint.h>

#include "lib/memory_barriers.h"
#include "lib/mmio.h"


#include "boot/hardware_detect.h"

void bcm2835_write(uint32_t address, uint32_t data) {
	mmio_write(io_base+address, data);
}

uint32_t bcm2835_read(uint32_t address) {
	return mmio_read(io_base+address);
}

/* "BCM2835 ARM Peripherals" document page 7 says the BCM2835 */
/* requires a memory write berrier before first write to a peripheral */
/* and a memory read barrier after the last read in order to keep */
/* data in order on the AXI bus. */
void bcm2835_peripheral_entry(void) {
	dsb();
}

void bcm2835_peripheral_exit(void) {
	dmb();
}

/* Based on code from https://github.com/rsta2/circle/ */

#include <stdint.h>
#include "lib/mmio.h"
#include "interrupts/gic-400.h"
#include "lib/printk.h"


uint32_t num_interrupts = 0;

/* Initialize gic400 interrupt controller as found on a Raspberry pi4 */
/* Note: this just does the bare minimum to get interrupts delivered */

uint32_t gic400_init(void* interrupt_controller_base) {

	int n;

    /* Disable the controller so we can configure it before it passes any
       interrupts to the CPU */
	mmio_write(GICD_CTLR, GICD_CTLR_DISABLE);

	/* Get the number of interrupt lines implemented */
	/* (number of registers * 32) in the GIC400 controller */
	num_interrupts=(mmio_read(GICD_TYPE)&0xf)*32;
//	num_interrupts=80;

	/* disable, ack, and deactivate all */
	for (n = 0; n < num_interrupts/32; n++) {
		mmio_write(GICD_ICENABLER0 + 4*n, 0xffffffffUL);
		mmio_write(GICD_ICPENDR0   + 4*n, 0xffffffffUL);
		mmio_write(GICD_ICACTIVER0 + 4*n, 0xffffffffUL);
	}

	/* direct all interrupts to core 0 with default priority */
	for (n = 0; n < num_interrupts/4; n++) {

		/* Priority default is 0xA0 */
		mmio_write(GICD_IPRIORITYR0 + 4*n,
				GICD_IPRIORITYR_DEFAULT |
				GICD_IPRIORITYR_DEFAULT << 8 |
				GICD_IPRIORITYR_DEFAULT << 16 |
				GICD_IPRIORITYR_DEFAULT << 24);

		mmio_write(GICD_ITARGETSR0 + 4*n,
				GICD_ITARGETSR_CORE0 |
				GICD_ITARGETSR_CORE0 << 8 |
				GICD_ITARGETSR_CORE0 << 16 |
				GICD_ITARGETSR_CORE0 << 24);
	}

	/* set all interrupts to level triggered */
	for (n = 0; n < num_interrupts/16; n++) {
		mmio_write (GICD_ICFGR0 + 4*n, 0);
	}

	/* Enable PPI interrupts */
//	mmio_write(GICD_ISENABLER0,0xffffffffUL);
	mmio_write(GICD_ISENABLER0,0x80000000UL);

	/* enable */
	mmio_write(GICD_CTLR, GICD_CTLR_ENABLE);

	/* initialize core 0 CPU interface: */
	mmio_write(GICC_PMR, GICC_PMR_PRIORITY);
	mmio_write(GICC_CTLR, GICC_CTLR_ENABLE);

	return 0;
}

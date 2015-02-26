#include <stdint.h>
#include "mmio.h"
#include "bcm2835_periph.h"

int timer_init(void) {

	mmio_write(IRQ_ENABLE_BASIC_IRQ,IRQ_ENABLE_BASIC_IRQ_ARM_TIMER);

	/* Timer frequency = Clk/256 * 0x400 */
	mmio_write(TIMER_LOAD,0x400);

	/* Setup the ARM Timer */
	mmio_write(TIMER_CONTROL,
		TIMER_CONTROL_32BIT |
		TIMER_CONTROL_ENABLE |
		TIMER_CONTROL_INT_ENABLE |
		TIMER_CONTROL_PRESCALE_256);

}


#include <stdint.h>
#include "bcm2835_periph.h"
#include "led.h"
#include "mmio.h"

int led_init(void) {

	uint32_t old;

	/* Enable GPIO for the ACT LED */
	/* On the Model B this is 16, on the B+ this is 47 */

	old=mmio_read(GPIO_GPFSEL1);
	old |= (1<<18);
	mmio_write(GPIO_GPFSEL1, old);

	return 0;

}

int led_on(void) {

	/* Write 1 to clear the GPIO (turn on the LED) */

	mmio_write(GPIO_GPCLR0, 1 << 16);

	return 0;
}

int led_off(void) {

	/* Write 1 to set the GPIO (turn off the LED) */

	mmio_write(GPIO_GPSET0, 1 << 16);

	return 0;
}


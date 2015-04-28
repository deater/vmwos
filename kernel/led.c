#include <stdint.h>
#include "bcm2835_periph.h"
#include "led.h"
#include "mmio.h"
#include "hardware.h"

int led_init(void) {

	uint32_t old;

	/* Enable GPIO for the ACT LED */
	/* On the Model B this is 16, on the B+ this is 47 */

	if ((hardware_type==RPI_MODEL_BPLUS) ||
	 	(hardware_type==RPI_MODEL_APLUS)) {
		old=mmio_read(GPIO_GPFSEL4);
		old |= (1<<21);
		mmio_write(GPIO_GPFSEL4, old);
	}
	else {
		/* Assume Model B */
		old=mmio_read(GPIO_GPFSEL1);
		old |= (1<<18);
		mmio_write(GPIO_GPFSEL1, old);
	}

	return 0;

}

int led_on(void) {

	/* LED on MODELB is active low, on BPLUS active high */

	if ((hardware_type==RPI_MODEL_BPLUS) ||
		(hardware_type==RPI_MODEL_APLUS)) {
		mmio_write(GPIO_GPSET1, 1 << 15);
	}
	else {
		mmio_write(GPIO_GPCLR0, 1 << 16);
	}

	return 0;
}

int led_off(void) {

	/* LED on MODELB is active low, on BPLUS active high */

	if ((hardware_type==RPI_MODEL_BPLUS) ||
		(hardware_type==RPI_MODEL_APLUS)) {
		mmio_write(GPIO_GPCLR1, 1 << 15);
	}
	else {
		mmio_write(GPIO_GPSET0, 1 << 16);
	}

	return 0;
}


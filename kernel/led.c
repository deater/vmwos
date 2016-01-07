#include <stdint.h>
#include "bcm2835_periph.h"
#include "led.h"
#include "mmio.h"
#include "hardware.h"
#include "gpio.h"

int led_init(void) {

	uint32_t old;

	/* Enable GPIO for the ACT LED */
	/* On the Model B this is 16, on the B+ this is 47 */

	if ((hardware_type==RPI_MODEL_BPLUS) ||
	 	(hardware_type==RPI_MODEL_APLUS)) {
		gpio_request(47,"act_led");
		gpio_direction_output(47);
	}
	else {
		/* Assume Model B */
		gpio_request(16,"act_led");
		gpio_direction_output(16);
	}

	return 0;

}

int led_on(void) {

	/* LED on MODELB is active low, on BPLUS active high */

	if ((hardware_type==RPI_MODEL_BPLUS) ||
		(hardware_type==RPI_MODEL_APLUS)) {
		gpio_set_value(47,1);
	}
	else {
		gpio_set_value(16,0);
	}

	return 0;
}

int led_off(void) {

	/* LED on MODELB is active low, on BPLUS active high */

	if ((hardware_type==RPI_MODEL_BPLUS) ||
		(hardware_type==RPI_MODEL_APLUS)) {
		gpio_set_value(47,0);
	}
	else {
		gpio_set_value(16,1);
	}

	return 0;
}


#include <stdint.h>

#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/led/led.h"

#include "boot/hardware_detect.h"
#include "drivers/gpio/gpio.h"
#include "lib/printk.h"
#include "lib/errors.h"

/* Default for Model B */
static uint32_t led_gpio=16;
static uint32_t led_active_low=1;

static uint32_t led_configured=0;

/* Enable GPIO for the ACT LED */
int led_init(void) {

	led_configured=1;

	/* On the Model B this is 16, on the B+/A+ this is 47 */
	/* Model B is active low, B+/A+ active high */
	if ((hardware_get_type()==RPI_MODEL_B) ||
	 	(hardware_get_type()==RPI_MODEL_A)) {
		led_gpio=16;
		led_active_low=1;
	}
	else if ((hardware_get_type()==RPI_MODEL_BPLUS) ||
	 	(hardware_get_type()==RPI_MODEL_APLUS) ||
		(hardware_get_type()==RPI_MODEL_2B)) {
		led_gpio=47;
		led_active_low=0;
	}
	else if ((hardware_get_type()==RPI_MODEL_3B) ||
		(hardware_get_type()==RPI_MODEL_3BPLUS)) {
		/* Not ACT LED, just one we have hooked to GPIO18 */
		led_gpio=18;
		led_active_low=0;
	}
	else {
		printk("Unknown hardware type, not enabling LED\n");
		led_configured=0;
		return ENODEV;
	}

	gpio_request(led_gpio,"act_led");
	gpio_direction_output(led_gpio);

	printk("Starting heartbeat LED on GPIO%d\n",led_gpio);

	return 0;

}

int led_on(void) {

	if (!led_configured) return 0;

	if (led_active_low) {
		gpio_set_value(led_gpio,0);
	}
	else {
		gpio_set_value(led_gpio,1);
	}

	return 0;
}

int led_off(void) {

	if (!led_configured) return 0;

	if (led_active_low) {
		gpio_set_value(led_gpio,1);
	}
	else {
		gpio_set_value(led_gpio,0);
	}

	return 0;
}


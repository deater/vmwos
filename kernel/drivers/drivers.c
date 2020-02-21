#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"

#include "drivers/drivers.h"

#include "drivers/console/console_io.h"
#include "drivers/framebuffer/framebuffer.h"
#include "drivers/framebuffer/framebuffer_console.h"
#include "drivers/led/led.h"
#include "drivers/timer/timer.h"
#include "drivers/keyboard/ps2-keyboard.h"
#include "drivers/serial/serial.h"
#include "drivers/random/bcm2835_rng.h"
#include "drivers/thermal/thermal.h"
#include "drivers/pmu/arm-pmu.h"


void drivers_init_all(void) {

	uint32_t framebuffer_width=640,framebuffer_height=480;
	uint32_t temperature;

	/**************************/
	/* Device Drivers	  */
	/**************************/

	/* Set up console */
	console_init();

	/* Set up ACT LED */
	led_init();

	/* Set up timer */
	timer_init();

	/* Set up keyboard */
	ps2_keyboard_init();

	/* Enable the Framebuffer */
#if 0
	if (atag_info.framebuffer_x!=0) {
		framebuffer_width=atag_info.framebuffer_x;
	}
	if (atag_info.framebuffer_y!=0) {
		framebuffer_height=atag_info.framebuffer_y;
	}
#endif

	framebuffer_init(framebuffer_width,framebuffer_height,24);
#if 0
	framebuffer_console_init();
#endif

	serial_enable_interrupts();

	/* Enable HW random number generator */
	bcm2835_rng_init();

	/* Check temperature */
	temperature=thermal_read();
	printk("CPU Temperature: %dC, %dF\n",
		temperature/1000,
		((temperature*9)/5000)+32);

	/* Start HW Perf Counters */
	pmu_init();

}

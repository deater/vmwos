#include <stdint.h>
#include <stddef.h>

#include "lib/smp.h"

#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/led/led.h"
#include "time/time.h"
#include "processes/waitqueue.h"
#include "processes/process.h"

/* This is the standard ARM timer */
/* Described in Chapter 14 of the BCM2835 ARM Peripherals Document */
/* It is similar but not exactly the same as an ARM SP804 Timer */

/* The BCM2835 supports a number of other types of timer too */

/* Note that the timer we use is based on the APB clock which */
/* may change frequency in power-saving modes, so maybe not the */
/* best clock to be using */

/* We set the timer to overflow at roughly 64Hz */

static struct wait_queue_t timer_wait_queue = {
	NULL
};

static uint32_t timer_next_wakeup=0;


int timer_init(void) {

	uint32_t old;

	/* Disable the clock before changing config */
	old=bcm2835_read(TIMER_CONTROL);
	old&=~(TIMER_CONTROL_ENABLE|TIMER_CONTROL_INT_ENABLE);
	bcm2835_write(TIMER_CONTROL,old);

	/* Timer is based on the APB bus clock which is 250MHz on Rasp-Pi */

	/* First we scale this down to 1MHz using the pre-divider */
	/* We want to /250.  The pre-divider adds one, so 249 = 0xf9 */
	bcm2835_write(TIMER_PREDIVIDER,0xf9);

	/* We enable the /256 prescalar */
	/* So final frequency = 1MHz/256/61 = 64.04 Hz */

	/* The value is loaded into TIMER_LOAD and then it counts down */
	/* and interrupts once it hits zero. */
	/* Then this value is automatically reloaded and restarted */

	bcm2835_write(TIMER_LOAD,61);

	/* Enable the timer in 32-bit mode, enable interrupts */
	/* And pre-scale the clock down by 256 */
	bcm2835_write(TIMER_CONTROL,
		TIMER_CONTROL_32BIT |
		TIMER_CONTROL_ENABLE |
		TIMER_CONTROL_INT_ENABLE |
		TIMER_CONTROL_PRESCALE_256);

	/* Enable timer interrupt */
	bcm2835_write(IRQ_ENABLE_BASIC_IRQ,IRQ_ENABLE_BASIC_IRQ_ARM_TIMER);

	return 0;

}

/* externally visible variables */
uint32_t blinking_enabled=1;
uint32_t tick_counter=0;

void timer_interrupt_handler(void) {

	static int lit = 0;

	/* Clear the ARM Timer interrupt */

	bcm2835_write(TIMER_IRQ_CLEAR,0x1);
	tick_counter++;

	if ((timer_next_wakeup) && (tick_counter>timer_next_wakeup)) {
		timer_next_wakeup=0;
		/* Wake anyone waiting on a timer */
	        wait_queue_wake(&timer_wait_queue);
	}

	if (blinking_enabled) {
		lit++;
		/* Blink the LED at 1HZ */
		if (lit==TIMER_HZ/2) {
			led_off();
		}
		if (lit==TIMER_HZ) {
			led_on();
			lit=0;
		}
	}

}

void timer_sleep_until(uint32_t time) {

	if (timer_next_wakeup==0) {
		timer_next_wakeup=time;
	}
	else {
		if (time<timer_next_wakeup) {
			timer_next_wakeup=time;
		}
	}
	wait_queue_add(&timer_wait_queue,current_proc[get_cpu()]);
}

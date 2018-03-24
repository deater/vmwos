#include <stdint.h>
#include "time/time.h"

/* In seconds */
uint32_t time_since_boot(void) {

	return tick_counter/TIMER_HZ;

}

uint32_t ticks_since_boot(void) {

	return tick_counter;

}

int32_t clock_gettime(uint32_t clkid, struct timespec *t) {

	uint32_t counter=tick_counter;
	uint32_t seconds,remainder;

	seconds=counter/TIMER_HZ;
	remainder=counter-(seconds*TIMER_HZ);

	remainder*=1000000;
	remainder/=64;		// us
	remainder*=1000;	// ns

	t->seconds=seconds;
	t->ns=remainder;

	return 0;
}

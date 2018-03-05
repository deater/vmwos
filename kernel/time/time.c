#include <stdint.h>
#include "time/time.h"

/* In seconds */
uint32_t time_since_boot(void) {

	return tick_counter/TIMER_HZ;

}

uint32_t ticks_since_boot(void) {

	return tick_counter;

}

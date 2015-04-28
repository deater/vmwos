#include "time.h"

unsigned int time_since_boot(void) {

	return tick_counter/TIMER_HZ;

}

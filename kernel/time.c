#include <stdint.h>
#include "time.h"

uint32_t time_since_boot(void) {

	return tick_counter/TIMER_HZ;

}

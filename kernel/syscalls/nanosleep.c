#include <stdint.h>

#include "time/time.h"
#include "drivers/timer/timer.h"
#include "syscalls/nanosleep.h"

int32_t nanosleep(const struct timespec *req, struct timespec *rem) {

	uint32_t ticks_to_sleep=0;
	uint32_t current_time,end_time;
	/* We ignore rem for now */

	ticks_to_sleep=((req->ns/1000000)*TIMER_HZ)/1000;
	ticks_to_sleep+=(req->seconds*64);

	current_time=ticks_since_boot();
	end_time=current_time+ticks_to_sleep;

	/* Sleep until enough ticks have passed */
	while(ticks_since_boot() <= end_time) {
		timer_sleep_until(end_time);
	}

	return 0;
}


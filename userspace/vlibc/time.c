#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"
#include "vlibc.h"

int32_t sleep(uint32_t seconds) {

	struct timespec t;

	t.tv_sec=seconds;
	t.tv_nsec=0;

	nanosleep(&t, NULL);

	return 0;
}

int32_t usleep(uint32_t usecs) {

	struct timespec t;

	t.tv_sec=usecs/1000000;
	t.tv_nsec=(usecs-(t.tv_sec*1000000))*1000;

	nanosleep(&t, NULL);

	return 0;
}

int time(int *t) {

	int our_time;

	our_time=sys_time();

	if (t!=NULL) *t=our_time;

	return our_time;
}

char *time_pretty(int32_t time, char *buffer, int32_t size) {

	int32_t days=0,hours=0,minutes=0,seconds=0;

	buffer[0]='\0';

	if (time>(24*60*60)) {
		days=time/(24*60*60);
		time-=(days*24*60*60);
	}

	if (time>(60*60)) {
		hours=time/(60*60);
		time-=(hours*60*60);
	}

	if (time>60) {
		minutes=time/60;
		time-=(minutes*60);
	}
	seconds=time;

	if (days) {
		sprintf(buffer,"%dd %dh %dm %ds",days,hours,minutes,seconds);
	} else if (hours) {
		sprintf(buffer,"%dh %dm %ds",hours,minutes,seconds);
	}
	else if (minutes) {
		sprintf(buffer,"%dm %ds",minutes,seconds);
	}
	else {
		sprintf(buffer,"%ds",seconds);
	}

	return buffer;
}

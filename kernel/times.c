#include <stdint.h>

#include "times.h"

int32_t times(struct tms *buf) {


	buf->tms_utime=0;
	buf->tms_stime=0;
	buf->tms_cutime=0;
	buf->tms_cstime=0;
}

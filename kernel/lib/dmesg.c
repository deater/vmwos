#include <stdint.h>

#include "lib/errors.h"
#include "lib/dmesg.h"
#include "lib/memcpy.h"

#define DMESG_SIZE	4096

static char dmesg_buffer[DMESG_SIZE];
static int32_t dmesg_head=0,dmesg_tail=0;

int32_t dmesg_append(char *buf, int32_t length) {

	int32_t old_dmesg_tail,wrapped=0;

	old_dmesg_tail=dmesg_tail;

	if (length>=DMESG_SIZE) return -1;

	dmesg_tail+=length;

	if (dmesg_tail>DMESG_SIZE) {
		/* copy part 1 */
		memcpy(&dmesg_buffer[old_dmesg_tail],buf,
			DMESG_SIZE-old_dmesg_tail);
		if (old_dmesg_tail<dmesg_head) wrapped=1;

		dmesg_tail-=DMESG_SIZE;

		/* copy part 2 */
		memcpy(&dmesg_buffer[0],buf+(DMESG_SIZE-old_dmesg_tail),
			dmesg_tail);
		if (dmesg_tail>dmesg_head) wrapped=1;


	}
	else {
		memcpy(&dmesg_buffer[old_dmesg_tail],buf,length);
		if ((dmesg_head>old_dmesg_tail) &&
			(dmesg_head<dmesg_tail)) wrapped=1;
	}

	/* adjust head if necessary */
	if (wrapped) {
		dmesg_head=dmesg_tail+1;
		if (dmesg_head>DMESG_SIZE) dmesg_head-=DMESG_SIZE;
	}

	return 0;
}

int32_t dmesg_syscall(int32_t cmd, char *buf) {

	int32_t result,length=0;

	switch(cmd) {
		case SYSLOG_ACTION_READ_ALL:
			if (dmesg_head>dmesg_tail) {
				/* head to end */
				memcpy(buf,&dmesg_buffer[dmesg_head],
					DMESG_SIZE-dmesg_head);
				length+=(DMESG_SIZE-dmesg_head);

				/* wrap, copy tail */
				memcpy(buf+length,&dmesg_buffer[0],
					dmesg_tail);
				length+=dmesg_tail;

			}
			else if (dmesg_head<dmesg_tail) {
				memcpy(buf,&dmesg_buffer[dmesg_head],
					dmesg_tail-dmesg_head);
				length=(dmesg_tail-dmesg_head);
			}
			return length;

		case SYSLOG_ACTION_SIZE_BUFFER:
			return DMESG_SIZE;
		default:
			result=-EINVAL;
			break;
	}

	return result;
}

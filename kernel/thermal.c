/* Read temperature from BCM2835			*/
/* Uses the mailbox interface to talk to the firmware	*/
/* Based on rpi-linux/drivers/hwmon/bcm2835-hwmon.c	*/
/* See also https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface */

#include <stddef.h>
#include <stdint.h>

#include "printk.h"
#include "mmio.h"

#include "mailbox.h"

#include "string.h"


#define VC_TAG_GET_TEMP 0x00030006
#define VC_TAG_GET_MAX_TEMP 0x0003000A

/* tag part of the message */
struct vc_msg_tag {
	uint32_t tag_id;	/* the tag ID for the temperature */
	uint32_t buffer_size;	/* size of the buffer (should be 8) */
	uint32_t request_code;	/* identifies message as a request (should be 0) */
	uint32_t id;		/* extra ID field (should be 0) */
	uint32_t val;		/* returned value of the temperature */
};

/* message structure to be sent to videocore */
struct vc_msg {
	uint32_t msg_size;		/* sizeof(struct vc_msg) */
	uint32_t request_code;		/* holds various information like the success and number of bytes returned (refer to mailboxes wiki) */
	struct vc_msg_tag tag;		/* the tag structure above */
	uint32_t end_tag;		/* an end identifier, should be set to NULL */
};


int thermal_read(void) {

	struct vc_msg msg  __attribute__ ((aligned(16)));

	uint32_t temp=0,result;

	printk("TEMP: trying to read\n");

	/* Clear the struct */
	memset(&msg, 0, sizeof(msg));

	/* We want to read temp. */
	/* TODO: add MAX_TEMP support? */
	msg.tag.tag_id = VC_TAG_GET_TEMP;
	msg.msg_size = sizeof(msg);
	msg.tag.buffer_size = 8;

	/* send the message */
	result = mailbox_write((unsigned int)(&msg),
				MAILBOX_CHAN_PROPERTY);

	if (result<0) printk("THERM: Mailbox write problem\n");

	result=mailbox_read(MAILBOX_CHAN_PROPERTY);

	/* check if it was all ok and return the rate in milli degrees C */
	if (msg.request_code & 0x80000000) {
		temp = (uint32_t)msg.tag.val;
	}
	else {
		printk("THERM: Failed to get temperature! (%d,%x)\n",
			result,msg.request_code);
	}

	printk("Temp = %d mC\n",temp);

	return temp;
}

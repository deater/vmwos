/*
 * bcm2835_rng.c -- vmwOS driver for raspberry pi random number generator
 *
 *	by Vince Weaver <vincent.weaver _at_ maine.edu>
 *
 *	Based on the bcm2708-rng.c file from Broadcom
 */


#include <stdint.h>
#include "lib/printk.h"
#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/bcm2835/bcm2835_periph.h"


/* RNG Enable */
#define RNG_RBGEN	0x1
/* Double speed, less random mode (???) */
#define RNG_RBG2X	0x2

/* the initial numbers generated are "less random" so will be discarded */
#define RNG_WARMUP_COUNT	0x40000


int bcm2835_rng_init(void) {

	/* Set up warm-up count */
	bcm2835_write(RNG_STATUS, RNG_WARMUP_COUNT);

	/* Enable RNG */
	bcm2835_write(RNG_CTRL, RNG_RBGEN);

	return 0;
}

int bcm2835_rng_exit(void) {

	/* Disable RNG */
	bcm2835_write(RNG_CTRL, 0);

	return 0;
}

int32_t bcm2835_rng_read(uint32_t *value) {

	uint32_t count;
	uint32_t timeout=0;

	/* wait for a random number to be in fifo */
	while(1) {
		count = bcm2835_read(RNG_STATUS)>>24;
		if (count!=0) break;
		timeout++;
		if (timeout>100) return -1;
	}

	/* read the random number */
	*value = bcm2835_read(RNG_DATA);
	return 4;
}

/* Raspberry Pi Audio driver */
/* which is PWM GPIO40/45 and a filter connected to the 3.5mm jack */

#include <stddef.h>
#include <stdint.h>

#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/bcm2835/bcm2835_io.h"

uint32_t audio_pwm_init(void) {

	/* Set GPIO40 and GPIO45 to ALT0 (PWM) */
	/* FIXME: use the proper GPIO interface for this */
	bcm2835_write(GPIO_GPFSEL4, (0x4 << 0) + (0x4 << 15));
				// 40		45

	/* Set the PWM clock */
	bcm2835_write(CM_PWMDIV, CM_PASSWORD + 0x2000);
	bcm2835_write(CM_PWMCTL,
			CM_PASSWORD + CM_CLOCK_ENABLE + CM_SRC_OSCILLATOR);

	/* Set the range of the PWM */
	/* 0xc8 = 8-bit 48kHz */
	bcm2835_write(PWM0_RANGE, 0xc8);
	bcm2835_write(PWM1_RANGE, 0xc8);

	/* Update the control register */
	bcm2835_write(PWM_CONTROL,
		PWM_CONTROL_USE_FIFO1 + PWM_CONTROL_ENABLE1 +
		PWM_CONTROL_USE_FIFO0 + PWM_CONTROL_ENABLE0 +
		PWM_CONTROL_CLEAR_FIFO);

	return 0;
}

/* write the data */
int32_t audio_pwm_write(uint8_t *buffer, uint32_t len) {

	int i=0;
	uint32_t status,data;

	/* Stop if FIFO full */
	while(1) {

		data=20*(i&0x1f);

fifo_full:
		status=bcm2835_read(PWM_STATUS);
		if (status&PWM_STATUS_FULL) goto fifo_full;

		/* Data is interleaved in same FIFO */

		/* Channel 0 */
		bcm2835_write(PWM_FIFO_IN, data);
		/* Channel 1 */
		bcm2835_write(PWM_FIFO_IN, data);

		i++;

		if (i>len) break;

	}

	return i;
}


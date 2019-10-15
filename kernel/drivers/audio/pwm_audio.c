/* Raspberry Pi Audio driver */
/* which is PWM GPIO40/45 and a filter connected to the 3.5mm jack */

/* For example see https://github.com/PeterLemon/RaspberryPi/blob/master/Sound/PWM/14Bit/44100Hz/Mono/DMA/kernel.asm */

#include <stddef.h>
#include <stdint.h>

#include "drivers/gpio/gpio.h"
#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/bcm2835/bcm2835_io.h"



#include "Sample.bin.short.h"

static uint32_t  __attribute__ ((aligned (32))) cb_struct[8] = {
	DMA_TI_DEST_DREQ  | DMA_TI_PERMAP_5  | DMA_TI_SRC_INC,
					// DMA Transfer Information
	(uint32_t)Sample_bin_short,	// DMA Source Address
	0x7E000000+PWM_BASE+PWM_FIFO_IN,// DMA Destination Address
	524288,				// DMA Transfer Length
	0,				// DMA 2D Mode Stride
	(uint32_t)&cb_struct,		// DMA Next Control Block Address
	0,
	0,
};

uint32_t audio_pwm_init(void) {

	int i;
	/* change data from 16 to 14 bit */
	for(i=0;i<Sample_bin_short_len;i++) {
		Sample_bin_short[i]>>=2;
	}

	/* Setup GPIO pins 14 and 15 */
	gpio_request(40,"pwm0");
	gpio_request(45,"pwm1");

	/* Set GPIO40 and GPIO45 to ALT0 (PWM) */
	gpio_function_select(40,GPIO_GPFSEL_ALT0);
	gpio_function_select(45,GPIO_GPFSEL_ALT0);

	/* Set the PWM clock */
	/* Bits 0..11 Fractional Part Of Divisor = 0 */
	/* Bits 12..23 Integer Part Of Divisor = 2 */
	bcm2835_write(CM_PWMDIV, CM_PASSWORD + 0x2000);
	bcm2835_write(CM_PWMCTL,
			CM_PASSWORD + CM_CLOCK_ENABLE + CM_SRC_OSCILLATOR);

	/* Set the range of the PWM */
	/*   0xc8 = 8-bit 48kHz */
	/* 0x2C48  14bit 44100Hz Mono */
	bcm2835_write(PWM0_RANGE, 0x2c48);
	bcm2835_write(PWM1_RANGE, 0x2c48);

	/* Update the control register */
	bcm2835_write(PWM_CONTROL,
		PWM_CONTROL_USE_FIFO1 + PWM_CONTROL_ENABLE1 +
		PWM_CONTROL_USE_FIFO0 + PWM_CONTROL_ENABLE0 +
		PWM_CONTROL_CLEAR_FIFO);

	/* Set PWM DMA Enable bit */
	/* Bits 0..7 DMA Threshold For DREQ Signal = 1 */
	/* Bits 8..15 DMA Threshold For PANIC Signal = 0 */
	bcm2835_write(PWM_DMAC,PWM_DMAC_ENAB+0x1);

	/* Set DMA Channel 0 Enable Bit */
	bcm2835_write(DMA_ENABLE,DMA_EN0);

	/* Set Control Block Data Address To DMA Channel 0 Controller */
	bcm2835_write(DMA0_BASE+DMA_CONBLK_AD,(uint32_t)&cb_struct);

	/* Start DMA */
	bcm2835_write(DMA0_BASE+DMA_CS,DMA_CS_DMA_ACTIVE);

	printk("Sound enabled\n");

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

int32_t audio_beep(void) {
	audio_pwm_write(NULL,1000);
	return 0;
}



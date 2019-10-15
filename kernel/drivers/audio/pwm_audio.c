/* Raspberry Pi Audio driver */
/* which is PWM GPIO40/45 and a filter connected to the 3.5mm jack */

/* For example see https://github.com/PeterLemon/RaspberryPi/blob/master/Sound/PWM/14Bit/44100Hz/Mono/DMA/kernel.asm */

#include <stddef.h>
#include <stdint.h>

#include "drivers/gpio/gpio.h"
#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/bcm2835/bcm2835_io.h"
#include "lib/printk.h"


#include "goodwork.h"

struct dma_control_structure {
	uint32_t	transfer_information;
	uint32_t	source_address;
	uint32_t	destination_address;
	uint32_t	transfer_length;
	uint32_t	twod_stride;
	uint32_t	next_control_block;
	uint32_t	padding0;
	uint32_t	padding1;
};

static struct dma_control_structure
	__attribute__ ((aligned (32))) cb_struct1;
static struct dma_control_structure
	__attribute__ ((aligned (32))) cb_struct2;


static uint32_t __attribute__ ((aligned(16))) dma_buffer_0[0x2000];
static uint32_t __attribute__ ((aligned(16))) dma_buffer_1[0x2000];


uint32_t audio_pwm_init(void) {

	int i;
	/* copy audio into buffers */
	/* Input is 8-bit, dma_buffer is 64 bit */
	for(i=0;i<0x2000;i++) {
		dma_buffer_0[i]=goodwork_bin[i];
		dma_buffer_1[i]=goodwork_bin[i+0x2000];
	}

	/* setup csbs */
	cb_struct1.transfer_information=0x00050141;
	cb_struct1.source_address=(uint32_t)&dma_buffer_0;
	cb_struct1.destination_address=0x7E20C018;
	cb_struct1.transfer_length=0x8000;
	cb_struct1.twod_stride=0;
	cb_struct1.next_control_block=(uint32_t)&cb_struct2;
	cb_struct1.padding0=0;
	cb_struct1.padding1=0;

	/* setup csbs */
	cb_struct2.transfer_information=0x00050141;
	cb_struct2.source_address=(uint32_t)&dma_buffer_1;
	cb_struct2.destination_address=0x7E20C018;
	cb_struct2.transfer_length=0x8000;
	cb_struct2.twod_stride=0;
	cb_struct2.next_control_block=(uint32_t)&cb_struct2;
	cb_struct2.padding0=0;
	cb_struct2.padding1=0;


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
//	bcm2835_write(CM_PWMCTL,
//			CM_PASSWORD + CM_CLOCK_ENABLE + CM_SRC_OSCILLATOR);
	bcm2835_write(CM_PWMCTL,
			CM_PASSWORD + 0x16);

	/* Set the range of the PWM */
	/*   0xc8 = 8-bit 48kHz */
	/* 0x2C48  14bit 44100Hz Mono */
	bcm2835_write(PWM0_RANGE, 0x2c48);
	bcm2835_write(PWM1_RANGE, 0x2c48);

	/* Update the control register */
	bcm2835_write(PWM_CONTROL,0x2161);
//		PWM_CONTROL_USE_FIFO1 + PWM_CONTROL_ENABLE1 +
//		PWM_CONTROL_USE_FIFO0 + PWM_CONTROL_ENABLE0 +
//		PWM_CONTROL_CLEAR_FIFO);

	/* Set PWM DMA Enable bit */
	/* Bits 0..7 DMA Threshold For DREQ Signal = 1 */
	/* Bits 8..15 DMA Threshold For PANIC Signal = 0 */
	bcm2835_write(PWM_DMAC,PWM_DMAC_ENAB+0x1);

	/* Set DMA Channel 0 Enable Bit */
	bcm2835_write(DMA_ENABLE,DMA_EN0);

	/* Set Control Block Data Address To DMA Channel 0 Controller */
	bcm2835_write(DMA0_BASE+DMA_CONBLK_AD,
		(uint32_t)&cb_struct1);

	/* Start DMA */
	bcm2835_write(DMA0_BASE+DMA_CS,DMA_CS_DMA_ACTIVE);

	printk("Sound enabled\n");

	return 0;
}

/* write the data */
int32_t audio_pwm_write(uint8_t *buffer, uint32_t len) {

#if 0
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
#endif
	return 0;
}

int32_t audio_beep(void) {

	uint32_t status;

	status=bcm2835_read(PWM_STATUS);
	audio_pwm_write(NULL,1000);

	printk("Status: %x\n",status);

	return 0;
}



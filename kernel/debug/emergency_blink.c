/* Blinks GPIO18, to be used when all other debugging fails */

#include <stdint.h>

/* Values from the BCM2835-ARM-Peripherals.pdf manual */
//#define IO_BASE	0x20000000

#define IO_BASE	0x3f000000
//#define IO_BASE	0x20000000


#define GPIO_BASE	(IO_BASE+0x200000)

#define GPIO_GPFSEL0	0
#define GPIO_GPFSEL1	1
#define GPIO_GPFSEL2	2
#define GPIO_GPFSEL3	3
#define GPIO_GPFSEL4	4

#define GPIO_GPSET0	7
#define GPIO_GPSET1	8

#define GPIO_GPCLR0	10
#define GPIO_GPCLR1	11

/* Reference to the GPIO space */
/* An array of 32-bit integers */
volatile uint32_t *gpio;

/* If we make this static, the C compiler will optimize it away */
static void __attribute__ ((noinline)) delay(int length) {

	volatile int i;

	/* the asm(""); keeps gcc from optimizing away */
	for(i=0;i<length;i++) asm("");

}

/* main function */
void emergency_blink(void) {

	/* Point to the physical GPIO region */
	gpio = (uint32_t *)GPIO_BASE;

	/* Enable GPIO for the ACT LED */
	/* On the Model B this is 16, on the B+ this is 47 */

	/* We want GPIO18 */

//#ifdef PI_BLUS
//	gpio[GPIO_GPFSEL4] |= (1 << 21);
//#else
//	gpio[GPIO_GPFSEL1] |= (1 << 18);
//#endif

	/* bit 24 of GPFSEL1 */
	gpio[GPIO_GPFSEL1] |= (1 << 24);


	while(1) {

		/* Write 1 to clear the GPIO (turn on the LED) */

//#ifdef PI_BPLUS
//		gpio[GPIO_GPCLR1] = (1 << 15);
//#else
//		gpio[GPIO_GPCLR0] = (1 << 16);
//#endif

		gpio[GPIO_GPCLR0] = (1 << 18);

		/* delay */
		delay(0x3f0000);

		/* Write 1 to set the GPIO (turn off the LED) */

//#ifdef PI_BPLUS
//		gpio[GPIO_GPSET1] = (1 << 15);
//#else
//		gpio[GPIO_GPSET0] = (1 << 16);
//#endif

		gpio[GPIO_GPSET0] = (1 << 18);

		/* delay */
		delay(0x3f0000);

	}


	/* Hang forever; there is no OS to return to! */
	while(1) {
	}

	return;
}

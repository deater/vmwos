#include <stdint.h>

/* Values from the BCM2835-ARM-Peripherals.pdf manual */
#define GPIO_BASE	0x20200000

#define GPIO_GPFSEL1	1
#define GPIO_GPSET0	7
#define GPIO_GPCLR0	10

/* Reference to the GPIO space */
/* An array of 32-bit integers */
volatile uint32_t *gpio;


static void delay(int length) {

	volatile int i;

	for(i=0;i<length;i++) ;

}

/* main function */
int main(int argc, char **argv) {

	/* Point to the physical GPIO region */
	gpio = (uint32_t *)GPIO_BASE;

	/* Enable GPIO for the ACT LED */
	/* On the Model B this is 16, on the B+ this is 47 */

	gpio[GPIO_GPFSEL1] |= (1 << 18);


	while(1) {

		/* Write 1 to clear the GPIO (turn on the LED) */

		gpio[GPIO_GPCLR0] = (1 << 16);

		/* delay */
		delay(0x3f000);

		/* Write 1 to set the GPIO (turn off the LED) */

		gpio[GPIO_GPSET0] = (1 << 16);

		/* delay */
		delay(0x3f000);

	}


	/* Hang forever; there is no OS to return to! */
	while(1) {
	}

	return 0;
}

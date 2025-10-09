/* This is code to drive the BCM2835 "BSC" i2c driver			*/
/* As decribed in Chapter 3 of the BCM2835 ARM Peripherals Manual	*/

/* We assume we're using the i2c1 bus on GPIO2/3 pins 3/5 */
/* Which have built-in pull up resistors on Pis */

#include <stddef.h>
#include <stdint.h>

#include "drivers/gpio/gpio.h"
#include "drivers/console/console_io.h"
#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/i2c/i2c.h"
#include "drivers/i2c/bcm2835_i2c.h"

#include "lib/delay.h"
#include "lib/printk.h"
#include "lib/locks.h"

#include "interrupts/interrupts.h"

static int bcm2835_i2c_initialized=0;


uint32_t bcm2835_i2c_write(unsigned char *buffer, uint32_t length) {
	/* FIFO only 16 bytes */

	int i;
	uint32_t control;

	printk("Writing %d bytes to i2c (%02X)\n",length,buffer[0]);

	if (length>15) {
		printk("i2c write too big %d\n",length);
	}

	/* transfer length */
	bcm2835_write(I2C1_DLEN, length);

	/* reset the FIFO */
	control=bcm2835_read(I2C1_CONTROL);
	control|=I2C_CONTROL_CLEAR_FIFO;
	bcm2835_write(I2C1_CONTROL,control);

	for(i=0;i<length;i++) {
		bcm2835_write(I2C1_FIFO,buffer[i]);
	}

	/* set done flag in status field */
	bcm2835_write(I2C1_STATUS,I2C_STATUS_DONE);

	/* write start */
	control=bcm2835_read(I2C1_CONTROL);
	control|=I2C_CONTROL_START_TRANSFER;
	bcm2835_write(I2C1_CONTROL,control);

	/* wait for finish */
	while (bcm2835_read(I2C1_STATUS&I2C_STATUS_DONE) != 1) {

	}

	return 0;
}


uint32_t bcm2835_set_address(uint32_t address) {
	/* set address */
	printk("Setting i2c to address %02X\n",address);
	bcm2835_write(I2C1_ADDRESS, address);
	return 0;
}

uint32_t bcm2835_i2c_init(struct i2c_type *i2c) {

	/* Set up config */

	/* Set up function pointers */
//	serial->uart_interrupt_handler=pl011_uart_interrupt_handler;

	/* Disable i2c */
	/* Turns off i2c and resets a few things */
	bcm2835_write(I2C1_CONTROL, 0x0);

	/* Setup GPIO 2/3 pins 3/5 */
	gpio_request(2,"i2c1_sda");
	gpio_request(3,"i2c1_scl");

	/* Set GPIO2 and GPIO3 to be i2c1 SDA/SCL, Alt Function 0 */
	gpio_function_select(2,GPIO_GPFSEL_ALT0);
	gpio_function_select(3,GPIO_GPFSEL_ALT0);

	/* Disable the pull up/down on GPIO 2/3 */
	/* See the Peripheral Manual p101 for more info */
	/* Configure to disable pull up/down and delay for 150 cycles */
	bcm2835_write(GPIO_GPPUD, GPIO_GPPUD_DISABLE);
	delay(150);

	/* Pass the disable clock to GPIO 2/3 and delay*/
	bcm2835_write(GPIO_GPPUDCLK0, (1 << 2) | (1 << 3));
	delay(150);

	/* write 0 to GPPUD?  Already 0 because of disable */
	bcm2835_write(GPIO_GPPUD, GPIO_GPPUD_DISABLE);

	/* Write 0 to GPPUDCLK0 to make it take effect */
	bcm2835_write(GPIO_GPPUDCLK0, 0x0);

	/* Set speed */
	/* Default to 100kbit/s? */

//	bcm2835_write(UART0_IBRD, 26);
//	bcm2835_write(UART0_FBRD, 3);


	/* Enable i2c */
	bcm2835_write(I2C1_CONTROL, I2C_CONTROL_I2CEN);

	unsigned char buffer[17];

	/* Debug */

	bcm2835_set_address(0xE1);

	#define HT16K33_REGISTER_ADDRESS_POINTER        0x00
#define HT16K33_REGISTER_SYSTEM_SETUP           0x20
#define HT16K33_REGISTER_KEY_DATA_POINTER       0x40
#define HT16K33_REGISTER_INT_ADDRESS_POINTER    0x60
#define HT16K33_REGISTER_DISPLAY_SETUP          0x80
#define HT16K33_REGISTER_ROW_INT_SET            0xA0
#define HT16K33_REGISTER_TEST_MODE              0xD0
#define HT16K33_REGISTER_DIMMING                0xE0

/* Blink rate */
#define HT16K33_BLINKRATE_OFF                   0x00

	buffer[0]= HT16K33_REGISTER_DISPLAY_SETUP | HT16K33_BLINKRATE_OFF | 0x1;
	bcm2835_i2c_write(buffer,1);

	buffer[0]= HT16K33_REGISTER_DIMMING | 15;
	bcm2835_i2c_write(buffer,1);

	buffer[0]=0x00;
	buffer[1]=0xff;
	buffer[2]=0xff;
	buffer[3]=0xff;
	buffer[4]=0xff;
	buffer[5]=0xff;
	buffer[6]=0xff;
	buffer[7]=0xff;
	bcm2835_i2c_write(buffer,8);


	bcm2835_i2c_initialized=1;

	return 0;
}



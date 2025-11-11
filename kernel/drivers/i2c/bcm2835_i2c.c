/* This is code to drive the BCM2835 "BSC" i2c driver			*/
/* As decribed in Chapter 3 of the BCM2835 ARM Peripherals Manual	*/

/* We assume we're using the i2c1 bus on GPIO2/3 pins 3/5 */
/* Which have built-in pull up resistors on Pis */

/* After my initial attempt didn't work, fixed the code */
/* based on Willow Cunningham's ECE531 Final Project */

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

#define CORE_CLOCK_SPEED	150000000	/* 150MHz says the manual */
#define IC2_SPEED_100K_DIVIDER	1500		/* 100kHz */
#define I2C_SPEED_10K_DIVIDER	15000		/* 10kHz */

static int bcm2835_i2c_initialized=0;


uint32_t bcm2835_i2c_write(uint32_t address,
			unsigned char *buffer, uint32_t length) {
	/* FIFO only 16 bytes */

	int i;
	uint32_t control,status;

	printk("Device %02x: Writing %d bytes to i2c (%02x)\n",
			address,length,buffer[0]);

	/* max size is 16 bits */
	if (length>65535) {
		printk("i2c write too big %d\n",length);
	}

	/* Set address */
	bcm2835_write(I2C1_ADDRESS, address);

	/* reset the FIFO */
	control=bcm2835_read(I2C1_CONTROL);
	control|=I2C_CONTROL_CLEAR_FIFO;
	bcm2835_write(I2C1_CONTROL,control);

	/* reset the status register fields */
	bcm2835_write(I2C1_STATUS,
		I2C_STATUS_DONE | I2C_STATUS_CLKT | I2C_STATUS_ERR);

	printk("Before write status = %x\n",bcm2835_read(I2C1_STATUS));

	/* set transfer length */
	bcm2835_write(I2C1_DLEN, length);

	/* start a write (do not set READ bit) */
	bcm2835_write(I2C1_CONTROL,
		I2C_CONTROL_I2CEN | I2C_CONTROL_START_TRANSFER);

	for(i=0;i<length;i++) {
		/* wait for there to be space in FIFO */
		while((bcm2835_read(I2C1_STATUS)&I2C_STATUS_TXD)==0) {
			asm("");	/* avoid optimizing away */
		}
		bcm2835_write(I2C1_FIFO,buffer[i]);
	}

	/* set done flag in status field */
//	bcm2835_write(I2C1_STATUS,I2C_STATUS_DONE);

	/* write start */
//	control=bcm2835_read(I2C1_CONTROL);
//	control|=I2C_CONTROL_START_TRANSFER;
//	bcm2835_write(I2C1_CONTROL,control);

	/* wait for finish */
	while (bcm2835_read(I2C1_STATUS&I2C_STATUS_DONE) != 1) {
		asm("");	/* avoid optimizing away */
	}

	status=bcm2835_read(I2C1_STATUS);
	printk("After write status = %x\n",status);
	if (status) {
		if (status&I2C_STATUS_ERR) printk("i2c: error slave did not ACK\n");
		if (status&I2C_STATUS_CLKT) printk("i2c: error clock stretch\n");
	}

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
	bcm2835_write(I2C1_DIV, IC2_SPEED_100K_DIVIDER);

	/* Enable i2c */
	bcm2835_write(I2C1_CONTROL, I2C_CONTROL_I2CEN);

	bcm2835_i2c_initialized=1;

	return 0;
}


uint32_t bcm2835_i2c_debug(void) {

	unsigned char buffer[17];

	uint32_t address;

	address=0x70;

	/* Debug */

	/* 1110 0001 */
	/* 1100 0101 */

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

	/* 0x21 */

	buffer[0]= HT16K33_REGISTER_DISPLAY_SETUP | HT16K33_BLINKRATE_OFF | 0x1;
	bcm2835_i2c_write(address,buffer,1);

	/* 0xEF */

	buffer[0]= HT16K33_REGISTER_DIMMING | 15;
	bcm2835_i2c_write(address,buffer,1);

	buffer[0]=0x00;
	buffer[1]=0xff;
	buffer[2]=0xff;
	buffer[3]=0xff;
	buffer[4]=0xff;
	buffer[5]=0xff;
	buffer[6]=0xff;
	buffer[7]=0xff;
	bcm2835_i2c_write(address,buffer,8);

	return 0;
}



/* This is code to drive the BCM2835 spi	 driver			*/
/* As decribed in Chapter ? of the BCM2835 ARM Peripherals Manual	*/

/* We assume we're using the spi bus on GPIO? pins ? */

/* also the libbcm2835 by Mike McCauley can be a good reference */

#include <stddef.h>
#include <stdint.h>

#include "drivers/gpio/gpio.h"
#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/spi/spi.h"
#include "drivers/spi/bcm2835_spi.h"

#include "lib/delay.h"
#include "lib/errors.h"
#include "lib/printk.h"
#include "lib/locks.h"

#include "interrupts/interrupts.h"

static int spi_debug=1;

static int bcm2835_spi_initialized=0;

int32_t bcm2835_spi_write(uint32_t device,
			unsigned char *buffer, uint32_t length) {

	bcm2835_peripheral_entry();

	/* FIFO only 16 bytes */

	int i;
	uint32_t control,status;
#if 0
	if (i2c_debug) {
		printk("Device %02x: Writing %d bytes to i2c (%02x)\n",
			address,length,buffer[0]);
	}

	/* max size is 16 bits */
	if (length>65535) {
		printk("i2c write too big %d\n",length);
		return -E2BIG;
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


	printk("Before write status\n");
	dump_status(bcm2835_read(I2C1_STATUS));



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
		/* now that the FIFO has space, push next byte */
		bcm2835_write(I2C1_FIFO,buffer[i]);
	}

	/* set done flag in status field */
//	bcm2835_write(I2C1_STATUS,I2C_STATUS_DONE);

	/* write start */
//	control=bcm2835_read(I2C1_CONTROL);
//	control|=I2C_CONTROL_START_TRANSFER;
//	bcm2835_write(I2C1_CONTROL,control);

	/* wait for finish */
	while ((bcm2835_read(I2C1_STATUS)&I2C_STATUS_DONE) ==0) {
		asm("");	/* avoid optimizing away */
	}

	status=bcm2835_read(I2C1_STATUS);
	printk("After write status\n");
	dump_status(status);
	if (status) {
		if (status&I2C_STATUS_ERR) printk("i2c: error slave did not ACK\n");
		if (status&I2C_STATUS_CLKT) printk("i2c: error clock stretch\n");
	}
#endif
	bcm2835_peripheral_exit();

	return 0;
}


int32_t bcm2835_spi_read(uint32_t device,
			unsigned char *buffer, uint32_t length) {

	int32_t result=0;

	bcm2835_peripheral_entry();
#if 0
	uint32_t i;
	uint32_t control,status;
	uint32_t remaining;

	if (i2c_debug) {
		printk("Device %02x: Reading %d bytes from i2c\n",
			address,length);
	}

	/* max size is 16 bits */
	if (length>65535) {
		printk("i2c read too big %d\n",length);
		return -E2BIG;
	}

	remaining=length;

	/* Set address */
	bcm2835_write(I2C1_ADDRESS, address);

	/* reset the FIFO */
	control=bcm2835_read(I2C1_CONTROL);
	control|=I2C_CONTROL_CLEAR_FIFO;
	bcm2835_write(I2C1_CONTROL,control);

	/* reset the status register fields */
	bcm2835_write(I2C1_STATUS,
		I2C_STATUS_DONE | I2C_STATUS_CLKT | I2C_STATUS_ERR);

	/* set transfer length */
	bcm2835_write(I2C1_DLEN, length);

	/* start a read */
	bcm2835_write(I2C1_CONTROL,
		I2C_CONTROL_I2CEN | I2C_CONTROL_START_TRANSFER |
		I2C_CONTROL_READ);

	/* do read transaction */
	i=0;
	while (1) {
		if (bcm2835_read(I2C1_STATUS) & I2C_STATUS_DONE) break;

		while (remaining &&
			(bcm2835_read(I2C1_STATUS) & I2C_STATUS_RXD)) {

			buffer[i] = bcm2835_read(I2C1_FIFO);
			i++;
			remaining--;
		}
	}

	/* get any stray data from FIFO */
	while (remaining &&
		(bcm2835_read(I2C1_STATUS) & I2C_STATUS_RXD)) {

		buffer[i] = bcm2835_read(I2C1_FIFO);
		i++;
		remaining--;
	}


	result=i;

	status=bcm2835_read(I2C1_STATUS);
	printk("After write status\n");
	dump_status(status);
	if (status) {
		if (status&I2C_STATUS_ERR) {
			printk("i2c: error slave did not ACK\n");
			result=-EIO;
		}
		if (status&I2C_STATUS_CLKT) {
			printk("i2c: error clock stretch\n");
			result=-EIO;
		}
	}

	/* reset done flag */
	status=bcm2835_read(I2C1_STATUS);
	status|=I2C_STATUS_DONE;
	bcm2835_write(I2C1_STATUS,status);
#endif

	bcm2835_peripheral_exit();

	return result;
}


uint32_t bcm2835_spi_init(struct spi_type *spi) {

	bcm2835_peripheral_entry();

#if 0
	/* Set up config */


	/* Set up function pointers */
//	serial->uart_interrupt_handler=pl011_uart_interrupt_handler;

	/* Disable i2c */
	/* Turns off i2c and resets a few things */
	bcm2835_write(I2C1_CONTROL, 0x0);

	/* Setup GPIO 2/3 (pins 3/5) */
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
	/* Default to 10kbit/s? */
	/* willow sets this to 1,500,000,000/10,000 = 150,000 */
	/* which on a Pi-1B gives ~12kHz SCLK measured with an oscilloscope */

	bcm2835_write(I2C1_DIV, 150000); //IC2_SPEED_100K_DIVIDER);

	/* Enable i2c */
//	bcm2835_write(I2C1_CONTROL, I2C_CONTROL_I2CEN);

	bcm2835_i2c_initialized=1;

	bcm2835_peripheral_exit();
#endif

	return 0;
}


uint32_t bcm2835_spi_debug(void) {

	return 0;
}



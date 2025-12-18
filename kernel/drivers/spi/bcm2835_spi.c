/* This is code to drive the BCM2835 SPI hardware		   */
/* As decribed in Chapter 10 of the BCM2835 ARM Peripherals Manual */

/* For now we just support "standard" mode.  The hardware also	*/
/* supports "bi-directional" (use one wire for MOSI/MISO) and	*/
/* LoSSI  as well						*/

/* Note there are two "mini" SPI busses described in Chapter 2	*/

/* We assume we're using the spi0 bus				*/
/*    on GPIO 7/8/9/10/11 (pins 26/14/21/19/23)			*/

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

	/* Set up config */


	/* Set up function pointers */
//	serial->uart_interrupt_handler=pl011_uart_interrupt_handler;

	/* Disable SPI? */
	/* needed? */

	/* Setup GPIO 10/9/11/8/7 (pins 19/21/23/24/26) */
	gpio_request(10,"spi_mosi");
	gpio_request( 9,"spi_miso");
	gpio_request(11,"spi_clk");
	gpio_request( 8,"spi_ce0");
	gpio_request( 7,"spi_ce1");

	/* Set SPI GPIOs to be Alt Function 0 */
	gpio_function_select(10,GPIO_GPFSEL_ALT0);
	gpio_function_select( 9,GPIO_GPFSEL_ALT0);
	gpio_function_select(11,GPIO_GPFSEL_ALT0);
	gpio_function_select( 8,GPIO_GPFSEL_ALT0);
	gpio_function_select( 7,GPIO_GPFSEL_ALT0);

	/* set SPI defaults */
	bcm2835_write(SPI0_CS, 0);

	/* clear SPI FIFOs */
	bcm2835_write(SPI0_CS, SPI0_CS_CLEAR_BOTH);

	bcm2835_spi_initialized=1;

	bcm2835_peripheral_exit();

	return 0;
}


uint32_t bcm2835_spi_debug(void) {

	return 0;
}



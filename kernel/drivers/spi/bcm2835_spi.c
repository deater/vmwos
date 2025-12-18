/* This is code to drive the BCM2835 SPI hardware		   */
/* As decribed in Chapter 10 of the BCM2835 ARM Peripherals Manual */

/* For now we just support "standard" mode.  The hardware also	*/
/* supports "bi-directional" (use one wire for MOSI/MISO) and	*/
/* LoSSI  as well						*/

/* Note there are two "mini" SPI busses described in Chapter 2	*/

/* We assume we're using the spi0 bus				*/
/*    on GPIO 7/8/9/10/11 (pins 26/14/21/19/23)			*/

/* Transfer is MSB first */

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


/* default is 0 (which means 65536) */
/* Must be power of 2, others rounded down */
/* The MAX is the APB clock (???MHz) */

void bcm2835_spi_set_clock_divider(uint16_t divider) {

	bcm2835_write(SPI0_CLK, divider);
}

void bcm2835_spi_set_speed_hz(uint32_t speed_hz) {

	uint16_t divider = (uint16_t) ((uint32_t) 250000000 / speed_hz);
	divider &= 0xfffe;

	if (spi_debug) {
		printk("Setting SPI to %d kHz (divider=%x)\n",
			speed_hz/1000,divider);
	}
	bcm2835_spi_set_clock_divider(divider);
}

/* Currently always send MSB first */
int32_t bcm2835_spi_transaction(uint32_t device,
			unsigned char *write_buffer,
			unsigned char *read_buffer,
			uint32_t length) {

	/* This is a polled transfer described in chapter 10.6.1 */
	/* 1. Set CS, CPOL, CPHA as required and set TA = 1 */
	/* 2. Poll TXD writing bytes to SPI_FIFO, */
	/*	RXD reading bytes from SPI_FIFO until all data written. */
	/* 3. Poll DONE until it goes to 1 */
	/* 4. Set TA = 0 */

	int32_t result=0,tx_count=0,rx_count=0;
	uint32_t control_status=0;
	int32_t chip_select=0;	/* FIXME: get from device info */

	bcm2835_peripheral_entry();

	if (spi_debug) {
		printk("SPI: Reading/writing %d bytes from device %d\n",
			length,chip_select);
	}

	/* Clear both the TX and RX FIFOs */
	control_status=bcm2835_read(SPI0_CS);
	control_status|=SPI0_CS_CLEAR_BOTH;
	bcm2835_write(SPI0_CS,control_status);

	/* Set TA = 1 */
	control_status=bcm2835_read(SPI0_CS);
	control_status|=SPI0_CS_TA;
	bcm2835_write(SPI0_CS,control_status);

	/* Read and Write Values */

	while((tx_count < length) || (rx_count < length)) {
		/* If TX FIFO not full add some more bytes */
		while( ( (bcm2835_read(SPI0_CS) & SPI0_CS_TXD))
			&& (tx_count < length ) ) {

			/* FIXME: handle proper byte ordering */
			bcm2835_write(SPI0_FIFO, write_buffer[tx_count]);
			tx_count++;
		}

		/* If RX FIFO not empty get the next bytes */

		while( ( (bcm2835_read(SPI0_CS) & SPI0_CS_RXD))
			&& (rx_count < length ) ) {

			/* FIXME: handle proper byte ordering */
			read_buffer[rx_count]=bcm2835_read(SPI0_FIFO);
			rx_count++;
		}
	}

	/* Wait for DONE */
	while (!(bcm2835_read(SPI0_CS) & SPI0_CS_DONE)) {
	}

	/* Set TA = 0 */
	control_status=bcm2835_read(SPI0_CS);
	control_status&=~SPI0_CS_TA;
	bcm2835_write(SPI0_CS,control_status);

	bcm2835_peripheral_exit();

	return result;
}


int32_t bcm2835_spi_write(uint32_t device,
			unsigned char *buffer, uint32_t length) {

	bcm2835_peripheral_entry();

	/* FIFO only 16 bytes */

#if 0
	int i;
	uint32_t control,status;

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


uint32_t bcm2835_spi_test(void) {

	bcm2835_spi_set_speed_hz(100000);		/* 100kHz */

	/* test on MCP3008 hooked to GND, 3.3V, TMP36 sensor */
	uint8_t data_out[3];
	uint8_t data_in[3];

	data_out[0]=1;
	data_out[1]=(0<<4)|0x80;
	data_out[2]=0;

	printk("Testing channel %x\n",data_out[1]);

	bcm2835_spi_transaction(0,data_out,data_in,3);
	printk("Result=%x %x %x\n",data_in[0],data_in[1],data_in[2]);

	data_out[0]=1;
	data_out[1]=(1<<4)|0x80;
	data_out[2]=0;

	printk("Testing channel %x\n",data_out[1]);

	bcm2835_spi_transaction(0,data_out,data_in,3);
	printk("Result=%x %x %x\n",data_in[0],data_in[1],data_in[2]);

	data_out[0]=1;
	data_out[1]=(2<<4)|0x80;
	data_out[2]=0;

	printk("Testing channel %x\n",data_out[1]);

	bcm2835_spi_transaction(0,data_out,data_in,3);
	printk("Result=%x %x %x\n",data_in[0],data_in[1],data_in[2]);


	return 0;
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

	bcm2835_spi_test();

	return 0;
}





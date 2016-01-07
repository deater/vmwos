/*
 * gpio.c -- vmwOS driver for raspberry pi GPIO
 *
 *	by Vince Weaver <vincent.weaver _at_ maine.edu>
 *
 *	Modeled on the Linux GPIO interface
 */


#include <stdint.h>
#include "gpio.h"
#include "printk.h"
#include "mmio.h"
#include "bcm2835_periph.h"

#define MAX_GPIO 54

static uint32_t debug=1;

static uint64_t gpio_reserved;


/* Keeps track of all GPIOs so we don't double allocate */
int gpio_request(int which_one, char *string) {

	if (which_one>MAX_GPIO) {
		printk("Invalid GPIO%d\n",which_one);
		return -1;
	}

	if ( (1ULL<<which_one) & gpio_reserved) {
		printk("GPIO%d already in use\n",which_one);
		return -1;
	}

	gpio_reserved|=(1ULL<<which_one);

	if (debug) printk("Allocating GPIO%d as %s\n",which_one,string);

	return 0;
}

/* mark GPIO as being available */
int gpio_free(int which_one) {

	if (which_one>MAX_GPIO) {
		printk("Invalid GPIO%d\n",which_one);
		return -1;
	}

	gpio_reserved&=~(1ULL<<which_one);

	if (debug) printk("Freeing GPIO%d\n",which_one);

	return 0;

}

/* Set GPIO direction to be an input */
int gpio_direction_input(int which_one) {

	uint32_t old;
	uint32_t addr_offset;
	uint32_t bit;

	if (which_one>MAX_GPIO) {
		printk("Invalid GPIO%d\n",which_one);
		return -1;
	}

	/* GPFSEL0 = 9 - 0 */
	addr_offset=(which_one/10)*4;
	bit=(which_one%10)<<3;

	/* 000 means input */

	old=mmio_read(GPIO_GPFSEL0+addr_offset);
	old &= ~bit;
	mmio_write(GPIO_GPFSEL0+addr_offset, old);

	return 0;

}

/* Set GPIO direction to be an output */
int gpio_direction_output(int which_one) {

	uint32_t old;
	uint32_t addr_offset;
	uint32_t bit;

	if (which_one>MAX_GPIO) {
		printk("Invalid GPIO%d\n",which_one);
		return -1;
	}

	/* GPFSEL0 = 9 - 0 */
	addr_offset=(which_one/10)*4;
	bit=(which_one%10)<<3;

	/* 001 means output */

	old=mmio_read(GPIO_GPFSEL0+addr_offset);
	old |= bit;
	mmio_write(GPIO_GPFSEL0+addr_offset, old);

	return 0;
}

/* FIXME */
int gpio_to_irq(int which_one) {

	return 0;
}

int gpio_get_value(int which_one) {

	uint32_t address_offset,bit;
	uint32_t result;

	if (which_one>MAX_GPIO) {
		printk("GPIO%d too big\n",which_one);
		return -1;
	}

	bit=1<<(which_one&0x1f);
	address_offset=(which_one/32)*4;

	result=mmio_read(GPIO_GPLEV0+address_offset);

	result=!!(result&bit);

	return result;

}

int gpio_set_value(int which_one, int value) {

	int address_offset,bit;

	if (which_one>MAX_GPIO) {
		printk("GPIO%d too big\n",which_one);
		return -1;
	}

	bit=1<<(which_one&0x1f);
	address_offset=(which_one/32)*4;

	if (value==0) {
		mmio_write(GPIO_GPSET0+address_offset,bit);
	}
	else if (value==1) {
		mmio_write(GPIO_GPCLR0+address_offset,bit);
	}
	else {
		printk("Invalid GPIO value %d\n",value);
	}

	return 0;
}

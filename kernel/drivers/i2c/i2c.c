/* This code tries to be a generic i2c driver */

#include <stddef.h>
#include <stdint.h>

#include "drivers/i2c/i2c.h"
#include "drivers/i2c/bcm2835_i2c.h"

#include "lib/errors.h"

static struct i2c_type i2c;

uint32_t i2c_init(uint32_t type) {

	uint32_t result=0;

	result=bcm2835_i2c_init(&i2c);

	if (result!=0) {
		i2c.initialized=0;
		return -ENODEV;
	}

	i2c.initialized=1;

	return 0;
}


/* write a series of bytes to the i2c port */
uint32_t i2c_write(const char* buffer, size_t size) {

	size_t i;

	if (!i2c.initialized) return 0;


	/* TODO */

	return i;
}


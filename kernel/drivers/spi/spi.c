/* This code tries to be a generic spi driver */

#include <stddef.h>
#include <stdint.h>

#include "drivers/spi/spi.h"
#include "drivers/spi/bcm2835_spi.h"

#include "lib/errors.h"

static struct spi_type spi;

uint32_t spi_init(uint32_t type) {

	uint32_t result=0;

	result=bcm2835_spi_init(&spi);

	if (result!=0) {
		spi.initialized=0;
		return -ENODEV;
	}

	spi.initialized=1;

	return 0;
}


/* write a series of bytes to spi */
int32_t spi_write(const char* buffer, size_t size) {

	size_t i;

	if (!spi.initialized) return 0;


	/* TODO */

	return i;
}


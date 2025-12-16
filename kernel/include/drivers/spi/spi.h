#define SPI_BCM2835		1

struct spi_type {
	uint32_t initialized;
	uint32_t speed;
};

uint32_t spi_init(uint32_t type);
int32_t spi_write(const char *buffer, size_t size);
int32_t spi_read(const char *buffer, size_t size);

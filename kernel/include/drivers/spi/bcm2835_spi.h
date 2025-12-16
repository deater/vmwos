uint32_t bcm2835_spi_init(struct spi_type *spi);
int32_t bcm2835_spi_write(uint32_t device,
                        unsigned char *buffer, uint32_t length);
int32_t bcm2835_spi_read(uint32_t device,
                        unsigned char *buffer, uint32_t length);
int32_t bcm2835_spi_read_write(uint32_t device,
                        unsigned char *read_buffer,
			unsigned char *write_buffer,uint32_t length);


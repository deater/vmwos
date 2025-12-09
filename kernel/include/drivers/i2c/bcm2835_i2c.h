uint32_t bcm2835_i2c_init(struct i2c_type *i2c);
int32_t bcm2835_i2c_write(uint32_t address,
                        unsigned char *buffer, uint32_t length);
int32_t bcm2835_i2c_read(uint32_t address,
                        unsigned char *buffer, uint32_t length);


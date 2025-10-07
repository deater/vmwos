#define I2C_BCM2835		1

struct i2c_type {
	uint32_t initialized;
	uint32_t speed;
};

uint32_t i2c_init(uint32_t type);
uint32_t i2c_write(const char *buffer, size_t size);
uint32_t i2c_read(const char *buffer, size_t size);

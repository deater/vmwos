uint32_t ramdisk_read(uint32_t offset, uint32_t length, char *dest);
uint32_t ramdisk_init(unsigned char *start, uint32_t length);
uint32_t ramdisk_read_string(uint32_t offset, uint32_t maxlen, char *dest);


#define RAMDISK_MAJOR	1

//int32_t ramdisk_read(uint32_t offset, uint32_t length, char *dest);
struct block_dev_type *ramdisk_init(unsigned char *start, uint32_t length);



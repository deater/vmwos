#include <stdint.h>
#include <stddef.h>

#include "lib/errors.h"
#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"

#include "drivers/block.h"
#include "drivers/ramdisk/ramdisk.h"

static int debug=1;

static int32_t minors_allocated=0;

int32_t ramdisk_read(struct block_dev_type *dev,
			uint32_t offset, uint32_t length, char *dest) {

	/* Make sure we are in range */
	if (dev->start+length>dev->length) {
		if (debug) {
			printk("ramdisk: access out of range %d > %d\n",
				dev->start+length,dev->length);
		}
		return -ERANGE;
	}

	memcpy(dest,(char *)(dev->private)+offset,length);

	return length;

}

int32_t ramdisk_write(struct block_dev_type *dev,
			uint32_t offset, uint32_t length, char *src) {

	/* Make sure we are in range */
	if (dev->start+length>dev->length) {
		if (debug) {
			printk("ramdisk: access out of range %d > %d\n",
				dev->start+length,dev->length);
		}
		return -ERANGE;
	}

	memcpy((char *)(dev->private)+offset,src,length);

	return length;

}

int32_t ramdisk_ioctl(struct block_dev_type *dev,
			uint32_t cmd, uint32_t three, uint32_t four) {

	return -ENOTTY;
}

static struct block_operations ramdisk_ops = {
	.read = ramdisk_read,
	.write = ramdisk_write,
	.ioctl = ramdisk_ioctl,
};

struct block_dev_type *ramdisk_init(unsigned char *start, uint32_t length) {

	struct block_dev_type *dev;

	dev=allocate_block_dev();
	if (dev==NULL) {
		return NULL;
	}

	dev->major=RAMDISK_MAJOR;
	dev->minor=minors_allocated;
	snprintf(dev->name,BLOCK_NAME_LENGTH,"ramdisk%d",dev->minor);
	if (debug) printk("Allocated ramdisk %s\n",dev->name);

	minors_allocated++;

	dev->start=0;
	dev->length=length;
	dev->capacity=length;
	dev->private=start;
	dev->block_ops=&ramdisk_ops;

	printk("Initialized ramdisk%d of size %d at address 0x%x\n",
		dev->minor,length,start);

	return dev;

}

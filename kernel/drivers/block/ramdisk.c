#include <stdint.h>

#include "lib/errors.h"
#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"
#include "drivers/block/ramdisk.h"

static int debug=1;

struct ramdisk_info_t {
	unsigned char *start;
	uint32_t length;
	uint32_t flags;
} ramdisk_info;

int32_t ramdisk_read(uint32_t offset, uint32_t length, char *dest) {

	/* Make sure we are in range */
	if (offset+length>ramdisk_info.length) {
		if (debug) {
			printk("ramdisk: access out of range %d > %d\n",
				offset+length,ramdisk_info.length);
		}
		return -ERANGE;
	}

	memcpy(dest,ramdisk_info.start+offset,length);

	return length;

}

int32_t ramdisk_init(unsigned char *start, uint32_t length) {

	printk("Initializing ramdisk of size %d at address %x\n",
		length,start);

	ramdisk_info.start=start;
	ramdisk_info.length=length;

	return 0;

}

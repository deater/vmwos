#include <stdint.h>


#include "errors.h"
#include "lib/printk.h"
#include "lib/string.h"
#include "drivers/block/ramdisk.h"

struct ramdisk_info_t {

	char *start;
	uint32_t length;
	uint32_t flags;
} ramdisk_info;

uint32_t ramdisk_read(uint32_t offset, uint32_t length, char *dest) {

	/* Make sure we are in range */
	if (offset+length>ramdisk_info.length) {
		return ERANGE;
	}

	memcpy(dest,ramdisk_info.start+offset,length);

	return 0;

}

/* read NUL-terminated string */
/* FIXME: verify we got the ranges right */
uint32_t ramdisk_read_string(uint32_t offset, uint32_t maxlen, char *dest) {

	uint32_t ptr=offset;
	char *output=dest;
	char ch;
	uint32_t length=0;

	while(1) {
		ch=*(ramdisk_info.start+ptr);

		/* We hit the end of the ramdisk */
		if (
			(ch==0) ||
			(length>=maxlen) ||
			(offset+length>ramdisk_info.length)) {
			*output=0;
			return 0;
		}

		*output=ch;

		ptr++;
		output++;
		length++;
	}

	return 0;

}

uint32_t ramdisk_init(unsigned char *start, uint32_t length) {

	printk("Initializing ramdisk of size %d at address %x\n",
		length,start);

	ramdisk_info.start=start;
	ramdisk_info.length=length;

	return 0;

}

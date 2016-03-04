#include <stdint.h>


#include "errors.h"
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

uint32_t ramdisk_init(char *start, uint32_t length) {

	ramdisk_info.start=start;
	ramdisk_info.length=length;

	return 0;

}

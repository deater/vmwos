#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"

#include "fs/files.h"

#include "drivers/drivers.h"
#include "drivers/char.h"



static struct char_dev_type char_devs[CHAR_DEV_MAX];

static int32_t num_char_devs=0;

struct char_dev_type *allocate_char_dev(void) {

	if (num_char_devs==CHAR_DEV_MAX-1) {
		return NULL;
	}

	num_char_devs++;

	return &char_devs[num_char_devs];
}

struct char_dev_type *char_dev_find(const char *name) {

	int i;

	for(i=0;i<CHAR_DEV_MAX;i++) {
		if (!strncmp(name,char_devs[i].name,strlen(name))) {
			return &char_devs[i];
		}
	}

	return NULL;
}

struct char_dev_type *char_dev_lookup(uint32_t devnum) {

	int i;

	for(i=0;i<CHAR_DEV_MAX;i++) {
		if ( (char_devs[i].major==(devnum>>16)) &&
			(char_devs[i].minor==(devnum&0xffff)) ) {
			return &char_devs[i];
		}
	}

	return NULL;
}

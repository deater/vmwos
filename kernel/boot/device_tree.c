/* Original references */
/* "Device Trees Everywhere" by Gibson and Herrenschmidt */

#include <stdint.h>

#include "hardware.h"
#include "lib/string.h"
#include "lib/memset.h"
#include "errors.h"

#include "lib/printk.h"

#include "boot/device_tree.h"

struct device_tree_type {
	uint32_t valid;
	uint32_t magic;
	uint32_t totalsize;
	uint32_t off_struct;
	uint32_t off_strs;
	uint32_t off_rsvmap;
	uint32_t version;
	uint32_t last_comp_ver;
	uint32_t boot_cpu_id;
	uint32_t size_strs;
} device_tree;

/* We assume the pi is always little endian */
static uint32_t big_to_little(uint32_t big) {

	uint32_t little;

	little=(	((big&0xff000000)>>24) |
			((big&0x00ff0000)>>8) |
			((big&0x0000ff00)<<8) |
			((big&0x000000ff)<<24)	);

	return little;
}

int32_t devicetree_decode(uint32_t *dt_ptr) {

	uint32_t *dt;
#if 0
	dt=dt_ptr;

	device_tree.valid=0;

	device_tree.magic=big_to_little(dt[0]);
//	device_tree.magic=big_to_little(0xdeadbeef);
	if (device_tree.magic!=0xd00dfeed) {
		return -ENODEV;
	}

	device_tree.valid=1;

	device_tree.totalsize=big_to_little(dt[1]);
	device_tree.off_struct=big_to_little(dt[2]);
	device_tree.off_strs=big_to_little(dt[3]);
	device_tree.off_rsvmap=big_to_little(dt[4]);
	device_tree.version=big_to_little(dt[5]);
	device_tree.last_comp_ver=big_to_little(dt[6]);
	if (device_tree.version>=2) {
		device_tree.boot_cpu_id=big_to_little(dt[7]);
	}

	if (device_tree.version>=3) {
		device_tree.size_strs=big_to_little(dt[8]);
	}
#endif
	return -ENODEV;

}


void devicetree_dump(void) {

	if (!device_tree.valid) return;

	printk("Device tree:\n");
	printk("\tMagic: %x\n",device_tree.magic);
	printk("\tTotalsize:  %d\n",device_tree.totalsize);
	printk("\tOff Struct: %x\n",device_tree.off_struct);
	printk("\tOff strs:   %x\n",device_tree.off_strs);
	printk("\tOff rsvmap: %x\n",device_tree.off_rsvmap);
	printk("\tVersion:    %d\n",device_tree.version);
	printk("\tLastCompVer:%d\n",device_tree.last_comp_ver);
	if (device_tree.version>=2) {
		printk("\tBootCpuID: %d\n",device_tree.boot_cpu_id);
	}
	if (device_tree.version>=3) {
		printk("\tSizeStrs: %d\n",device_tree.size_strs);
	}
}

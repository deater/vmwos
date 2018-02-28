/* Original references */
/* "Device Trees Everywhere" by Gibson and Herrenschmidt */


#define OF_DT_BEGIN_NODE	1
#define OF_DT_PROP		3
#define OF_DT_END_NODE		2
#define OF_DT_END		9

#include <stdint.h>

#include "hardware.h"
#include "lib/string.h"
#include "lib/memset.h"
#include "errors.h"

#include "lib/printk.h"

#include "boot/device_tree.h"

struct device_tree_type {
	void *address;
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

/* We assume the pi is always little endian */
static uint64_t big_to_little64(uint64_t big) {

	uint64_t little;

	little=(	((big&0xff00000000000000ULL)>>48) |
			((big&0x00ff000000000000ULL)>>32) |
			((big&0x0000ff0000000000ULL)>>16) |
			((big&0x000000ff00000000ULL)>>8) |
			((big&0x00000000ff000000ULL)<<8) |
			((big&0x0000000000ff0000ULL)<<16) |
			((big&0x000000000000ff00ULL)<<32) |
			((big&0x00000000000000ffULL)<<48));


	return little;
}


int32_t devicetree_decode(uint32_t *dt_ptr) {

	uint32_t *dt;

	dt=dt_ptr;

	device_tree.valid=0;

	device_tree.magic=big_to_little(dt[0]);

	if (device_tree.magic!=0xd00dfeed) {
		return -ENODEV;
	}

	device_tree.valid=1;

	device_tree.address=(void *)dt_ptr;

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

	return 0;

}


void devicetree_dump(void) {

	int i;
	uint64_t *memory_reserved;
	uint32_t *tree;

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

	/* Print reserved memory table */
	memory_reserved=(uint64_t *)(device_tree.address+device_tree.off_rsvmap);

	printk("Reserved memory:\n");

	i=0;
	while(1) {
		if ((memory_reserved[i]==0) && (memory_reserved[i+1]==0)) break;
		printk("\t0x%llx bytes starting at address 0x%llx\n",
			big_to_little64(memory_reserved[i+1]),
			big_to_little64(memory_reserved[i]));
		i+=2;
	}

	tree=(uint32_t *)(device_tree.address+device_tree.off_struct);
	printk("Device Tree:\n");
	i=0;

	while(1) {
		if (big_to_little(tree[i])!=OF_DT_BEGIN_NODE) {
			printk("Expected OF_DT_BEGIN_NODE, got %x\n",tree[i]);
			return;
		}
		i++;

		/* See if at end */
		if (big_to_little(tree[i])==OF_DT_END) break;
		if (device_tree.off_struct + (i*4) > device_tree.totalsize) {
			break;
		}
	}
}


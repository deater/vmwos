
#include <stddef.h>
#include <stdint.h>

static int bflt_debug=1;

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/errors.h"
#include "lib/endian.h"
#include "lib/memcpy.h"
#include "lib/smp.h"

#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/romfs/romfs.h"


#include "processes/bflt_loader.h"

int32_t bflt_load(int32_t inode,
		uint32_t *stack_size, uint32_t *text_start,
		uint32_t *data_start, uint32_t *bss_start,
		uint32_t *bss_end, uint32_t *total_size) {

	int result;
	char bflt_header[64];
	uint32_t temp_int;

	if (bflt_debug) printk("Running BFLT executable!\n");

	result=romfs_read_file(inode,0,&bflt_header,64);
	if (result<0) return result;

	/* Find stack size */
	memcpy(&temp_int,&bflt_header[BFLT_STACK_SIZE],4);
	*stack_size=ntohl(temp_int);

	if (bflt_debug) printk("BFLT: stack size=%d\n",*stack_size);

	/* Find binary size */
	memcpy(&temp_int,&bflt_header[BFLT_ENTRY],4);
	*text_start=ntohl(temp_int);
	if (bflt_debug) printk("BFLT: text_start=%x\n",*text_start);

	memcpy(&temp_int,&bflt_header[BFLT_DATA_START],4);
	*data_start=ntohl(temp_int);
	if (bflt_debug) printk("BFLT: data_start=%x\n",*data_start);

	memcpy(&temp_int,&bflt_header[BFLT_BSS_START],4);
	*bss_start=ntohl(temp_int);
	if (bflt_debug) printk("BFLT: bss_start=%x\n",*bss_start);

	memcpy(&temp_int,&bflt_header[BFLT_BSS_END],4);
	*bss_end=ntohl(temp_int);
	if (bflt_debug) printk("BFLT: bss_end=%x\n",*bss_end);

	*total_size=*bss_end-*text_start;
	if (bflt_debug) printk("BFLT: total size=%x (%d)\n",
				*total_size,*total_size);

	return 0;
}


int32_t bflt_reloc(int32_t inode, void *binary_address) {

	int result,i;
	char bflt_header[64];
	uint32_t temp_int;
	uint32_t reloc_count,reloc_start;

	if (bflt_debug) printk("Relocating BFLT executable!\n");

	result=romfs_read_file(inode,0,&bflt_header,64);
	if (result<0) return result;

	/* Find Number of Relocations */
	memcpy(&temp_int,&bflt_header[BFLT_RELOC_COUNT],4);
	reloc_count=ntohl(temp_int);

	if (bflt_debug) printk("BFLT: reloc_count=%d\n",reloc_count);

	if (reloc_count==0) return 0;

	/* Find Relocation Offset */
	memcpy(&temp_int,&bflt_header[BFLT_RELOC_START],4);
	reloc_start=ntohl(temp_int);
	if (bflt_debug) printk("BFLT: reloc_start=%x\n",reloc_start);

	for(i=0;i<reloc_count;i++) {
		printk("Reloc %d: \n",i);
	}

	return 0;
}


#include <stddef.h>
#include <stdint.h>

static int bflt_debug=0;

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/errors.h"
#include "lib/endian.h"
#include "lib/memcpy.h"
#include "lib/smp.h"

//#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"
#include "fs/romfs/romfs.h"



#include "processes/bflt_loader.h"

int32_t bflt_load(struct file_object *file,
		uint32_t *stack_size, uint32_t *text_start,
		uint32_t *data_start, uint32_t *bss_start,
		uint32_t *bss_end,
		uint32_t *total_ondisk_size,
		uint32_t *total_program_size) {

	int result;
	char bflt_header[64];
	uint32_t temp_int;
	uint64_t file_offset;

	if (bflt_debug) printk("Running BFLT executable!\n");

	file_offset=0;
	result=file->file_ops->read(file->inode,
		(char *)&bflt_header,64,&file_offset);
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

	*total_ondisk_size=*bss_start-*text_start;
	if (bflt_debug) printk("BFLT: total ondisk size=%x (%d)\n",
				*total_ondisk_size,*total_ondisk_size);

	*total_program_size=*bss_end-*text_start;
	if (bflt_debug) printk("BFLT: total program-size=%x (%d)\n",
				*total_program_size,*total_program_size);

	return 0;
}


int32_t bflt_reloc(struct file_object *file, void *binary_address) {

	int result,i;
	char bflt_header[64];
	uint32_t temp_int;
	uint32_t reloc_count,reloc_start;
	uint32_t reloc_addr;
	uint32_t old_val,new_val;
	uint64_t file_offset;

	if (bflt_debug) printk("Relocating BFLT executable!\n");

	file_offset=0;
	result=file->file_ops->read(file->inode,
		(char *)&bflt_header,64,&file_offset);
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
		file_offset=reloc_start+(i*4);
		result=file->file_ops->read(file->inode,
							(char *)&reloc_addr,
							4,&file_offset);
		if (result<0) return -1;
		reloc_addr=ntohl(reloc_addr);

		memcpy(&old_val,(binary_address+reloc_addr),4);
		new_val=(uint32_t)(old_val+binary_address);
		memcpy((binary_address+reloc_addr),&new_val,4);

		if (bflt_debug) {
			printk("Reloc %d: %x, %x->%x\n",i,
				reloc_addr,old_val,new_val);
		}
	}

	return 0;
}


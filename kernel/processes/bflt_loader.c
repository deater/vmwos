
#include <stddef.h>
#include <stdint.h>

static int bflt_debug=1;

#if 0
#include "memory/memory.h"
#include "memory/mmu-common.h"
#endif

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/errors.h"
#include "lib/endian.h"
#include "lib/memcpy.h"
#include "lib/smp.h"

#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/romfs/romfs.h"

#if 0
#include "processes/process.h"
#include "processes/exit.h"


static int exec_summary_debug=1;

int32_t execve(const char *filename, char *const argv[], char *const envp[]) {

	int result,i;
	int32_t inode;
	struct stat stat_info;
	void *binary_start,*stack_page;
	int32_t stack_size,size;
	int32_t argc=0;
	char *argv_location;
	int32_t argv_length=0;
	uint32_t *stack_argv;
	char *argv_ptr;
	char magic[16];
	uint32_t text_start;


	if (exec_debug) printk("Entering execve\n");

	inode=get_inode(filename);
	if (inode<0) {
		if (exec_debug) printk("Error get_inode(%s)\n",filename);
		return inode;
	}

	/* See what kind of file it is */
	result=romfs_read_file(inode,0,&magic,16);

	/* see if a bFLT file */
	if ((magic[0]=='b') && (magic[1]=='F') &&
		(magic[2]=='L') && (magic[3]=='T')) {

#endif

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
	memcpy(&temp_int,&bflt_header[24],4);
	*stack_size=ntohl(temp_int);

	if (bflt_debug) printk("BFLT: stack size=%d\n",*stack_size);

	/* Find binary size */
	memcpy(&temp_int,&bflt_header[8],4);
	*text_start=ntohl(temp_int);
	if (bflt_debug) printk("BFLT: text_start=%x\n",*text_start);

	memcpy(&temp_int,&bflt_header[12],4);
	*data_start=ntohl(temp_int);
	if (bflt_debug) printk("BFLT: data_start=%x\n",*data_start);

	memcpy(&temp_int,&bflt_header[16],4);
	*bss_start=ntohl(temp_int);
	if (bflt_debug) printk("BFLT: bss_start=%x\n",*bss_start);

	memcpy(&temp_int,&bflt_header[20],4);
	*bss_end=ntohl(temp_int);
	if (bflt_debug) printk("BFLT: bss_end=%x\n",*bss_end);

	*total_size=*bss_end-*text_start;
	if (bflt_debug) printk("BFLT: total size=%x (%d)\n",
				*total_size,*total_size);

	return 0;
}

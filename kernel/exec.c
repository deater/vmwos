#include <stddef.h>
#include <stdint.h>

#include "memory.h"

#include "lib/printk.h"
#include "lib/string.h"

#include "drivers/block/ramdisk.h"

#include "fs/romfs/romfs.h"

int load_exe(char *name,char **binary_start,char **stack_start,
		int *size, int *stack_size) {

	struct romfs_file_header_t file;
	int result;

	result=open_romfs_file(name,&file);
	if (result<0) {
		return result;
	}

	*size=file.size;
	*stack_size=8192;

	/* Allocate Memory */
	*binary_start=(char *)memory_allocate(*size);
	*stack_start=(char *)memory_allocate(*stack_size);

	/* Load executable */
	ramdisk_read(file.data_start,*size,*binary_start);

//	memcpy(*binary_start,initrd_image+file.data_start,*size);

	return 0;
}



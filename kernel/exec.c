#include <stddef.h>
#include <stdint.h>

#include "memory.h"

#include "lib/printk.h"
#include "lib/string.h"

#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/romfs/romfs.h"

/* Load raw executable */
/* We don't have a file format yet */
int load_exe(char *name,char **binary_start,char **stack_start,
		int *size, int *stack_size) {

	int result;
	int32_t inode;
	struct stat stat_info;

	inode=romfs_get_inode(name);
	if (inode<0) {
		return result;
	}

	result=romfs_stat(inode,&stat_info);
	*size=stat_info.st_size;

	*stack_size=8192;

	/* Allocate Memory */
	*binary_start=(char *)memory_allocate(*size);
	*stack_start=(char *)memory_allocate(*stack_size);

	/* Load executable */
	romfs_read_file(inode,0,*binary_start,*size);

	return 0;
}



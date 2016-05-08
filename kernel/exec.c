#include <stddef.h>
#include <stdint.h>

#include "memory.h"

#include "lib/printk.h"
#include "lib/string.h"

#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/romfs/romfs.h"

#include "process.h"

/* Load raw executable */
/* We don't have a file format yet */
int32_t execve(const char *filename, char *const argv, char *const envp) {

	int result;
	int32_t inode;
	struct stat stat_info;
	void *binary_start,*stack_start;
	int32_t stack_size,size;

	inode=romfs_get_inode(filename);
	if (inode<0) {
		return result;
	}

	result=romfs_stat(inode,&stat_info);
	size=stat_info.st_size;

	/* TODO: get from executable */
	stack_size=8192;

	/* Allocate Memory */
	binary_start=memory_allocate(size);
	stack_start=memory_allocate(stack_size);

	/* FIXME: handle memory allocation failure */

	/* Load executable */
	romfs_read_file(inode,0,binary_start,size);

	/* Set name */
	strncpy(process[current_process].name,filename,32);

	/* Setup the stack */
        /* is the -4 needed? */
        process[current_process].reg_state.r[13]=((long)stack_start+stack_size);
        process[current_process].stack=stack_start;
        process[current_process].stacksize=stack_size;

        /* Setup the entry point */
        process[current_process].reg_state.lr=(long)binary_start;
        process[current_process].text=binary_start;
        process[current_process].textsize=size;

        printk("Execed process %s pid %d "
                "allocated %dkB at %x and %dkB stack at %x\n",
                filename,process[current_process].pid,
                size/1024,binary_start,
                stack_size/1024,stack_start);


	return 0;
}



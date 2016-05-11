#include <stddef.h>
#include <stdint.h>

#include "memory.h"

#include "lib/printk.h"
#include "lib/string.h"

#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/romfs/romfs.h"

#include "process.h"
#include "exit.h"

/* Load raw executable */
/* We don't have a file format yet */
int32_t execve(const char *filename, char *const argv[], char *const envp[]) {

	int result,i;
	int32_t inode;
	struct stat stat_info;
	void *binary_start,*stack_start;
	int32_t stack_size,size;
	int32_t argc=0;
	char *argv_location;
	int32_t argv_length=0;
	uint32_t *stack_argv;
	char *argv_ptr;

	inode=romfs_get_inode(filename);
	if (inode<0) {
		return result;
	}

	result=romfs_stat(inode,&stat_info);
	size=stat_info.st_size;

	/* TODO: get from executable */
	stack_size=DEFAULT_USER_STACK_SIZE;

	/* Allocate Memory */
	binary_start=memory_allocate(size);
	stack_start=memory_allocate(stack_size);

	/* FIXME: handle memory allocation failure */

	/* Load executable */
	romfs_read_file(inode,0,binary_start,size);

	/* Set name */
	strncpy(process[current_process].name,filename,32);

	argv_location=(stack_start+stack_size);

	if (argv!=NULL) {

		/* Setup argv */

		/* Calculate argc */
		argc=0;
		while(argv[argc]!=0) {
			argc++;
		}

		printk("vmwos:exec: found %d arguments\n",argc);
		for(i=0;i<argc;i++) {
			printk("%d: %x %s\n",i,(long)argv[i],argv[i]);
		}

		argv_length=(argc+1)*sizeof(char *)+
			(argv[argc-1]-argv[0])+strlen(argv[argc]+1);
		printk("vmwos:exec: argv length %d\n",argv_length);

		/* Align to 8-byte boundary */
		argv_length=((argv_length/8)+1)*8;
		printk("vmwos:exec: argv length aligned %d\n",argv_length);

		argv_location=(stack_start+stack_size-argv_length);
		printk("vmwos:exec: argv location: %x\n",argv_location);

		stack_argv=(uint32_t *)argv_location;
		argv_ptr=(char *)stack_argv[argc+2];
		*argv_ptr=0;

		for(i=0;i<argc;i++) {
			stack_argv[i]=(uint32_t)argv_ptr;
			argv_ptr=strncpy(argv_ptr,argv[i],strlen(argv[i]));
		}

	}

	/* Setup the stack */
        /* is the -4 needed? */
        process[current_process].reg_state.r[13]=(long)argv_location;
        process[current_process].stack=stack_start;
        process[current_process].stacksize=stack_size;

	/* Setup lr to point to exit */
	/* That way when a program exits it will return to where lr points */
	process[current_process].reg_state.r[14]=(long)exit;

	/* Make r0=argc */
	process[current_process].reg_state.r[0]=argc;
	/* Make r1=argv */
	process[current_process].reg_state.r[1]=(long)argv_location;



        /* Setup the entry point */
        process[current_process].reg_state.lr=(long)binary_start;
        process[current_process].text=binary_start;
        process[current_process].textsize=size;

        printk("Execed process %s current_process %d pid %d "
                "allocated %dkB at %x and %dkB stack at %x\n",
                filename,current_process,process[current_process].pid,
                size/1024,binary_start,
                stack_size/1024,stack_start);


	/* r0 gets overwritten with syscall result */
	/* at end of syscall handler */
	return argc;
}



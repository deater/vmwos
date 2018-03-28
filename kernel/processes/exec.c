#include <stddef.h>
#include <stdint.h>

#include "memory/memory.h"
#include "memory/mmu-common.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/errors.h"
#include "lib/endian.h"
#include "lib/memcpy.h"

#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/romfs/romfs.h"

#include "processes/process.h"
#include "processes/exit.h"

static int exec_debug=0;
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

		char bflt_header[64];
		uint32_t temp_int;
		uint32_t data_start;
		uint32_t bss_start,bss_end;

		if (exec_debug) printk("Found BFLT executable!\n");

		result=romfs_read_file(inode,0,&bflt_header,64);

		/* Find stack size */
		memcpy(&temp_int,&bflt_header[24],4);
		stack_size=ntohl(temp_int);
		if (exec_debug) printk("BFLT: stack size=%d\n",stack_size);

		/* Find binary size */
		memcpy(&temp_int,&bflt_header[8],4);
		text_start=ntohl(temp_int);
		if (exec_debug) printk("BFLT: text_start=%x\n",text_start);

		memcpy(&temp_int,&bflt_header[12],4);
		data_start=ntohl(temp_int);
		if (exec_debug) printk("BFLT: data_start=%x\n",data_start);

		memcpy(&temp_int,&bflt_header[16],4);
		bss_start=ntohl(temp_int);
		if (exec_debug) printk("BFLT: bss_start=%x\n",bss_start);

		memcpy(&temp_int,&bflt_header[20],4);
		bss_end=ntohl(temp_int);
		if (exec_debug) printk("BFLT: bss_end=%x\n",bss_end);

		size=bss_end-text_start;
		if (exec_debug) printk("BFLT: total size=%x (%d)\n",size,size);

		current_process->datasize=bss_start-data_start;
		current_process->bsssize=bss_end-bss_start;


	}
	/* Otherwise, treat as raw binary */
	else {
		if (exec_debug) printk("Assuming RAW executable!\n");

		/* Allocate stack */
		stack_size=DEFAULT_USER_STACK_SIZE;
		if (exec_debug) printk("RAW: stack size = %d\n",stack_size);

		result=romfs_stat(inode,&stat_info);
		if (result<0) {
			if (exec_debug) printk("Error stat()\n");
			return result;
		}

		text_start=0;
		size=stat_info.st_size;
		if (exec_debug) printk("RAW: total size = %d\n",size);
	}


	/* Allocate stack */
	stack_page=memory_allocate(stack_size);

	/* Allocate text memory */
	binary_start=memory_allocate(size);

	if ((binary_start==NULL) || (stack_page==NULL)) {

		if (binary_start!=NULL) memory_free(binary_start,size);
		if (stack_page!=NULL) memory_free(stack_page,stack_size);

		if (exec_debug) printk("execve: no memory\n");
		return -ENOMEM;
	}

	/* Load executable */
	romfs_read_file(inode,text_start,binary_start,size);

	/* Set name */
	/* FIXME: strip off path before setting filename */
	strlcpy(current_process->name,filename,32);

	/* Set up command line arguments */

	/* Set the location to be just above stack */
	/* stack_page is beginning of stack page, but stack starts at end */
	/* and grows down */

	argv_location=(stack_page+stack_size);

	if (argv!=NULL) {

		/* Setup argv */

		/* Calculate argc */
		argc=0;
		while(argv[argc]!=0) {
			argc++;
		}

		if (exec_debug) {
			printk("vmwos:exec: found %d arguments\n",argc);
			for(i=0;i<argc;i++) {
				printk("%d: %x %s\n",i,(long)argv[i],argv[i]);
			}
		}

		argv_length=(argc+1)*sizeof(char *)+/* number of pointers */
						/* plus one for NULL terminated list */
			(argv[argc-1]-argv[0])+	/* add size of N-1 strings */
			strlen(argv[argc])+	/* add length of last string */
			1;			/* 1 for last NUL terminator */
		if (exec_debug) printk("vmwos:exec: argv length %d\n",argv_length);

		/* Align to 8-byte boundary */
		argv_length=((argv_length/8)+1)*8;
		if (exec_debug) {
			printk("vmwos:exec: argv length aligned %d\n",
								argv_length);
		}

		argv_location=(stack_page+stack_size-argv_length);
		if (exec_debug) {
			printk("vmwos:exec: argv location: %x\n",argv_location);
		}

		stack_argv=(uint32_t *)argv_location;
		argv_ptr=(char *)(&stack_argv[argc+2]);

		argv_ptr[0]=0;

		for(i=0;i<argc;i++) {
			stack_argv[i]=(uint32_t)argv_ptr;
			argv_ptr=strncpy(argv_ptr,argv[i],strlen(argv[i])+1);
			argv_ptr+=(strlen(argv[i])+1);
			if (exec_debug) printk("vmwos: argv[%d]=%x %s\n",
				i,stack_argv[i],(char *)stack_argv[i]);
		}
	}

	/* Setup the stack */
        current_process->user_state.r[13]=(long)argv_location;
        current_process->stack=stack_page;
        current_process->stacksize=stack_size;

	/* Setup lr to point to exit */
	/* That way when a program exits it will return to where lr points */
//	current_process->reg_state.r[14]=(long)exit;

	/* Make r0=argc */
	current_process->user_state.r[0]=argc;
	/* Make r1=argv */
	current_process->user_state.r[1]=(long)argv_location;

        /* Setup the entry point */
        current_process->user_state.pc=(long)binary_start;
        current_process->text=binary_start;
        current_process->textsize=size;

	/* Flush icache, as we have changed executable code */
	/* so the various caches might be out of date */
	if (exec_debug) printk("About to flush icache\n");
	flush_icache();
	if (exec_debug) printk("Done flushing icache\n");

	if (exec_summary_debug) {
		printk("Execed process %s current_process %x pid %d "
			"allocated %dkB at %x and %dkB stack at %x\n",
			filename,(long)current_process,current_process->pid,
			size/1024,binary_start,
			stack_size/1024,stack_page);
	}

	/* r0 gets overwritten with syscall result */
	/* at end of syscall handler */
	return argc;
}

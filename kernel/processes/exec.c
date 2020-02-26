#include <stddef.h>
#include <stdint.h>

#include "memory/memory.h"
#include "memory/mmu-common.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/errors.h"
#include "lib/endian.h"
#include "lib/memcpy.h"
#include "lib/smp.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"

#include "processes/process.h"
#include "processes/exit.h"
#include "processes/bflt_loader.h"

static int exec_debug=0;
static int exec_summary_debug=0;

int32_t execve(const char *filename, char *const argv[], char *const envp[]) {

	int result,i;
	void *binary_start,*stack_page;
	int32_t argc=0;
	char *argv_location;
	int32_t argv_length=0;
	uint32_t *stack_argv;
	char *argv_ptr;
	char magic[16];
	struct file_object *file;

	uint32_t text_start,data_start,bss_start,bss_end;
	uint32_t stack_size,total_program_size,total_ondisk_size;
	uint64_t file_offset;

	if (exec_debug) printk("Entering execve\n");

	result=open_file_object(&file,filename,O_RDONLY,0);
	if (result<0) {
		if (exec_debug) {
			printk("Error %d opening %s\n",result,filename);
		}
		return result;
	}

	/* FIXME: Check if executable */

	/* FIXME: Check pemissions */

	/* See what kind of file it is */
	file_offset=0;
	result=file->file_ops->read(file->inode,
			(char *)&magic,16,&file_offset);

	/* see if a bFLT file */
	if ((magic[0]=='b') && (magic[1]=='F') &&
		(magic[2]=='L') && (magic[3]=='T')) {

		result=bflt_load(file,
			&stack_size,
			&text_start,&data_start,&bss_start,
			&bss_end,&total_ondisk_size,&total_program_size);

		current_proc[0]->datasize=bss_start-data_start;
		current_proc[0]->bsssize=bss_end-bss_start;

		/* Allocate stack */
		stack_page=memory_allocate(stack_size,MEMORY_USER);

		/* Allocate root for text/data/bss */
		/* note that memory_allocate() clears to zero */
		/* so we don't have to explicitly clear the bss */
		binary_start=memory_allocate(total_program_size,MEMORY_USER);

		if ((binary_start==NULL) || (stack_page==NULL)) {
			if (binary_start!=NULL) memory_free(binary_start,total_program_size);
			if (stack_page!=NULL) memory_free(stack_page,stack_size);
			if (exec_debug) printk("execve: no memory\n");
			return -ENOMEM;
		}

		/* Load executable */
		/* Size does not include bss */
		file_offset=text_start;
		file->file_ops->read(file->inode,
				binary_start,total_ondisk_size,&file_offset);

		/* Relocate values in the executable */
		bflt_reloc(file,binary_start);

	}
	/* Otherwise, treat as raw binary */
	else {
		if (exec_debug) printk("Assuming RAW executable!\n");

		/* Allocate stack */
		stack_size=DEFAULT_USER_STACK_SIZE;
		if (exec_debug) printk("RAW: stack size = %d\n",stack_size);

		text_start=0;
		total_ondisk_size=file->inode->size;
		total_program_size=file->inode->size;
		if (exec_debug) printk("RAW: total size = %d\n",total_program_size);

		/* Allocate stack */
		stack_page=memory_allocate(stack_size,MEMORY_USER);

		/* Allocate text memory */
		binary_start=memory_allocate(total_program_size,MEMORY_USER);

		if ((binary_start==NULL) || (stack_page==NULL)) {

			if (binary_start!=NULL) memory_free(binary_start,total_program_size);
			if (stack_page!=NULL) memory_free(stack_page,stack_size);

			if (exec_debug) printk("execve: no memory\n");
			return -ENOMEM;
		}

		/* Load executable */
		file_offset=text_start;
		file->file_ops->read(file->inode,
			binary_start,total_ondisk_size,&file_offset);
	}

	/* Done loading executable? */
	file_object_free(file);


	/************/
	/* Set name */
	/************/

	/* FIXME: strip off path before setting filename */
	strlcpy(current_proc[0]->name,filename,32);

	/*********************************/
	/* Set up command line arguments */
	/*********************************/

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

	/*******************/
	/* Setup the stack */
	/*******************/
        current_proc[0]->user_state.r[13]=(long)argv_location;
        current_proc[0]->stack=stack_page;
        current_proc[0]->stacksize=stack_size;

	/* Setup lr to point to exit */
	/* That way when a program exits it will return to where lr points */
//	current_proc[0]->reg_state.r[14]=(long)exit;

	/****************************/
	/* Make r0=argc and r1=argv */
	/****************************/
	current_proc[0]->user_state.r[0]=argc;
	current_proc[0]->user_state.r[1]=(long)argv_location;

	/*************************/
        /* Setup the entry point */
	/*************************/
        current_proc[0]->user_state.pc=(long)binary_start;
        current_proc[0]->text=binary_start;
        current_proc[0]->textsize=total_program_size;

	/* Flush icache, as we have changed executable code */
	/* so the various caches might be out of date */
	if (exec_debug) printk("About to flush icache\n");
	flush_icache();
	if (exec_debug) printk("Done flushing icache\n");

	if (exec_summary_debug) {
		printk("Execed process %s current_process %x pid %d "
			"allocated %dkB at %x and %dkB stack at %x, cpu %d\n",
			filename,(long)current_proc[0],
			current_proc[0]->pid,
			(total_program_size/1024)+4*(!!(total_program_size%1024)),
			binary_start,
			stack_size/1024,stack_page,get_cpu());
	}

	/* r0 gets overwritten with syscall result */
	/* at end of syscall handler */
	return argc;
}

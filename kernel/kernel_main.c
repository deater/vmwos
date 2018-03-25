#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"

#include "boot/hardware_detect.h"

#include "drivers/drivers.h"
#include "drivers/block/ramdisk.h"
#include "drivers/serial/serial.h"

#include "fs/files.h"

#include "memory/memory.h"

#include "processes/idle_task.h"
#include "processes/process.h"
#include "processes/userspace.h"

#include "debug/panic.h"

/* Initrd hack */
#include "../userspace/initrd.h"

#include "version.h"

void kernel_main(uint32_t r0, uint32_t r1, uint32_t r2,
		uint32_t memory_kernel) {

	(void) r0;	/* Ignore boot method */
	uint32_t rounded_memory;

	/*******************/
	/* Detect Hardware */
	/*******************/

	hardware_detect((uint32_t *)r2);

	/*****************************/
	/* Initialize Serial Console */
	/*****************************/

	/* Serial console is most important so do that first */
	serial_init(SERIAL_UART_PL011);
	serial_printk("\n\n\nUsing pl011-uart\n");

	/************************/
	/* Boot messages!	*/
	/************************/

	printk("From bootloader: r0=%x r1=%x r2=%x\n",r0,r1,r2);
	printk("\nBooting VMWos...\n");

	/* Print boot message */
	printk("\033[0;41m   \033[42m \033[44m   \033[42m \033[44m   \033[0m VMW OS\n");
	printk(" \033[0;41m \033[42m   \033[44m \033[42m   \033[44m \033[0m  Version %s\n\n",VERSION);

	printk("Detected hardware:\n");

	/* Print model info */
	hardware_print_model(r1);

	/* Print command line */
	hardware_print_commandline();

	/**************************/
	/* Init Device Drivers	  */
	/**************************/

	drivers_init_all();

	/**************************/
	/* Init Memory Hierarchy  */
	/**************************/
	/* round to 1MB granularity for mem protection reasons */
	rounded_memory=memory_kernel/(1024*1024);
	rounded_memory+=1;
	rounded_memory*=(1024*1024);
	printk("Initializing memory: kernel=0x%x bytes, rounded up to 0x%x\n",
		memory_kernel,rounded_memory);
	memory_hierarchy_init(rounded_memory);

	/************************/
	/* Other init		*/
	/************************/

	/* Init the file descriptor table */
	fd_table_init();

	/* Initialize the ramdisk */
	ramdisk_init(initrd_image,sizeof(initrd_image));

	/* Mount the ramdisk */
	mount("/dev/ramdisk","/","romfs",0,NULL);

	/* Create idle task */
	create_idle_task();

	/* Enter our "init" process */
	start_userspace("/bin/shell");

	/* we should never get here */
	printk("Error starting init!\n");

	while(1) {

		/* Loop Forever */
		/* Should probably execute a wfi instruction */
		/* In theory only here for HZ until scheduler kicks in */
	}

}

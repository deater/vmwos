#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/smp.h"

#include "boot/hardware_detect.h"
#include "boot/smp_boot.h"

#include "drivers/drivers.h"
#include "drivers/ramdisk/ramdisk.h"
#include "drivers/serial/serial.h"
#include "drivers/console/console_io.h"

#include "drivers/audio/audio.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"

#include "memory/memory.h"

#include "processes/idle_task.h"
#include "processes/process.h"
#include "processes/userspace.h"

#include "debug/panic.h"

/* Initrd hack */
#include "../userspace/initrd.h"
#include "../userspace/initrd2.h"

#include "version.h"

void kernel_main(uint32_t r0, uint32_t r1, uint32_t r2,
		uint32_t memory_kernel) {

	(void) r0;	/* Ignore boot method */
	uint32_t rounded_memory;
	struct block_dev_type *dev;

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
	/* Enable floating point  */
	/**************************/
	/* FIXME: where to put this? */
	printk("Enabling FPU...\n");

#ifdef ARMV7
	/* Pi3 (Cortex A53) FPU enable */
	asm volatile(   "mrc p15, 0, r0, c1, c1, 2 @ Enable nonsecure to cp10/cp11\n"
			"orr r0, r0, #3<<10\n"
			"mcr p15, 0, r0, c1, c1, 2\n"
			"mrc p15,0,r0,c1,c0, #2 @ Access Control Register\n"
			"orr r0, #(0x300000 + 0xC00000)	@  Enable Single & Double Precision\n"
			"mcr p15,0,r0,c1,c0, #2	@ Access Control Register\n"
			"mov r0, #0x40000000 @ Enable VFP\n"
			"vmsr fpexc,r0\n"
                : : : "r0");
#else
	/* ARM1176 FPU enable */
	asm volatile(   "mrc p15, #0, r0, c1, c0, #2\n"
			"orr r0, r0, #0xf00000 @ Single + double precision\n"
			"mcr p15, #0, r0, c1, c0, #2\n"
			"mov r0, #0x40000000   @ Set VFP enable bit\n"
			"fmxr fpexc, r0\n"
                : : : "r0");

#endif

	/**************************/
	/* Init Device Drivers	  */
	/**************************/
	drivers_init_all();

	printk("Trying audio\n");
	audio_pwm_init();
	audio_beep();
	printk("Done trying audio\n");


	/**************************/
	/* SMP Boot               */
	/**************************/
#ifdef ARMV7
	printk("Starting multi-core:\n");
	smp_boot();
	console_enable_locking();
#endif

	/************************/
	/* Other init		*/
	/************************/

	/* Init the file descriptor table */
	file_objects_init();

	/* Setup first ramdisk */
	dev=ramdisk_init(initrd_image,sizeof(initrd_image));
	if (dev!=NULL) {
		mount_syscall("ramdisk0","/","romfs",0,NULL);
	}

	/* Setup the second ramdisk */
	dev=ramdisk_init(initrd2_image,sizeof(initrd2_image));
	if (dev!=NULL) {
		mount_syscall("ramdisk1","/mnt","dos33fs",0,NULL);
	}

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

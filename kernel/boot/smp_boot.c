#include <stdint.h>

#include "boot/smp_boot.h"
#include "lib/printk.h"
#include "lib/mmio.h"

/* Boot up secondary cores */

void secondary_boot_c(int core) {

	/* Set up cache */

	printk("Booting core %d\n",core);

	while(1) {

	}

}

/* For now, assume 4 cores */
#define NUMCORES	4

void smp_boot(void) {

	int i;
	uint32_t start_core_addr,mailbox_addr;

	start_core_addr=(uint32_t)(&start_core);

	/* Write the boot routine to the wakeup mailboxes */
	/* The Pi firmware parks the extra cores here waiting for */
	/* This address to change to a location to jump to */
	/* core 1: 0x4000009C */
	/* core 2: 0x400000AC */
	/* core 3: 0x400000BC */


	for(i=1;i<NUMCORES;i++) {
		mailbox_addr=0x4000008c+(i*0x10);
		printk("Writing %x to mailbox %x\n",start_core_addr,mailbox_addr);
		mmio_write(mailbox_addr,start_core_addr);

	}
	printk("Done SMP init from core0\n");
}

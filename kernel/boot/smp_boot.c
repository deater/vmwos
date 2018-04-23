#include <stdint.h>

#include "boot/smp_boot.h"

#include "lib/printk.h"
#include "lib/mmio.h"
#include "lib/smp.h"

#include "interrupts/ipi.h"

#include "memory/memory.h"
#include "memory/mmu-common.h"

static volatile uint32_t core_booted[NUM_CORES];

/* Boot up secondary cores */

void secondary_boot_c(int core) {

	/* Set up cache */
	enable_mmu(0);

	printk("Core %d MMU enabled\n",core);

//	invalidate_l1_dcache();

	core_booted[core]=1;

//	invalidate_l1_dcache();

//	flush_dcache((uint32_t)&core_booted,
//		((uint32_t)&core_booted)+sizeof(core_booted));

//	printk("Booted core %d, %d %d %d\n",
//		core,
//		core_booted[1],
//		core_booted[2],
//		core_booted[3]);

	/* Low power infinite loop for now */
	/* Busy looping makes the GPU complain */
	while(1) {
		asm volatile("wfi\n"
			: : : "memory");
	}

}



void smp_boot(void) {

	int i,timeout;
	uint32_t start_core_addr,mailbox_addr;

	start_core_addr=(uint32_t)(&start_core);

	/* Write the boot routine to the wakeup mailboxes */
	/* The Pi firmware parks the extra cores here waiting for */
	/* This address to change to a location to jump to */
	/* core 1: 0x4000009C */
	/* core 2: 0x400000AC */
	/* core 3: 0x400000BC */


	for(i=1;i<NUM_CORES;i++) {
		mailbox_addr=0x4000008c+(i*0x10);
		printk("\tWriting %x to mailbox %x\n",
			start_core_addr,mailbox_addr);
		mmio_write(mailbox_addr,start_core_addr);

		printk("\tWaiting for core %d to become ready\n",i);

		/* Send event -- wake other cores sleeping in WFE */
		asm volatile("sev\n"  // SEV
			: : : "memory");

//		printk("\tCore0 waiting for core %i to become ready (%d %d %d)\n",
//			i,core_booted[1],core_booted[2],core_booted[3]);
		timeout=0;

//		flush_dcache((uint32_t)&core_booted,
//			((uint32_t)&core_booted)+sizeof(core_booted));

//		invalidate_l1_dcache();

		while(core_booted[i]==0) {
			timeout++;
			if (timeout>1000000000) {
				printk("\tCore0 timed out waiting for "
					"core%d!\n",i);
				break;
			}
		}

	}


	printk("Done SMP init: ");
	for(i=0;i<NUM_CORES;i++) {
		printk("core%d=%d ",i,core_booted[i]);
	}
	printk("\n");

	/* Enable IPI interrupts */
	ipi_enable();


}

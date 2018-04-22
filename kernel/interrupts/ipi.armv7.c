/* Code for sending inter-processor interrupts */

#include <stdint.h>

#include "interrupts/ipi.h"

#include "lib/mmio.h"
#include "lib/errors.h"
#include "lib/printk.h"

#define NUM_CPUS	4

static uint32_t mailbox_set[NUM_CPUS]={
	0x40000080,
	0x40000090,
	0x400000a0,
	0x400000b0,
};

static uint32_t mailbox_clear[NUM_CPUS]={
	0x400000c0,
	0x400000d0,
	0x400000e0,
	0x400000f0,
};

static uint32_t mailbox_irq_enable[NUM_CPUS]={
	0x40000050,
	0x40000054,
	0x40000058,
	0x4000005c,
};


int32_t send_ipi(int32_t which) {

	if ((which<0) || (which>=NUM_CPUS)) {
		return -EINVAL;
	}

	mmio_write(mailbox_set[which],1);

	return 0;

}

int32_t ipi_enable(void) {
	int i;

	/* Enable mailbox0 irq */
	for(i=0;i<NUM_CPUS;i++) {
		mmio_write(mailbox_irq_enable[i],1);
	}
	return 0;
}

int32_t ipi_interrupt_handler(int32_t core) {

	uint32_t value;

	printk("Core%d received IPI interrupt!\n",core);

	/* Clear interrupt */
	/* Can read value, then writing same value back */
	/* clears it because it is set-high-to-clear */
	value=mmio_read(mailbox_clear[core]);
        mmio_write(mailbox_clear[core],value);

        return 0;
}


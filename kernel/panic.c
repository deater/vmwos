#include <stdint.h>

#include "process.h"
#include "lib/printk.h"

int32_t dump_state(void) {

	int i;

	printk("Kernel panic!\n");
	printk("pc: %x\tcpsr: %x\n",process[current_process].reg_state.lr,
		process[current_process].reg_state.spsr);
	/* TODO: stack dump */

	for(i=0;i<8;i++) {
		printk("r%d: %x\t",i,process[current_process].reg_state.r[i]);
		if (i!=7) printk("r%d: %x",
			i+8,process[current_process].reg_state.r[i+8]);
		printk("\n");
	}

	return 0;
}

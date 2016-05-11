#include <stdint.h>

#include "process.h"
#include "lib/printk.h"

int32_t dump_state(void) {

	int i;

	printk("Kernel panic! %s\n",current_process->name);
	printk("pc: %x\tcpsr: %x\n",current_process->reg_state.lr,
		current_process->reg_state.spsr);
	/* TODO: stack dump */

	for(i=0;i<8;i++) {
		printk("r%d: %x\t",i,current_process->reg_state.r[i]);
		if (i!=7) printk("r%d: %x",
			i+8,current_process->reg_state.r[i+8]);
		printk("\n");
	}

	return 0;
}

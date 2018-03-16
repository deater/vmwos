#include <stdint.h>

#include "processes/process.h"
#include "lib/printk.h"

void dump_saved_user_state(struct process_control_block_type *proc) {

	int i;

	printk("Process %d dump! %s\n",proc->pid,proc->name);
	printk("pc: %x\tcpsr: %x\n",proc->user_state.pc,
		proc->user_state.spsr);
	/* TODO: stack dump */

	for(i=0;i<8;i++) {
		printk("r%d: %x\t",i,proc->user_state.r[i]);
		if (i!=7) printk("r%d: %x",
			i+8,proc->user_state.r[i+8]);
		printk("\n");
	}


}

void dump_saved_kernel_state(struct process_control_block_type *proc) {

	int i;

	printk("Kernel %d dump! %s\n",proc->pid,proc->name);

	/* TODO: stack dump */

	for(i=0;i<8;i++) {
		printk("r%d: %x\t",i,proc->kernel_state.r[i]);
		if (i!=7) printk("r%d: %x",
			i+8,proc->kernel_state.r[i+8]);
		printk("\n");
	}


}

void dump_kernel_regs(void) {

	register long r0 asm ("r0");
	register long r1 asm ("r1");
	register long r2 asm ("r2");
	register long r3 asm ("r3");
	register long r4 asm ("r4");
	register long r5 asm ("r5");
	register long r6 asm ("r6");
	register long r7 asm ("r7");
	register long r8 asm ("r8");
	register long r9 asm ("r9");
	register long r10 asm ("10");
	register long r11 asm ("r11");
	register long r12 asm ("r12");
	register long r13 asm ("r13");
	register long r14 asm ("r14");
	printk("Kernel register dump: PC=%x\n",r14);
	printk("r0: %x r1: %x r2: %x r3: %x\n",r0,r1,r2,r3);
	printk("r4: %x r5: %x r6: %x r7: %x\n",r4,r5,r6,r7);
	printk("r8: %x r9: %x r10: %x r11: %x\n",r8,r9,r10,r11);
	printk("r12: %x r13: %x r14: %x\n",r12,r13,r14);

}

void dump_memory(uint32_t address,uint32_t size) {

	unsigned char *ptr;
	int i;

	ptr=(unsigned char *)address;

	for(i=0;i<size;i++) {
		if ((i&0xf)==0) {
			printk("\n%x: ",ptr);
		}
		printk("%x ",*ptr);
		ptr++;
	}
}


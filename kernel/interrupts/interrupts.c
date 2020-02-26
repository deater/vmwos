#include <stddef.h>
#include <stdint.h>

#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/keyboard/ps2-keyboard.h"
#include "drivers/timer/timer.h"
#include "drivers/serial/serial.h"

#include "lib/printk.h"
#include "lib/mmio.h"
#include "lib/smp.h"

#include "time/time.h"
#include "processes/scheduler.h"
#include "processes/exit.h"
#include "processes/process.h"

#include "interrupts/interrupts.h"
#include "interrupts/ipi.h"


#define MAX_IRQ	64

int irq_enable(int which_one) {

	uint32_t address_offset,bit,old;

        if (which_one>MAX_IRQ) {
                printk("IRQ%d too big\n",which_one);
                return -1;
        }

        bit=1<<(which_one&0x1f);
        address_offset=(which_one/32)*4;

        old=bcm2835_read(IRQ_ENABLE_IRQ1+address_offset);
        bcm2835_write(IRQ_ENABLE_IRQ1+address_offset,old|bit);

        return 0;
}


int irq_disable(int which_one) {

	uint32_t address_offset,bit,old;

        if (which_one>MAX_IRQ) {
                printk("IRQ%d too big\n",which_one);
                return -1;
        }

        bit=1<<(which_one&0x1f);
        address_offset=(which_one/32)*4;

        old=bcm2835_read(IRQ_DISABLE_IRQ1+address_offset);
        bcm2835_write(IRQ_DISABLE_IRQ1+address_offset,old|bit);

        return 0;

}



static void user_reg_dump(void) {

	unsigned long regs[15];
	unsigned long saved_sp;
	int i;

	asm volatile(
		"str sp,%[saved_sp]\n"
                "mov sp,%[regs]\n"
                "stmia sp, {r0 - lr}^\n"        /* the ^ means load user regs */
		"ldr sp,%[saved_sp]\n"
                : [saved_sp]"=m"(saved_sp)/* output */
                :       [regs] "r"(regs) /* input */
                : "sp", "memory" /* clobbers */
                        );

	printk("Process: %d (text %p+%x, stack %p+%x)\n",
		current_proc[get_cpu()]->pid,
		current_proc[get_cpu()]->text,
		current_proc[get_cpu()]->textsize,
		current_proc[get_cpu()]->stack,
		current_proc[get_cpu()]->stacksize);

	for(i=0;i<8;i++) {
		printk("r%02d: %08x\t",i,regs[i]);
		if (i!=7) printk("r%02d: %08x",
			i+8,regs[i+8]);
		printk("\n");
	}
}


void interrupt_handler_c(uint32_t r0, uint32_t r1) {

	uint32_t basic_pending,pending1,pending2;
	uint32_t handled=0;

	/**************************************/
	/* First check basic_pending register */
	/**************************************/
	basic_pending=bcm2835_read(IRQ_BASIC_PENDING);

	if (basic_pending & IRQ_BASIC_PENDING_IRQ57) {
		handled++;
		serial_interrupt_handler();
	}
	if (basic_pending & IRQ_BASIC_PENDING_TIMER) {
		handled++;
	}

	/*************************************/
	/* Next check pending1 register      */
	/*************************************/
	pending1=bcm2835_read(IRQ_PENDING1);
	if ((pending1) && (!handled)) {
		printk("Unknown pending1 interrupt %x\n",pending1);
	}

	/*************************************/
	/* Next check pending2 register      */
	/*************************************/
	pending2=bcm2835_read(IRQ_PENDING2);

	if (pending2) {
		// Check if GPIO23 (ps2 keyboard) (irq49)
		if (pending2 & IRQ_PENDING2_IRQ49) {
			handled++;
			ps2_interrupt_handler();
		}

		if (!handled) {
			printk("Unknown pending2 interrupt %x\n",pending2);
		}
	}


#ifdef ARMV7
/*
Address: 0x4000_0060 Core0 interrupt source
Address: 0x4000_0064 Core1 interrupt source
Address: 0x4000_0068 Core2 interrupt source
Address: 0x4000_006C Core3 interrupt source
Reset: 0x0000_0000
Bits	Description
31-28	<Reserved>
17:12	Peripheral 1..15 interrupt (Currently not used)
11	Local timer interrupt
10	AXI-outstanding interrupt <For core 0 only!> all others are 0
9	PMU interrupt
8	GPU interrupt <Can be high in one core only>
7	Mailbox 3 interrupt
6	Mailbox 2 interrupt
5	Mailbox 1 interrupt
4	Mailbox 0 interrupt
3	CNTVIRQ interrupt
2	CNTHPIRQ interrupt
1	CNTPNSIRQ interrupt
0	CNTPSIRQ interrupt (Physical Timer -1)
*/
	uint32_t per_core[4],i;

	per_core[0]=mmio_read(0x40000060);
	per_core[1]=mmio_read(0x40000064);
	per_core[2]=mmio_read(0x40000068);
	per_core[3]=mmio_read(0x4000006c);

	for(i=0;i<4;i++) {
		if (per_core[i]==0) continue;

		/* GPU interrupt, meaning basic_pending/pending1/pending2 */
		/* Should have handled this */
		if (per_core[i]&0x100) {
			if (!handled) {
				printk("Unhandled GPU interrupt core %d "
					"%x %x %x "
					"(possibly power sag)\n",
					i,basic_pending,pending1,pending2);
			}
			/* IPI interrupt */
		} else if (per_core[i]&0x10) {
			ipi_interrupt_handler(i);
			handled++;
		}
		else {
			printk("Unknown core %d interrupt %x\n",
				i,per_core[i]);
		}
	}

#endif

	if (!handled) {
		printk("Unknown interrupt!\n");
	}

	// check if it's a timer interrupt
	if (basic_pending & IRQ_BASIC_PENDING_TIMER) {
//		printk("About to handle timer interrupt\n");
		timer_interrupt_handler();
//		printk("Returned from timer interrupt\n");
		if (scheduling_enabled) schedule();
	}

//	printk("Exiting interrupt_c\n");

	return;
}


void __attribute__((interrupt("FIQ"))) fiq_handler(void) {
	printk("UNHANDLED FIQ\n");
}

/* 1415 */
void __attribute__((interrupt("ABORT"))) data_abort_handler(void) {
	uint32_t dfsr,dfar,fs;
	register long lr asm ("lr");
	uint32_t *code;

	code=(uint32_t *)(lr-8);

	printk("MEMORY ABORT at PC=%x (%x)\n",lr-8,*code);

	/* Read DFSR reg (see B4.1.52) */
	asm volatile("mrc p15, 0, %0, c5, c0, 0" : "=r" (dfsr) : : "cc");

	/* Read DFAR reg (see B4.1.52) */
	asm volatile("mrc p15, 0, %0, c6, c0, 0" : "=r" (dfar) : : "cc");

	fs=dfsr&0xf;

	if (fs==1) printk("\tAlignment fault\n");
	if (fs==2) printk("\tDebug event\n");
	if ((fs&0xd)==0xd) printk("\tPermission fault accessing %x\n",dfar);

	user_reg_dump();

	exit(-1);

}

/* 1637 */
void __attribute__((interrupt("ABORT"))) prefetch_abort_handler(void) {
	uint32_t ifsr,ifar,fs;
	register long lr asm ("lr");

	printk("PREFETCH ABORT at PC=%x\n",lr-4);

	/* Read IFSR reg (see B4.1.96) */
	asm volatile("mrc p15, 0, %0, c5, c0, 1" : "=r" (ifsr) : : "cc");

	/* Read IFAR reg (see B4.1.96) */
	asm volatile("mrc p15, 0, %0, c6, c0, 2" : "=r" (ifar) : : "cc");

	fs=ifsr&0xf;

	if (fs==2) printk("\tDebug event\n");
	if ((fs&0xd)==0xd) printk("\tPermission fault accessing %x\n",ifar);

	user_reg_dump();

	exit(-1);

}


void __attribute__((interrupt("UNDEF"))) undef_handler(void) {

	register long lr asm ("lr");

	long *ptr=(long *)lr;

	/* point back 4 bytes */
	ptr--;

	printk("UNDEFINED INSTRUCTION, PC=%x, insn=%x\n",lr-4,*ptr);

}


#include <stddef.h>
#include <stdint.h>
#include "bcm2835_periph.h"
#include "mmio.h"
#include "printk.h"
#include "time.h"
#include "scheduler.h"

#include "ps2-keyboard.h"
#include "uart.h"
#include "timer.h"

#define MAX_IRQ	64

int irq_enable(int which_one) {

	uint32_t address_offset,bit,old;

        if (which_one>MAX_IRQ) {
                printk("IRQ%d too big\n",which_one);
                return -1;
        }

        bit=1<<(which_one&0x1f);
        address_offset=(which_one/32)*4;

        old=mmio_read(IRQ_ENABLE_IRQ1+address_offset);
        mmio_write(IRQ_ENABLE_IRQ1+address_offset,old|bit);

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

        old=mmio_read(IRQ_DISABLE_IRQ1+address_offset);
        mmio_write(IRQ_DISABLE_IRQ1+address_offset,old|bit);

        return 0;

}

void user_reg_dump(void) {

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

	for(i=0;i<15;i++) {
		printk("reg%d = %x ",i,regs[i]);
	}
	printk("\n");
}



void interrupt_handle_unknown(int which) {

	printk("Unknown interrupt happened %x!\n",which);
}

void interrupt_handler_c(uint32_t saved_sp) {

	uint32_t basic_pending,pending2;
	uint32_t handled=0;

	// Check if GPIO23 (ps2 keyboard) (irq49)
	pending2=mmio_read(IRQ_PENDING2);

	if (pending2 & IRQ_PENDING2_IRQ49) {
		handled++;
		ps2_interrupt_handler();
	}

	// Check if UART (irq57)
	basic_pending=mmio_read(IRQ_BASIC_PENDING);

	if (basic_pending & IRQ_BASIC_PENDING_IRQ57) {
		handled++;
		uart_interrupt_handler();
	}

	// check if it's a timer interrupt
	if (basic_pending & IRQ_BASIC_PENDING_TIMER) {
		handled++;
	}
	else {
		if (!handled) {
			interrupt_handle_unknown(basic_pending);
		}
		return;
	}

	// timer_interrupt

	timer_interrupt_handler();

//	schedule((long *)saved_sp);

	return;
}


void __attribute__((interrupt("FIQ"))) fiq_handler(void) {
	printk("UNHANDLED FIQ\n");
}

void __attribute__((interrupt("ABORT"))) abort_handler(void) {
	printk("UNHANDLED ABORT\n");
}

void __attribute__((interrupt("UNDEF"))) undef_handler(void) {
	printk("UNHANDLED UNDEF\n");
}


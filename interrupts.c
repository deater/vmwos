#include <stdint.h>
#include "bcm2835_periph.h"
#include "mmio.h"
#include "led.h"
#include "printk.h"
#include "time.h"
#include "scheduler.h"

/* global variable */
int blinking_enabled=1;
int tick_counter=0;


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
	printk("\r\n");
}


void interrupt_handle_timer(void) {

	static int lit = 0;

	/* Clear the ARM Timer interrupt		*/
	/* Since it's the only one we have enabled,	*/
	/* Assume it is what caused the interrupt.	*/
	/* FIXME: do proper detection of IRQ src	*/

	mmio_write(TIMER_IRQ_CLEAR,0x1);
	tick_counter++;

	if (blinking_enabled) {

		/* Flip the LED */
		if( lit ) {
			led_off();
			lit = 0;
		}
		else {
			led_on();
			lit = 1;
		}
	}

}

#if 0
void __attribute__((interrupt("IRQ"))) interrupt_handler_old(void) {


	long saved_sp;
	long *our_sp=&(process[current_process].reg_state.r[0]);

	asm volatile(
		"str sp,%[saved_sp]\n"
		"mov sp,%[our_sp]\n"
		"stmia sp, {r0 - lr}^\n"        /* the ^ means load user regs */
		"ldr sp,%[saved_sp]\n"
		: [saved_sp] "=m" (saved_sp) /* output */
		:       [our_sp] "r"(our_sp) /* input */
		: "memory" /* clobbers */
		);


	long entry_pc;

        asm volatile(
                "mov      %[entry_pc], lr\n"
                : [entry_pc]"=r"(entry_pc)
                :
                :
                );

	long entry_spsr,entry_sp;
        asm volatile(
                "mov      %[entry_sp], sp\n"
                : [entry_sp]"=r"(entry_sp)
                :
                :
                );

        asm volatile(
                "MRS      %[entry_spsr], spsr\n"
                : [entry_spsr]"=r"(entry_spsr)
                :
                :
                );

        printk("IRQ PC=%x SPSR=%x SP=%x\r\n",entry_pc,entry_spsr,entry_sp);
	user_reg_dump();


	static int lit = 0;

	/* Clear the ARM Timer interrupt		*/
	/* Since it's the only one we have enabled,	*/
	/* Assume it is what caused the interrupt.	*/
	/* FIXME: do proper detection of IRQ src	*/

	mmio_write(TIMER_IRQ_CLEAR,0x1);
	tick_counter++;



	if (blinking_enabled) {

		/* Flip the LED */
		if( lit ) {
			led_off();
			lit = 0;
		}
		else {
			led_on();
			lit = 1;
		}
	}

	/* Schedule.  We might not return */
	schedule(entry_pc);

}
#endif

void __attribute__((interrupt("FIQ"))) fiq_handler(void) {
	printk("UNHANDLED FIQ\r\n");
}

void __attribute__((interrupt("ABORT"))) abort_handler(void) {
	printk("UNHANDLED ABORT\r\n");
}

void __attribute__((interrupt("UNDEF"))) undef_handler(void) {
	printk("UNHANDLED UNDEF\r\n");
}

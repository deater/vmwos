#include <stdint.h>
#include "lib/printk.h"
#include "lib/delay.h"

void idle_task(void) {

//	printk("Idle Task!\n");

	while(1) {
//		printk("Idle\n");
//		delay(1000);

		/* Interesting, see */
		/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13332.html */
		/* On ARM1176 wfi is a nop */
		/* Use the traditional MCR p15, 0, Rn, c7, c0, 4 instead */
		/* Rn value SBZ should be zero */
		/* Works by having the interrupt handler return +8 instead */
		/* of +4 */
//		asm volatile("MCR p15, 0, %0, c7, c0, 4\n" ::"r"(0):);
		asm volatile("wfi\n" :::);

	}

//	asm volatile("idle_loop:\n"
//			"wfi\n"
//			"b idle_loop\n"
//			:::);

//	asm volatile("idle_loop:\n"
//			"nop\n"
//			"b idle_loop\n"
//			:::);
}

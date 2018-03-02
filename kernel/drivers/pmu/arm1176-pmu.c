/* Code for the Performance Monitoring Counters on ARM1176 */

#include <stdint.h>

#include "drivers/pmu/arm-pmu.h"

#include "lib/printk.h"

int pmu_init(void) {

	uint32_t control;

	control=0;
//	control|=((event1&0xff)<<20);  /* evtcount0 */
//	control|=((event2&0xff)<<12);  /* evtcount1 */
	/* x = 0 */
	/* CCR overflow interrupts = off = 0 */
	/* 0 */
	/* ECC overflow interrupts = off = 0 */
	/* D div/64 = 0 = off */
	control|=(1<<2); /* reset cycle-count register */
	control|=(1<<1); /* reset count registers */
	control|=(1<<0); /* start counters */

	asm volatile("mcr p15, 0, %0, c15, c12, 0\n"
		: "+r" (control));

	return 0;
}

uint32_t read_cycle_counter(void) {

	uint32_t cycles;

	asm volatile("mrc p15, 0, %0, c15, c12, 1\n"
		: "=r" (cycles));

	return cycles;

}


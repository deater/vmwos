#include <stdint.h>

#include "lib/smp.h"


int32_t get_cpu(void) {

	int32_t which=0;

#ifdef ARMV7
	/* Read MPIDR (Multiproc Affinity Register) */
	/* CPU ID is Bits 0..1 */
	asm volatile("mrc	p15, 0, %0, c0, c0, 5"
			: "=r" (which):: "cc");
	which&=0x3;
#else

#endif
	return which;
}

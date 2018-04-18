#include <stdint.h>
#include <stddef.h>

#include "lib/errors.h"

int getcpu(uint32_t *cpu, uint32_t *node, void *tcache) {

	uint32_t cpunum=0;

	if (cpu!=NULL) {
#ifdef ARMV7
		/* get CPU number from MPIDR */
		asm volatile("mrc	p15, 0, %0, c0, c0, 5\n"
			: "=r" (cpunum) : : "cc");
		cpunum&=0x3;
#else
		/* Only supports one CPU */
		*node=cpunum;
#endif
	}

	if (node!=NULL) {
		*node=0;
	}

	if (tcache!=NULL) {
		return -EINVAL;
	}

	return 0;

}

#include <stdint.h>

#include "lib/memory_barriers.h"

void isb(void) {

#ifdef ARM1176
	asm volatile ("mcr p15, 0, %0, c7,  c5, 4"
				:
				: "r" (0)
				: "memory");
#else
	asm volatile ("isb" ::: "memory");
#endif
}

void dmb(void) {

#ifdef ARM1176
	asm volatile ("mcr p15, 0, %0, c7, c10, 5"
				:
				: "r" (0)
				: "memory");
#else
	asm volatile ("dmb" ::: "memory");
#endif
}


/* Data synchronization barrier / drain write barrier */
void dsb(void) {
#ifdef ARM1176
	asm volatile ("mcr p15, 0, %0, c7, c10, 4"
				:
				: "r" (0)
				: "memory");
#else
	asm volatile ("dsb" ::: "memory");
#endif
}





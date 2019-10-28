#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"
#include "vlibc.h"

/* Pseudo-Random Number Generator */
/* Linear feedback */
/* FIXME: can this ever return 0? */
int32_t rand(void) {
        /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
        static uint32_t x = 0xfeb13;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return x;
}

#if 0

int32_t rand(void) {

	uint32_t buffer;
	int32_t result;

	result=vmwos_random(&buffer);

	if (result<4) {
		printf("Error!\n");
		return -1;
	}

	return buffer;
}
#endif

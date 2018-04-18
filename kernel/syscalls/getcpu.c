#include <stdint.h>
#include <stddef.h>

#include "lib/errors.h"

int getcpu(uint32_t *cpu, uint32_t *node, void *tcache) {

	if (cpu!=NULL) {
		*node=0;
	}

	if (node!=NULL) {
		*node=0;
	}

	if (tcache!=NULL) {
		return -EINVAL;
	}

	return 0;

}

/* Code for sending inter-processor interrupts */

#include <stdint.h>

#include "lib/errors.h"

int32_t send_ipi(int32_t which) {

	if ((which<0) || (which>3)) {
		return -EINVAL;
	}

	

	return 0;

}

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"
#include "vlibc.h"

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

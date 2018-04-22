#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <errno.h>
#include <string.h>
#endif

int main(int argc, char **argv) {

	int which=0;

	if (argc>1) {
		which=atoi(argv[1]);
	}

	printf("Poking core%d\n",which);

	vmwos_core_poke(which);

	return 0;
}

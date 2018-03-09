#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#endif

int main(int argc, char **argv) {

	char dirname[256];

	getcwd(dirname,256);

	printf("%s\n",dirname);

	return 0;
}

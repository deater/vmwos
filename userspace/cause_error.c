#include <stddef.h>
#include <stdint.h>

#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"

static int print_help(char *prog_name) {

	printf("%s [cpsr] [kernelmem] [memtest]\n");
	printf("* cpsr -- try to modify CPSR (should fail)\n");
	printf("* kernelmem -- try writing to kernel memory\n");
	printf("* memtest -- TODO\n");

	return 0;
}

int main(int argc, char **argv) {

	if (argc<2) {
		print_help(argv[0]);
		return -1;
	}

	if (!strncmp(argv[1],"cpsr",4)) {
		printf("Trying to change cpsr from userspace\n");
	}
	else if (!strncmp(argv[1],"kernelmem",9)) {
		printf("Trying to write kernel from userspace\n");
	}
	else if (!strncmp(argv[1],"memtest",7)) {
		printf("memtest not implemented yet\n");
	}
	else {
		printf("Unknown command %s\n",argv[1]);
	}

	return 0;
}

#include <stddef.h>
#include <stdint.h>

#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"

static int print_help(char *prog_name) {

	printf("%s [cpsr] [kernelwrite] [kernelread] [kernelexec] [undefined]\n",prog_name);
	printf("* cpsr -- try to modify CPSR\n");
	printf("* kernelwrite -- try writing to kernel memory\n");
	printf("* kernelread -- try reading kernel memory\n");
	printf("* kernelexec -- try jumping to kernel memory\n");
	printf("* undefined -- try running undefined instruction\n");

	return 0;
}

static inline uint32_t get_CPSR(void) {

	uint32_t temp;

	asm volatile ("mrs %0,CPSR":"=r" (temp):) ;

	return temp;
}

static inline void set_CPSR(uint32_t new_cpsr) {
	asm volatile ("msr CPSR_cxsf,%0"::"r"(new_cpsr) );
}


int main(int argc, char **argv) {

	int32_t temp,i;
	char *ptr;
	int32_t number=0;
	int32_t *int_ptr;

	if (argc<2) {
		print_help(argv[0]);
		return -1;
	}

	if (argc==3) {
		number=atoi(argv[2]);
	}


	if (!strncmp(argv[1],"cpsr",4)) {

		printf("Trying to change cpsr from userspace\n");
		temp=get_CPSR();
		printf("Read CPSR as %x\n",temp);
		printf("Writing back CPSR\n");
		set_CPSR(temp);

		uint32_t reg;
		asm("mrc p15, 0, %0, c0, c0, 1" : "=r" (reg) : : "cc");
		printf("Trying to read cache config %x\n",reg);

	}
	else if (!strncmp(argv[1],"undefined",9)) {
		/* arm / thumb illegal instruction */
		asm volatile (".word 0xe7f0def0\n");
	}
	else if (!strncmp(argv[1],"kernelwrite",9)) {
		ptr=(char *)0x8000;
		if (number==0) number=65536;
		printf("Trying to write kernel from userspace, %d @ %x\n",
			number,ptr);
		for(i=0;i<number;i++) {
			*ptr='v';
			ptr++;
		}
	}
	else if (!strncmp(argv[1],"kernelread",9)) {
		int_ptr=(int32_t *)0x8000;
		printf("Trying to read kernel from userspace, [%x]= %x\n",
			int_ptr,*int_ptr);
	}
	else if (!strncmp(argv[1],"kernelexec",9)) {
		printf("Trying to jump to address 0x8000\n");
		((void (*)(void))0x8000)();
	}
	else {
		printf("Unknown command %s\n",argv[1]);
	}

	return 0;
}

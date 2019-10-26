#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <unistd.h>
#endif

#include "svmwgraph.h"
#include "pi-graphics.h"
#include "demosplash2019.h"

extern unsigned char _binary_pi_logo_pcx_start[];
extern unsigned char _binary_pi_logo_pcx_end[];

int boot_intro(unsigned char *buffer, struct palette *pal) {

	unsigned char *pi_logo=_binary_pi_logo_pcx_start;
	int filesize=_binary_pi_logo_pcx_end-_binary_pi_logo_pcx_start;

	printf("Image is %d bytes\n",filesize);
	vmwPCXLoadPalette(pi_logo, filesize-769, pal);
	vmwLoadPCX(pi_logo,100,100, buffer);

//	print_string("Testing 123!",10,10,0xff,buffer);
	pi_graphics_update(buffer,pal);

	sleep(5);

	return 0;
}

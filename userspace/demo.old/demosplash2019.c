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

static struct palette pal;

static unsigned char buffer[XSIZE*YSIZE];

int main(int argc, char **argv) {

	pi_graphics_init();

	vmwClearScreen(0,buffer);

	vmwSetAllBlackPalette(&pal);

	pi_graphics_update(buffer,&pal);

	/* Do the VMW Software Production Logo */
	vmwos_open(buffer,&pal);

	/* init console */
	console_init(&pal);

	/* Do the boot screen fakeout */
	boot_intro(buffer,&pal);

	/* temporary mode7 */
	mode7_flying(buffer, &pal);

	/* Draw the credits */
	doom_fire(buffer,&pal);

	return 0;
}



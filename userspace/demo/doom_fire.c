/* Based on Doom Fire as described by FABIEN SANGLARD */
/* http://fabiensanglard.net/doom_fire_psx/ */

#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#endif

#include "svmwgraph.h"
#include "pi-graphics.h"
#include "demosplash2019.h"

static void vmwClearTopScreen(int color, unsigned char *buffer) {

        memset(buffer,color,XSIZE*200*sizeof(unsigned char));

}

/* The state word must be initialized to non-zero */
static uint32_t rand32(void) {
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	static uint32_t x = 0xfeb13;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return x;
}



void doom_fire(unsigned char *buffer, struct palette *pal) {

	int x,y,newcol,r,dst;
	int count=0,credit=0,texty=445;

	int rgbs[38]={
		0x000000,
                0x070707,0x1F0707,0x2F0F07,0x470F07,
		0x571707,0x671F07,0x771F07,0x8F2707,
		0x9F2F07,0xAF3F07,0xBF4707,0xC74707,
                0xDF4F07,0xDF5707,0xDF5707,0xD75F07,
                0xD75F07,0xD7670F,0xCF6F0F,0xCF770F,
                0xCF7F0F,0xCF8717,0xC78717,0xC78F17,
		0xC7971F,0xBF9F1F,0xBF9F1F,0xBFA727,
		0xBFA727,0xBFAF2F,0xB7AF2F,0xB7B72F,
		0xB7B737,0xCFCF6F,0xDFDF9F,0xEFEFC7,
                0xFFFFFF,
	};

	vmwClearScreen(0,buffer);

	for(x=0;x<38;x++) {
		pal->red[x]=(rgbs[x]>>16)&0xff;
		pal->green[x]=(rgbs[x]>>8)&0xff;
		pal->blue[x]=(rgbs[x])&0xff;
	}

	/* draw white line at bottom */
	vmwHlin(0,640,479,37,buffer);



	while(1) {
		vmwClearTopScreen(0, buffer);

		for(x=0;x<640;x++) {
			for(y=200;y<479;y++) {
				r=rand32()&7;
				dst=(y*XSIZE)+x-(r&3)+1;
//				if (dst>=640*480) dst=(640*480)-1;
				newcol=buffer[((y+1)*XSIZE)+x]-(r<2);
				if (newcol<0) newcol=0;
				buffer[dst]=newcol;
			}
		}

		if (credit==0) {
			texty-=1;
			if (texty==120) credit=1;
		}
			vmwTextXYx2("A VMW SOFTWARE PRODUCTION",60*2,texty,
				15,15,0,DEFAULT_FONT,buffer);

		pi_graphics_update(buffer,pal);
#ifdef VMWOS
#else
		usleep(30000);
#endif
		count++;
		if (count==600) break;

		if (count==300) {
			/* draw black line at bottom */
			vmwHlin(0,640,479,0,buffer);
		}
	}

	return;
}



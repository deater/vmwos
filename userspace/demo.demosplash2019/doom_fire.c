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
#include <stdlib.h>
#endif

#include "svmwgraph.h"
#include "pi-graphics.h"
#include "demosplash2019.h"

#include "pi_outline.h"

#define NUMCREDITS 6

struct scrolltext_type {
	int out;
	int trigger;
	int fade;
	int x,y;
	int color;
	char text[30];
};

static struct scrolltext_type scrolltext[NUMCREDITS]={
	{ 0, 0,   0, (640-(25*8*2))/2, 445, 15, "A VMW SOFTWARE PRODUCTION"},
	{ 0, 100, 0, (640-(12*8*2))/2, 445, 20, "CODE: DEATER"},
	{ 0, 200, 0, (640-(10*8*2))/2, 445, 25, "MUSIC: DYA"},
	{ 0, 300, 0, (640-(22*8*2))/2, 445, 30, "THANKS: TEAM28 / PIFOX"},
	{ 0, 400, 0, (640-(23*8*2))/2, 445, 35, "THANKS: FABIEN SANGLARD"},
	{ 0, 500, 0, (640-(19*8*2))/2, 445, 15, "THANKS: MPVANIERSEL"},
};


static int rgbs[38]={
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

static unsigned char pi_outline_sprite[2+128*160];

static void vmwClearTopScreen(int color, unsigned char *buffer) {

        memset(buffer,color,XSIZE*200*sizeof(unsigned char));

}
void doom_fire(unsigned char *buffer, struct palette *pal) {

	int x,y,newcol,r,dst;
	int count=0;
	int firetop=200;

	/* Load the Pi logo to the buffer */
	vmwLoadPCX(pi_outline_pcx,0,0, buffer, XSIZE);

	/* load into sprite */
	pi_outline_sprite[0]=128;
	pi_outline_sprite[1]=160;
	for(y=0;y<160;y++) {
		for(x=0;x<128;x++) {
			pi_outline_sprite[2+(y*128)+x]=buffer[y*XSIZE+x];
		}
	}

	vmwClearScreen(0,buffer);

	for(x=0;x<38;x++) {
		pal->red[x]=(rgbs[x]>>16)&0xff;
		pal->green[x]=(rgbs[x]>>8)&0xff;
		pal->blue[x]=(rgbs[x])&0xff;
	}

	/* draw white line at bottom */
	vmwHlin(0,640,479,37,buffer);



	while(1) {
		if (firetop==200) vmwClearTopScreen(0, buffer);

		for(x=0;x<640;x++) {
			for(y=firetop;y<479;y++) {
				r=rand()&7;
				dst=(y*XSIZE)+x-(r&3)+1;
//				if (dst>=640*480) dst=(640*480)-1;
				newcol=buffer[((y+1)*XSIZE)+x]-(r<2);
				if (newcol<0) newcol=0;
				buffer[dst]=newcol;
			}
		}

		for(x=0;x<NUMCREDITS;x++) {
			if (scrolltext[x].trigger==count) {
				scrolltext[x].out=1;
			}
		}

		for(x=0;x<NUMCREDITS;x++) {
			if (scrolltext[x].out) {
				vmwTextXYx2(scrolltext[x].text,
						scrolltext[x].x,
						scrolltext[x].y,
						scrolltext[x].color,
						scrolltext[x].color,
						0,DEFAULT_FONT,buffer);
			}
		}

		for(x=0;x<NUMCREDITS;x++) {
			if (scrolltext[x].out) {
				if (!scrolltext[x].fade) {
					scrolltext[x].y--;
					if (scrolltext[x].y==120) {
						scrolltext[x].fade=20;
					}
				}
			}

			if (scrolltext[x].fade) {
				scrolltext[x].fade--;
				if (scrolltext[x].fade<16) {
					if (scrolltext[x].color>0) {
						scrolltext[x].color--;
					}
				}
				if (scrolltext[x].fade==0) {
					scrolltext[x].out=0;
				}
			}
		}

		if (count>900) {
			put_sprite_cropped(buffer,pi_outline_sprite,256,300);
			firetop=100;
		}


		pi_graphics_update(buffer,pal);
#ifdef VMWOS
#else
		usleep(30000);
#endif
		count++;
		if (count==1600) break;

		if (count==1300) {
			/* draw black line at bottom */
			vmwHlin(0,640,479,0,buffer);
		}
	}
	vmwFadeToBlack(buffer,pal);

	return;
}



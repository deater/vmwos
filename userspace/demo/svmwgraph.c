#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "svmwgraph.h"
#include "pi-graphics.h"



void vmwHlin(int x1, int x2, int y, int color, unsigned char *buffer) {

	int x,output_pointer;

	output_pointer=(y*XSIZE);

	for(x=x1;x<x2;x++) {
		buffer[output_pointer+x]=color;
	}
}

void vmwVlin(int y1, int y2, int x, int color, unsigned char *buffer) {

	int y,output_pointer;

	output_pointer=(y1*XSIZE)+x;

	for(y=y1;y<y2;y++) {
		buffer[output_pointer]=color;
		output_pointer+=XSIZE;
	}
}


void vmwPlot(int x,int y, int color, unsigned char *buffer) {

	int output_pointer;

	output_pointer=(y*XSIZE)+x;

	buffer[output_pointer]=color;
}

#define FADESPEED 8

void vmwFadeToBlack(unsigned char *buffer, struct palette *pal) {

	int i,j;

	for(j=0;j<256;j+=FADESPEED) {
		for (i=0;i<256;i++) {
			if (pal->red[i]>FADESPEED) {
				pal->red[i]-=FADESPEED;
			}
			else {
				pal->red[i]=0;
			}

			if (pal->green[i]>FADESPEED) {
				pal->green[i]-=FADESPEED;
			}
			else {
				pal->green[i]=0;
			}

			if (pal->blue[i]>FADESPEED) {
				pal->blue[i]-=FADESPEED;
			}
			else {
				pal->blue[i]=0;
			}
		}

		pi_graphics_update(buffer,pal);

#ifdef VMWOS
#else
		usleep(30000);
#endif

	}
}

void vmwClearScreen(int color, unsigned char *buffer) {

	memset(buffer,color,XSIZE*YSIZE*sizeof(unsigned char));

}

void vmwClearScreenY(int starty, int color, unsigned char *buffer) {

	memset(buffer+(starty*XSIZE),
		color,XSIZE*(YSIZE-starty)*sizeof(unsigned char));

}

void vmwSetAllBlackPalette(struct palette *pal) {

	/* FIXME: use memset? */
	int i;

	for (i=0;i<256;i++) {
		pal->red[i]=0;
		pal->green[i]=0;
		pal->blue[i]=0;
	}

}


void vmwFadeFromBlack(unsigned char *buffer, struct palette *pal) {

	int i,j;

	/* hopefully not too big */
	struct palette temp_pal;

	vmwSetAllBlackPalette(&temp_pal);

	for(j=0;j<256;j+=FADESPEED) {
		for (i=0;i<256;i++) {
			if (temp_pal.red[i] < pal->red[i]-FADESPEED) {
				temp_pal.red[i]+=FADESPEED;
			}
			else {
				temp_pal.red[i]=pal->red[i];
			}
			if (temp_pal.green[i] < pal->green[i]-FADESPEED) {
				temp_pal.green[i]+=FADESPEED;
			}
			else {
				temp_pal.green[i]=pal->green[i];
			}
			if (temp_pal.blue[i] < pal->blue[i]-FADESPEED) {
				temp_pal.blue[i]+=FADESPEED;
			}
			else {
				temp_pal.blue[i]=pal->blue[i];
			}
		}

		pi_graphics_update(buffer,&temp_pal);
#ifdef VMWOS
#else
		usleep(30000);
#endif

	}
}

/* sprite format: x,y followed by sprite data */
/* color 0xff is transparent */

void put_sprite_cropped(unsigned char *buffer,
			unsigned char *sprite,int x,int y) {

	int xsize,ysize;
	int xx,yy;
	unsigned char *sprite_pointer,*output_pointer;

	xsize=sprite[0];
	ysize=sprite[1];
	sprite_pointer=&sprite[2];
	output_pointer=&buffer[(y*XSIZE)+x];

	for(yy=0;yy<ysize;yy++) {

		for(xx=0;xx<xsize;xx++) {
			if (*sprite_pointer!=0xff) {
				if ((xx+x<XSIZE) && (yy+y<YSIZE)) {
					*output_pointer=*sprite_pointer;
				}
			}
			output_pointer++;
			sprite_pointer++;
		}
		output_pointer+=(XSIZE-xsize);
	}



	return;
}


/* sprite format: x,y followed by sprite data */

/* FIXME: can probably optimize this */

void erase_sprite_cropped(unsigned char *buffer,
			unsigned char *sprite,int x,int y) {

	int xsize,ysize;
	int xx,yy;
	unsigned char *output_pointer;

	xsize=sprite[0];
	ysize=sprite[1];
	output_pointer=&buffer[(y*XSIZE)+x];

	for(yy=0;yy<ysize;yy++) {
		for(xx=0;xx<xsize;xx++) {
			*output_pointer=0;
			output_pointer++;
		}
		output_pointer+=(XSIZE-xsize);
	}

	return;
}

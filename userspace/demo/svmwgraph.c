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


void vmwFadeToBlack(unsigned char *buffer, struct palette *pal) {

	int i,j;

	for(j=0;j<256;j++) {
		for (i=0;i<256;i++) {
			if (pal->red[i]) pal->red[i]--;
			if (pal->green[i]) pal->green[i]--;
			if (pal->blue[i]) pal->blue[i]--;
		}
		usleep(1000);
		pi_graphics_update(buffer,pal);

	}
}

void vmwClearScreen(int color, unsigned char *buffer) {

	memset(buffer,color,XSIZE*YSIZE*sizeof(unsigned char));

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

	for(j=0;j<256;j++) {
		for (i=0;i<256;i++) {
			if (pal->red[i]) pal->red[i]--;
			if (pal->green[i]) pal->green[i]--;
			if (pal->blue[i]) pal->blue[i]--;
		}
		usleep(1000);
		pi_graphics_update(buffer,pal);
	}
}



#include <stdio.h>	/* For FILE I/O */
#include <string.h>	/* For strncmp */
#include <fcntl.h>	/* for open()  */
#include <unistd.h>	/* for lseek() */
#include <sys/stat.h>	/* for file modes */
#include <stdlib.h>	/* free() */

#include "svmwgraph.h"

#include "default_font.c"

#define FONTSIZE_Y	16
#define	FONTSIZE_X	8

void *select_font(int which) {

	return default_font;
}

int put_char(unsigned char c, int x, int y, int fg_color, int bg_color,
	int overwrite, int which_font, unsigned char *buffer) {
	int xx,yy;

	int output_pointer;

	unsigned char (*font)[256][16];

	font=select_font(which_font);

	output_pointer=(y*XSIZE)+x;

	for(yy=0;yy<FONTSIZE_Y;yy++) {
		for(xx=0;xx<FONTSIZE_X;xx++) {
			if ((*font)[c][yy]&(1<<(FONTSIZE_X-xx))) {
				buffer[output_pointer]=fg_color;
			} else if (overwrite) {
				buffer[output_pointer]=bg_color;
			}
			output_pointer++;
		}
		output_pointer+=(XSIZE-FONTSIZE_X);
	}
	return 0;

}

int put_char_cropped(unsigned char c, int x, int y, int fg_color, int bg_color,
	int overwrite, int which_font, unsigned char *buffer) {
	int xx,yy;

	int output_pointer;

	unsigned char (*font)[256][16];

	font=select_font(which_font);

	output_pointer=(y*XSIZE)+x;

	for(yy=0;yy<FONTSIZE_Y;yy++) {
		for(xx=0;xx<FONTSIZE_X;xx++) {
			if (	(xx+x>=0) && (yy+y>=0) &&
				(xx+x<XSIZE) && (yy+y<YSIZE)) {

				if ((*font)[c][yy]&(1<<(FONTSIZE_X-xx))) {
					buffer[output_pointer]=fg_color;
				} else if (overwrite) {
					buffer[output_pointer]=bg_color;
				}
			}
			output_pointer++;
		}
		output_pointer+=(XSIZE-FONTSIZE_X);
	}
	return 0;

}


int put_charx2(unsigned char c, int x, int y, int fg_color, int bg_color,
	int overwrite, int which_font, unsigned char *buffer) {
	int xx,yy;

	int output_pointer;

	unsigned char (*font)[256][16];

	if (x>=XSIZE-(FONTSIZE_X*2)) printf("error! X=%d\n",x);
	if (y>=YSIZE-(FONTSIZE_Y*2)) printf("error! Y=%d\n",y);

	font=select_font(which_font);

	output_pointer=(y*XSIZE)+x;

	for(yy=0;yy<FONTSIZE_Y;yy++) {
		for(xx=0;xx<FONTSIZE_X;xx++) {
			if ((*font)[c][yy]&(1<<(FONTSIZE_X-xx))) {
				buffer[output_pointer]=fg_color;
				buffer[output_pointer+1]=fg_color;
				buffer[output_pointer+XSIZE]=fg_color;
				buffer[output_pointer+XSIZE+1]=fg_color;
			} else if (overwrite) {
				buffer[output_pointer]=bg_color;
				buffer[output_pointer+1]=bg_color;
				buffer[output_pointer+XSIZE]=bg_color;
				buffer[output_pointer+XSIZE+1]=bg_color;
			}
			output_pointer+=2;
		}
		output_pointer+=(2*XSIZE-FONTSIZE_X*2);

	}
	return 0;

}

int put_charx4(unsigned char c, int x, int y, int fg_color,
	int which_font, unsigned char *buffer) {
	int xx,yy;

	int output_pointer,out_color;

	unsigned char (*font)[256][16];

	if (x>=(XSIZE-FONTSIZE_X*4)) printf("error! X=%d\n",x);
	if (y>=(YSIZE-FONTSIZE_Y*4)) printf("error! Y=%d\n",y);

	font=select_font(which_font);

	output_pointer=(y*XSIZE)+x;
	out_color=fg_color;

	for(yy=0;yy<FONTSIZE_Y;yy++) {
		for(xx=0;xx<FONTSIZE_X;xx++) {
			if (((*font)[c][yy]&(1<<(FONTSIZE_X-xx))) || (fg_color==0)) {
				if (fg_color!=0) {
					out_color=fg_color+(yy*2);
				}
				buffer[output_pointer+0]=out_color;
				buffer[output_pointer+1]=out_color;
				buffer[output_pointer+2]=out_color;
				buffer[output_pointer+3]=out_color;

				buffer[output_pointer+0+XSIZE]=out_color;
				buffer[output_pointer+1+XSIZE]=out_color;
				buffer[output_pointer+2+XSIZE]=out_color;
				buffer[output_pointer+3+XSIZE]=out_color;

				if (fg_color!=0) {
					out_color=(fg_color+1)+(yy*2);
				}
				buffer[output_pointer+0+XSIZE*2]=out_color;
				buffer[output_pointer+1+XSIZE*2]=out_color;
				buffer[output_pointer+2+XSIZE*2]=out_color;
				buffer[output_pointer+3+XSIZE*2]=out_color;

				buffer[output_pointer+0+XSIZE*3]=out_color;
				buffer[output_pointer+1+XSIZE*3]=out_color;
				buffer[output_pointer+2+XSIZE*3]=out_color;
				buffer[output_pointer+3+XSIZE*3]=out_color;

			}
			output_pointer+=4;
		}
		output_pointer+=(4*XSIZE-FONTSIZE_X*4);

	}
	return 0;

}



int print_string(char *string, int x, int y, int color,
	int which_font, unsigned char *buffer)  {

	int i;
//	printf("Putting %d at %d,%d,%d\n",string[0],x,y,color);
	for(i=0;i<strlen(string);i++) {
		put_char(string[i],x+(i*FONTSIZE_X),y,color,0,0,
			which_font,buffer);
	}

	return 0;
}

    /* Output a string at location x,y */
void vmwTextXY(char *string,int x,int y,int color,int background,int overwrite,
	int which_font, unsigned char *buffer) {

	int i;

	for(i=0;i<strlen(string);i++) {
		put_char(string[i],x+(i*FONTSIZE_X),y,
				color,background,overwrite,which_font,buffer);
	}
}

    /* Output a string at location x,y scaled up by 2 */
void vmwTextXYx2(char *string,int x,int y,int color,int background,int overwrite,
	int which_font, unsigned char *buffer) {

	int i;

	for(i=0;i<strlen(string);i++) {
		put_charx2(string[i],x+(i*FONTSIZE_X*2),y,
				color,background,overwrite,which_font,buffer);
	}
}

    /* Output a string at location x,y scaled up by 4 */
void vmwTextXYx4(char *string,int x,int y,int color,
	int which_font, unsigned char *buffer) {

	int i;

	for(i=0;i<strlen(string);i++) {
		put_charx4(string[i],x+(i*FONTSIZE_X*4),y,
				color,which_font,buffer);
	}
}



#include <stddef.h>
#include <stdint.h>

#include "svmwgraph.h"
#include "pi-graphics.h"

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#endif

#define ANSI_BLACK	0
#define ANSI_RED	1
#define ANSI_GREEN	2
#define ANSI_BLUE	4
#define ANSI_GREY	7


static int font_ysize=16;
static unsigned char *current_font;

#define CONSOLE_X 80
#define CONSOLE_Y 30

static unsigned char text_console[CONSOLE_X*CONSOLE_Y];
static unsigned char text_color[CONSOLE_X*CONSOLE_Y];
static signed short text_x[CONSOLE_X*CONSOLE_Y];
static signed short text_y[CONSOLE_X*CONSOLE_Y];
static signed char text_xspeed[CONSOLE_X*CONSOLE_Y];
static signed char text_yspeed[CONSOLE_X*CONSOLE_Y];
static int console_x=0;
static int console_y=0;
static int console_fore_color=0xffffff;
static int console_back_color=0;
static int console_fore_bright=0;

static unsigned int ansi_colors[16]={
	0x000000, /* black */
        0xaa0000, /* red */
        0x00aa00, /* green */
        0xaa5522, /* brown */
        0x0000aa, /* blue  */
        0xaa00aa, /* purple */
        0x00aaaa, /* cyan */
        0xaaaaaa, /* grey */
        0x7d7d7d, /* dark grey */
        0xff7d7d, /* bright red */
        0x00ff00, /* bright green */
        0xffff00, /* yellow */
        0x0000ff, /* bright blue */
        0xff00ff, /* pink */
        0x00ffff, /* bright_cyan */
        0xffffff, /* white */
};

static void console_load_ansi_pal(struct palette *pal) {
	int i;

	for(i=0;i<16;i++) {
		pal->red[i]=(ansi_colors[i]>>16)&0xff;
		pal->green[i]=(ansi_colors[i]>>8)&0xff;
		pal->blue[i]=(ansi_colors[i])&0xff;
	}
}

int console_clear(void) {

	int x,y;

	for(y=0;y<CONSOLE_Y;y++) {
		for(x=0;x<CONSOLE_X;x++) {
			text_console[x+(y*CONSOLE_X)]=' ';
			text_color[x+(y*CONSOLE_X)]=FORE_GREY|BACK_BLACK;
		}
	}

	return 0;
}

int console_home(void) {

	console_x=0;
	console_y=0;

	return 0;
}


int console_update(unsigned char *buffer, struct palette *pal, int pi_top) {

	int x,y,ystart;

	ystart=0;
	if (pi_top) ystart=5;


	for(y=ystart;y<CONSOLE_Y;y++) {
		for(x=0;x<CONSOLE_X;x++) {
			put_char(text_console[x+(y*CONSOLE_X)],
				x*8,y*font_ysize,
				text_color[x+(y*CONSOLE_X)]&0xf,
				(text_color[x+(y*CONSOLE_X)]>>4)&0xf,
				1,DEFAULT_FONT,buffer);
		}
	}

	pi_graphics_update(buffer,pal);

	return 0;
}

static int console_update_weird(int starty,
		unsigned char *buffer, struct palette *pal) {

	int x,y;

	for(y=starty;y<CONSOLE_Y;y++) {
		for(x=0;x<CONSOLE_X;x++) {
			put_char_cropped(text_console[x+(y*CONSOLE_X)],
				text_x[(y*CONSOLE_X)+x],
				text_y[(y*CONSOLE_X)+x],
				text_color[x+(y*CONSOLE_X)]&0xf,
				(text_color[x+(y*CONSOLE_X)]>>4)&0xf,
				0,DEFAULT_FONT,buffer);
		}
	}

	pi_graphics_update(buffer,pal);

	return 0;
}



/* Arbitrary, can't find a spec although more than 3 shouldn't be necessary */
#define ANSI_MAX_NUMBERS 10

#define ANSI_STATE_NORMAL	0
#define ANSI_STATE_ESCAPE	1
#define ANSI_STATE_NUMBER	2
#define ANSI_STATE_COMPLETE	3

#define ANSI_DEFAULT		0xffffffff

static uint32_t numbers[ANSI_MAX_NUMBERS];
static int32_t which_number=-1;
static uint32_t ansi_state=ANSI_STATE_NORMAL;
static uint32_t ansi_command;

int console_write(const char *string, int length,
		unsigned char *buffer, struct palette *pal,
		int pi_top) {

	int i=0;
	int refresh_screen=0;
	int distance;
	int x;
	int c;

	while(1) {
		if (ansi_state==ANSI_STATE_NORMAL) {
			if (string[i]=='\r') {
				/* Carriage Return */
				/* console_x=0; */
				/* we ignore and hope a \r is always */
				/* followed by a \n */
			} else if (string[i]=='\n') {
				/* Linefeed */
				console_x=0;
				console_y++;
				console_update(buffer,pal,pi_top);
				usleep(50000);
			} else if (string[i]=='\t') {
				console_x=(console_x+8)&(~0x7);
			} else if (string[i]=='\b') {
				console_x--;
				if (console_x<0) console_x=0;
				refresh_screen=1;
			} else if (string[i]==27) {
				ansi_state=ANSI_STATE_ESCAPE;
			}

			else {
				/* Write out the characters */
				/* If overwriting previous char, need to refresh */
				if (text_console[console_x+(console_y*CONSOLE_X)]!=' ') {
					refresh_screen=1;
				}
				text_console[console_x+(console_y*CONSOLE_X)]=string[i];
				text_color[console_x+(console_y*CONSOLE_X)]=
					(console_back_color<<4 |
					(console_fore_color&0xf));
				console_x++;
			}

		}
		else if (ansi_state==ANSI_STATE_ESCAPE) {
			if (string[i]=='[') {
				which_number=-1;
				numbers[0]=ANSI_DEFAULT;
				ansi_state=ANSI_STATE_NUMBER;
			}
			else {
				ansi_state=ANSI_STATE_NORMAL;
			}
		} else if (ansi_state==ANSI_STATE_NUMBER) {
			int val;
			val=string[i];

			/* If not a number */
			if ((val<'0') || (val>'9')) {
				/* ; separated list, move to next number */
				if (val==';') {
					which_number++;
					if (which_number==ANSI_MAX_NUMBERS) {
						ansi_state=ANSI_STATE_NORMAL;
						//printk("ANSI: Too many numbers!\n");
						break;
					}
					numbers[which_number]=0;
				}
				else {
					which_number++;
					ansi_command=val;
					ansi_state=ANSI_STATE_COMPLETE;
				}
			}

			else {
				if (which_number==-1) {
					which_number=0;
					numbers[0]=0;
				}
				numbers[which_number]*=10;
				numbers[which_number]+=val-'0';
			}
		}

		if (ansi_state==ANSI_STATE_COMPLETE) {

			switch(ansi_command) {
				case 'A':
					/* cursor up */
					if (numbers[0]==ANSI_DEFAULT) distance=1;
					else distance=numbers[0];
					console_y-=distance;
					break;
				case 'B':
					/* cursor down */
					if (numbers[0]==ANSI_DEFAULT) distance=1;
					else distance=numbers[0];
					console_y+=distance;
					break;
				case 'C':
					/* cursor forward */
					if (numbers[0]==ANSI_DEFAULT) {
						distance=1;
					}
					else {
						distance=numbers[0];
					}
					console_x+=distance;
					break;
				case 'D':
					/* cursor backward */
					if (numbers[0]==ANSI_DEFAULT) distance=1;
					else distance=numbers[0];
					console_x-=distance;
					break;
				case 'H':

					/* GotoXY */
					if (numbers[0]==ANSI_DEFAULT) {
						console_x=0;
						console_y=0;
					}
					else if (which_number==0) {
						/* 1-based */
						console_y=numbers[0]-1;
						//serial_printk("Y=%d\n",console_y);
					}
					else {
						/* 1-based */
						console_y=numbers[0]-1;
						console_x=numbers[1]-1;
						//serial_printk("X,Y=%d,%d\n",console_x,console_y);

					}

					break;
				case 'J':
					switch(numbers[0]) {

					case 0:
					case ANSI_DEFAULT:
						/* clear to end of screen */
					case 1:
						/* clear to beginning of screen */
					default:
						//printk("ANSI: unknown clear %d\n",numbers[0]);
						break;
					case 2:
						/* clear all of screen */
						console_clear();
						console_home();
						break;
					}
					break;
				case 'm':
					/* colors */
					for(c=0;c<which_number;c++) {
						if ((numbers[c]==0) ||
							(numbers[c]==ANSI_DEFAULT)) {

							console_fore_color=ANSI_GREY;
							console_back_color=ANSI_BLACK;
							console_fore_bright=0;
						}
						if (numbers[c]==1) {
							console_fore_bright=1;
							console_fore_color|=(1<<3);
						}

						/* Foreground Colors */
						if ((numbers[c]>=30)&&(numbers[c]<=37)) {
							console_fore_color=(numbers[c]-30)|(console_fore_bright<<3);
						}

						/* FIXME Color 38 used for 24-bit color support */

						/* Background Colors */
						if ((numbers[c]>=40)&&(numbers[c]<=47)) {
							console_back_color=numbers[c]-40;
						}
					}
					break;
				default:
					//printk("Unknown ansi command \'%c\'",
					//		ansi_command);
						break;
			}

			/* reset state */
			which_number=-1;
			ansi_state=ANSI_STATE_NORMAL;
		}

		/* Bounds check */
		if (console_x<0) console_x=0;

		if (console_y<0) console_y=0;

		/* FIXME: should we wrap multiple lines if console_x */
		/* is way longer than a line? */
		if (console_x>=CONSOLE_X) {
			console_x=console_x%CONSOLE_X;
			console_y++;
		}

		if (console_y>=CONSOLE_Y) {


			/* scroll up a line */

			refresh_screen=1;

			memmove(&(text_console[0]),&(text_console[CONSOLE_X]),
				(CONSOLE_Y-1)*CONSOLE_X*sizeof(unsigned char));
			memmove(&(text_color[0]),&(text_color[CONSOLE_X]),
				(CONSOLE_Y-1)*CONSOLE_X*sizeof(unsigned char));

			for(x=0;x<CONSOLE_X;x++) {
				text_console[x+(CONSOLE_Y-1)*CONSOLE_X]=' ';
				text_color[x+(CONSOLE_Y-1)*CONSOLE_X]=FORE_GREY|BACK_BLACK;
			}
			console_y--;
		}

		/* Go to next character */
		i++;
		if (i==length) break;

	}

//	if (refresh_screen) {
//		framebuffer_clear_screen(0);
//	}

	console_update(buffer,pal,pi_top);

	return 0;
}



int console_init(struct palette *pal) {

	console_clear();
	console_home();

	console_load_ansi_pal(pal);

	current_font=(unsigned char *)select_font(DEFAULT_FONT);

	return 0;
}


int console_text_collapse(int starty,int how_long,
			unsigned char *buffer, struct palette *pal) {
	int x,y,i;

	/* Init from current console state */

	for(y=starty;y<CONSOLE_Y;y++) {
		for (x=0;x<CONSOLE_X;x++) {
			text_x[(y*CONSOLE_X)+x]=x*8;
			text_y[(y*CONSOLE_X)+x]=y*16;
			text_xspeed[(y*CONSOLE_X)+x]=0;
			/* Note: rand()%4 was returning negative sometimes? */
			text_yspeed[(y*CONSOLE_X)+x]=4+(rand()&0x3);
		}
	}

	for(i=0;i<how_long;i++) {
		/* Move letters */
		for(y=starty;y<CONSOLE_Y;y++) {
			for (x=0;x<CONSOLE_X;x++) {
				text_x[(y*CONSOLE_X)+x]+=
					text_xspeed[(y*CONSOLE_X)+x];
				text_y[(y*CONSOLE_X)+x]+=
					text_yspeed[(y*CONSOLE_X)+x];
			}
		}
		vmwClearScreenY(starty*16,0,buffer);

		console_update_weird(starty,buffer,pal);
#ifdef VMWOS
#else
		usleep(50000);
#endif

	}

	return 0;
}

int console_text_explode(unsigned char *buffer, struct palette *pal) {
	return 0;
}


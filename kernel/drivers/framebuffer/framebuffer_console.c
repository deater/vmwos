#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"

#include "drivers/framebuffer/framebuffer.h"
#include "drivers/framebuffer/framebuffer_console.h"

#include "c_font.h"

#include "lib/string.h"
#include "lib/memcpy.h"
#include "lib/memset.h"
#include "lib/errors.h"

static uint32_t framebuffer_console_initialized=0;

//static int debug=1;

#define ANSI_BLACK	0
#define ANSI_RED	1
#define ANSI_GREEN	2
#define ANSI_BLUE	4
#define ANSI_GREY	7


static int font_ysize=16;
static unsigned char *current_font=(unsigned char *)default_font;

#define CONSOLE_X 80
#define CONSOLE_Y 35

static unsigned char text_console[CONSOLE_X*CONSOLE_Y];
static unsigned char text_color[CONSOLE_X*CONSOLE_Y];
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

void framebuffer_console_setfont(int which) {

	current_font=(unsigned char *)default_font;
	font_ysize=16;
	printk("Using default font\n");

	framebuffer_clear_screen(0);
	framebuffer_console_push();

}


int framebuffer_console_putchar(int fore_color, int back_color,
			int ch, int x, int y) {

	int xx,yy;

	if (!framebuffer_console_initialized) return -ENODEV;

	for(yy=0;yy<font_ysize;yy++) {
		for(xx=0;xx<8;xx++) {
			if (current_font[(ch*font_ysize)+yy] & (1<<(7-xx))) {
				framebuffer_putpixel(fore_color,x+xx,y+yy);
#if 0
				framebuffer_hline(fore_color,
						(x+xx)*2,(x+xx)*2+1,(y+yy)*2);
				framebuffer_hline(fore_color,
						(x+xx)*2,(x+xx)*2+1,((y+yy)*2)+1);
#endif
			}
			else {
				/* transparency */
				if (back_color!=0) {
				framebuffer_putpixel(back_color,x+xx,y+yy);
#if 0
					framebuffer_hline(back_color,
						(x+xx)*2,(x+xx)*2+1,(y+yy)*2);
					framebuffer_hline(back_color,
						(x+xx)*2,(x+xx)*2+1,((y+yy)*2)+1);
#endif
				}
			}
		}
	}
	return 0;

}

int framebuffer_console_clear(void) {

	int x,y;

	framebuffer_clear_screen(0);

	for(y=0;y<CONSOLE_Y;y++) {
		for(x=0;x<CONSOLE_X;x++) {
			text_console[x+(y*CONSOLE_X)]=' ';
			text_color[x+(y*CONSOLE_X)]=FORE_GREY|BACK_BLACK;
		}
	}

	return 0;
}

int framebuffer_console_home(void) {

	console_x=0;
	console_y=0;

	return 0;
}


int framebuffer_console_push(void) {

	int x,y;

	if (!framebuffer_ready()) return -1;

	for(x=0;x<CONSOLE_X;x++) {
		for(y=0;y<CONSOLE_Y;y++) {
			framebuffer_console_putchar(
				ansi_colors[text_color[x+(y*CONSOLE_X)]&0xf],
				ansi_colors[(text_color[x+(y*CONSOLE_X)]>>4)&0xf],
				text_console[x+(y*CONSOLE_X)],
				x*8,y*font_ysize);
		}
	}

	framebuffer_push();

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

int framebuffer_console_write(const char *buffer, int length) {

	int i=0;
	int refresh_screen=0;
	int distance;
	int x;
	int c;

	if (!framebuffer_console_initialized) return -ENODEV;

	while(1) {
		if (ansi_state==ANSI_STATE_NORMAL) {
			if (buffer[i]=='\r') {
				/* Carriage Return */
				/* console_x=0; */
				/* we ignore and hope a \r is always */
				/* followed by a \n */
			} else if (buffer[i]=='\n') {
				/* Linefeed */
				console_x=0;
				console_y++;
			} else if (buffer[i]=='\t') {
				console_x=(console_x+8)&(~0x7);
			} else if (buffer[i]=='\b') {
				console_x--;
				if (console_x<0) console_x=0;
				refresh_screen=1;
			} else if (buffer[i]==27) {
				ansi_state=ANSI_STATE_ESCAPE;
			}

			else {
				/* Write out the characters */
				/* If overwriting previous char, need to refresh */
				if (text_console[console_x+(console_y*CONSOLE_X)]!=' ') {
					refresh_screen=1;
				}
				text_console[console_x+(console_y*CONSOLE_X)]=buffer[i];
				text_color[console_x+(console_y*CONSOLE_X)]=
					(console_back_color<<4 |
					(console_fore_color&0xf));
				console_x++;
			}

		}
		else if (ansi_state==ANSI_STATE_ESCAPE) {
			if (buffer[i]=='[') {
				which_number=-1;
				numbers[0]=ANSI_DEFAULT;
				ansi_state=ANSI_STATE_NUMBER;
			}
			else {
				ansi_state=ANSI_STATE_NORMAL;
			}
		} else if (ansi_state==ANSI_STATE_NUMBER) {
			int val;
			val=buffer[i];

			/* If not a number */
			if ((val<'0') || (val>'9')) {
				/* ; separated list, move to next number */
				if (val==';') {
					which_number++;
					if (which_number==ANSI_MAX_NUMBERS) {
						ansi_state=ANSI_STATE_NORMAL;
						printk("ANSI: Too many numbers!\n");
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
						printk("ANSI: unknown clear %d\n",numbers[0]);
						break;
					case 2:
						/* clear all of screen */
						framebuffer_console_clear();
						framebuffer_console_home();
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
					printk("Unknown ansi command \'%c\'",
							ansi_command);
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

			/* we overlap */
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

	if (refresh_screen) {
		framebuffer_clear_screen(0);
	}

	framebuffer_console_push();

	return 0;
}

int framebuffer_console_val(int x, int y) {

	if (x>=CONSOLE_X) return -1;
	if (y>=CONSOLE_Y) return -1;

	return text_console[x+(y*CONSOLE_X)];

}

int framebuffer_console_init(void) {

	framebuffer_console_clear();
	framebuffer_console_home();

	framebuffer_console_initialized=1;

	return 0;
}


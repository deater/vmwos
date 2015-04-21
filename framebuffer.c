/* based on info at https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/screen01.html */
/* http://elinux.org/RPi_Framebuffer */

#include <stddef.h>
#include <stdint.h>

#include "printk.h"
#include "mmio.h"

#include "mailbox.h"
#include "framebuffer.h"

#include "serial.h"

#include "tbfont.h"
#include "medieval_font.h"
#include "marie_font.h"

#define ANSI_BLACK	0
#define ANSI_RED	1
#define ANSI_GREEN	2
#define ANSI_BLUE	4
#define ANSI_GREY	7


static int font_ysize=8;
static unsigned char *current_font=(unsigned char *)tb1_font;

unsigned int ansi_colors[16]={
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

struct frame_buffer_info_type {
	int phys_x,phys_y;	/* IN: Physical Width / Height*/
	int virt_x,virt_y;	/* IN: Virtual Width / Height */
	int pitch;		/* OUT: bytes per row */
	int depth;		/* IN: bits per pixel */
	int x,y;		/* IN: offset to skip when copying fb */
	int pointer;		/* OUT: pointer to the framebuffer */
	int size;		/* OUT: size of the framebuffer */
};

static struct frame_buffer_info_type current_fb;


void framebuffer_setfont(int which) {

	if (which==1) {
		current_font=(unsigned char *)medieval_font;
		font_ysize=16;
		printk("Using Medieval font\r\n");
	}
	else if (which==2) {
		current_font=(unsigned char *)marie_font;
		font_ysize=16;
		printk("Using Rune font\r\n");
	}
	else {
		current_font=(unsigned char *)tb1_font;
		font_ysize=8;
		printk("Using default font\r\n");
	}
	framebuffer_clear_screen(0);
	framebuffer_push_console();

}

static void dump_framebuffer_info(struct frame_buffer_info_type *fb) {

	printk("px %d py %d vx %d vy %d pitch %d depth %d x %d y %d ptr %x sz %d\r\n",
		fb->phys_x,fb->phys_y,
		fb->virt_x,fb->virt_y,
		fb->pitch,fb->depth,
		fb->x,fb->y,
		fb->pointer,fb->size);

	return;
}

char *framebuffer_init(int x, int y, int depth) {

	struct frame_buffer_info_type fb_info  __attribute__ ((aligned(16)));;

	int result;

	fb_info.phys_x=x;
	fb_info.phys_y=y;
	fb_info.virt_x=x;
	fb_info.virt_y=y;
	fb_info.pitch=0;
	fb_info.depth=depth;
	fb_info.x=0;
	fb_info.y=0;
	fb_info.pointer=0;
	fb_info.size=0;

	printk("Attempting to write %x to %x\r\n",
		&fb_info,MAILBOX_BASE);
	dump_framebuffer_info(&fb_info);

	result=mailbox_write( (unsigned int)(&fb_info)+0x40000000 ,
		MAILBOX_FRAMEBUFFER);

	if (result<0) {
		printk("Mailbox write failed\r\n");
		return NULL;
	}

	result=mailbox_read(MAILBOX_FRAMEBUFFER);

	dump_framebuffer_info(&fb_info);

	if (result==-1) {
		printk("Mailbox read failed\r\n");
		return NULL;
	}

	current_fb.pointer=(int)(fb_info.pointer);
	current_fb.phys_x=fb_info.phys_x;
	current_fb.phys_y=fb_info.phys_y;
	current_fb.pitch=fb_info.pitch;
	current_fb.depth=fb_info.depth;

	return (char *)(fb_info.pointer);
}

int framebuffer_hline(int color, int x0, int x1, int y) {

	int x;
	int r,g,b;

	unsigned char *fb=(unsigned char *)current_fb.pointer;

	r=(color&0xff0000)>>16;
	g=(color&0x00ff00)>>8;
	b=color&0x0000ff;

	for(x=x0;x<x1;x++) {
		fb[(y*current_fb.pitch)+(x*3)+0]=r;
		fb[(y*current_fb.pitch)+(x*3)+1]=g;
		fb[(y*current_fb.pitch)+(x*3)+2]=b;
	}

	return 0;
}

int framebuffer_clear_screen(int color) {

	int y;

	for(y=0;y<current_fb.phys_y;y++) {
		framebuffer_hline(color,0,current_fb.phys_x-1,y);
	}

	return 0;
}

int framebuffer_putpixel(int color, int x, int y) {

	int r,g,b;

	unsigned char *fb=(unsigned char *)current_fb.pointer;

	r=(color&0xff0000)>>16;
	g=(color&0x00ff00)>>8;
	b=color&0x0000ff;

	fb[(y*current_fb.pitch)+(x*3)+0]=r;
	fb[(y*current_fb.pitch)+(x*3)+1]=g;
	fb[(y*current_fb.pitch)+(x*3)+2]=b;

	return 0;

}

int framebuffer_putchar(int fore_color, int back_color,
			int ch, int x, int y) {

	int xx,yy;

	for(yy=0;yy<font_ysize;yy++) {
		for(xx=0;xx<8;xx++) {
			if (current_font[(ch*font_ysize)+yy] & (1<<(7-xx))) {
				framebuffer_putpixel(fore_color,x+xx,y+yy);
			}
			else {
				/* transparency */
				if (back_color!=0) {
					framebuffer_putpixel(back_color,
						x+xx,y+yy);
				}
			}
		}
	}
	return 0;

}

#define CONSOLE_X 80
#define CONSOLE_Y 50

static unsigned char text_console[CONSOLE_X][CONSOLE_Y];
static unsigned char text_color[CONSOLE_X][CONSOLE_Y];
static int console_x=0;
static int console_y=0;
static int console_fore_color=0xffffff;
static int console_back_color=0;



int framebuffer_console_clear(void) {

	int x,y;

	framebuffer_clear_screen(0);

	for(y=0;y<CONSOLE_Y;y++) {
		for(x=0;x<CONSOLE_X;x++) {
			text_console[x][y]=' ';
			text_color[x][y]=FORE_GREY|BACK_BLACK;
		}
	}

	return 0;
}

int framebuffer_console_home(void) {

	console_x=0;
	console_y=0;

	return 0;
}

int framebuffer_console_init(void) {

	framebuffer_console_clear();
	framebuffer_console_home();

	return 0;
}



int framebuffer_push_console(void) {

	int x,y;

	if (current_fb.pointer==0) return -1;

	for(x=0;x<CONSOLE_X;x++) {
		for(y=0;y<CONSOLE_Y;y++) {
			framebuffer_putchar(
				ansi_colors[text_color[x][y]&0xf],
				ansi_colors[(text_color[x][y]>>4)&0xf],
				text_console[x][y],x*8,y*font_ysize);
		}
	}

	return 0;
}

int isalpha(int ch) {

	if ((ch>='A') && (ch<='Z')) return 1;
	if ((ch>='a') && (ch<='z')) return 1;

	return 0;

}

void ansi_parse_color(char *code) {

	int p=0;
	int number=0;

//	printk("Parsing: %s\n",code);

	while(code[p]!=0) {

		if ((code[p]==';') || (code[p]=='m')) {

//			printk("Setting color %d\n",number);

			if (number==0) {
				console_fore_color=ANSI_GREY;
				console_back_color=ANSI_BLACK;
			}

			/* Foreground Colors */
			if ((number>=30)&&(number<=37)) {
				console_fore_color=number-30;
			}

			/* FIXME Color 38 used for 24-bit color support */

			/* Background Colors */
			if ((number>=40)&&(number<=47)) {
				console_back_color=number-40;
			}
			number=0;
		}
		else {
			number*=10;
			number+=(code[p]-'0');
		}

		p++;
	}
}



int framebuffer_write(const char *buffer, int length) {

	char escape_code[20];

	int i,e;

	for(i=0;i<length;i++) {

		if (buffer[i]=='\r') {
			console_y++;
		} else if (buffer[i]=='\n') {
			console_x=0;
		} else if (buffer[i]==27) {
			/* escape */
			i++;
			if (buffer[i]=='[') {
				i++;

				/* Read in the command */
				e=0;

				while (!isalpha(buffer[i])) {
					escape_code[e]=buffer[i];
					e++;
					i++;
				}

				escape_code[e]=buffer[i];
				escape_code[e+1]=0;

//				printk("ESCAPE! %s\r\n",escape_code);

				/* clear screen */
				if (escape_code[e]=='J') {
					framebuffer_console_clear();
				}
				/* colors */
				if (escape_code[e]=='m') {
//					printk("Parsing %s\r\n",escape_code);
					ansi_parse_color(escape_code);
				}

			}
		} else {
			text_console[console_x][console_y]=buffer[i];
			text_color[console_x][console_y]=
				(console_back_color<<4 |
				(console_fore_color&0xf));
			console_x++;
		}

		if (console_x>=CONSOLE_X) {
			console_x=0;
			console_y++;
		}

		if (console_y>=CONSOLE_Y) {
			console_y=0;
		}
	}

	framebuffer_push_console();

	return 0;
}

int framebuffer_console_val(int x, int y) {

	if (x>=CONSOLE_X) return -1;
	if (y>=CONSOLE_Y) return -1;

	return text_console[x][y];

}




int framebuffer_tb1(void) {

	int ch;
	int x=400,y=550;
	int xspeed=0;
	int missile_x=0,missile_y=0,missile_out=0;
	int explosion_x=0,explosion_y=0,explosion_out=0;

	while(1) {

		ch=uart_getc_noblock();

		framebuffer_clear_screen(0);
		framebuffer_push_console();

		switch(ch) {
			case ' ':
				if (!missile_out) {
					missile_x=x;
					missile_y=y;
					missile_out=1;
				}
				break;
			case 'j':
				xspeed--;
				break;
			case 'k':
				xspeed++;
				break;
			case 'q':
				return 0;
			default:
				break;
		}

		x+=xspeed;

		if (x<0) {
			x=0;
			xspeed=0;
		}

		if (x>790) {
			x=789;
			xspeed=0;
		}

		framebuffer_putchar(0xff,0x0,'^',x,y);

		if (missile_out) {

			/* check for collision */
			if (framebuffer_console_val(0,0)!=' ') {
				missile_out=0;
				explosion_out=1;
				explosion_x=missile_x;
				explosion_y=missile_y;
			}
		}


		if (missile_out) {
			missile_y--;

			if (missile_y<0) {
				missile_out=0;
			}
			else {
				framebuffer_putchar(0x00ff,0x0,'!',missile_x,missile_y);
			}
		}

		if (explosion_out) {
			explosion_out--;
			framebuffer_putchar(0xffff00,0x0,'*',explosion_x,explosion_y);
		}

	}

	return 0;
}

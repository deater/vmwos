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

#include "pi_logo.h"
#include "pi1_dmesg.h"
#include "linuxlogo.h"

extern unsigned char _binary_pi_logo_pcx_start[];
extern unsigned char _binary_pi_logo_pcx_end[];

static int future_pal[32]={
	0xffffff, 0xffffff, 0xffffff, 0xffffff,
	0x7f7fb8, 0x0d1134,
	0x26175b, 0x242882, 0x323ea2, 0x485bc0,
	0x6b82d7, 0x8f9cd8, 0xd9e1f1, 0xd9e1f1,
	0xb9bec7, 0x0d0e14, 0x1c0f35, 0x430b58,
	0x61358d, 0x61358d, 0x9a49aa, 0xb988c3,
	0xd0a6d7, 0xa89ec7, 0xffffff, 0xffffff,
	0xffffff, 0xffffff, 0xffffff, 0xffffff,
	0xffffff, 0xffffff,
};

#if 0
static short sine_table[64]={
	  0, 15, 31, 46, 61, 75, 88,101,
	113,123,133,141,147,153,156,159,
	160,159,156,153,147,141,133,123,
	113,101, 88, 75, 61, 46, 31, 15,
	0,-15,-31,-46,-61,-75,-88,-101,
	-113,-123,-133,-141,-147,-153,-156,-159,
	-160,-159,-156,-153,-147,-141,-133,-123,
	-113,-101,-88,-75,-61,-46,-31,-15
};
#endif

static short sine_table[128]={
	  0,  7, 15, 23, 31, 38, 46, 53, 61, 68, 75, 82, 88, 95,101,107,
	113,118,123,128,133,137,141,144,147,150,153,155,156,158,159,159,
	160,159,159,158,156,155,153,150,147,144,141,137,133,128,123,118,
	113,107,101,95,88,82,75,68,61,53,46,38,31,23,15,7,
	0,-7,-15,-23,-31,-38,-46,-53,-61,-68,-75,-82,-88,-95,-101,-107,
	-113,-118,-123,-128,-133,-137,-141,-144,-147,-150,-153,-155,-156,-158,-159,-159,
	-160,-159,-159,-158,-156,-155,-153,-150,-147,-144,-141,-137,-133,-128,-123,
	-118,-113,-107,-101,-95,-88,-82,-75,-68,-61,-53,-46,-38,-31,-23,-15,-7,
};

static unsigned char pi_logo_sprite[2+(63*80)];

static void put_rasterbar(int y, int color, unsigned char *buffer) {

	vmwHlin(0, 640, y-7, color+7, buffer);
	vmwHlin(0, 640, y-6, color+6, buffer);
	vmwHlin(0, 640, y-5, color+5, buffer);
	vmwHlin(0, 640, y-4, color+4, buffer);
	vmwHlin(0, 640, y-3, color+3, buffer);
	vmwHlin(0, 640, y-2, color+2, buffer);
	vmwHlin(0, 640, y-1, color+1, buffer);
	vmwHlin(0, 640, y, color, buffer);
//	vmwHlin(0, 640, y,   15,    buffer);
	vmwHlin(0, 640, y+1, color, buffer);
	vmwHlin(0, 640, y+2, color+1, buffer);
	vmwHlin(0, 640, y+3, color+2, buffer);
	vmwHlin(0, 640, y+4, color+3, buffer);
	vmwHlin(0, 640, y+5, color+4, buffer);
	vmwHlin(0, 640, y+6, color+5, buffer);
	vmwHlin(0, 640, y+7, color+6, buffer);
	vmwHlin(0, 640, y+8, color+7, buffer);
}

static void erase_rasterbar(int y, unsigned char *buffer) {

	int i;

	for(i=0;i<16;i++) {
		vmwHlin(0, 640, (y-7)+i, 0, buffer);
	}
}



int boot_intro(unsigned char *buffer, struct palette *pal) {

	int i,j,x,y,color;
	unsigned char *pi_logo=__pi_logo_pcx;
	int filesize=__pi_logo_pcx_len;
	int length;
	int red_y,orange_y,yellow_y,green_y,blue_y,purple_y;

	/* Load the Pi logo to the buffer */
	vmwPCXLoadPalette(pi_logo, filesize-769, pal);
	vmwLoadPCX(pi_logo,0,0, buffer);

	/* load into sprite */
	pi_logo_sprite[0]=63;
	pi_logo_sprite[1]=80;
	for(y=0;y<80;y++) {
		for(x=0;x<63;x++) {
			color=buffer[y*XSIZE+x];
			//if (color==243) color=0xff;
			pi_logo_sprite[2+(y*63)+x]=color;
		}
	}

	vmwClearScreen(0, buffer);

	put_sprite_cropped(buffer,pi_logo_sprite,0,0);

	/* Write the demsg to the screen */
	length=strlen(pi1_dmesg);
	console_write(pi1_dmesg, length,buffer,pal,1);

	sleep(1);

	/* Set palette for red letters */
	for(i=0;i<32;i++) {
		pal->red[i+16]=0x80+(4*i);
		pal->green[i+16]=0x00;
		pal->blue[i+16]=0x00;
	}

	/* Print systemd message */
	vmwTextXYx4("SYSTEMD ",192,300,0,DEFAULT_FONT,buffer);
	vmwTextXYx4("SYSTEMD",208,300,16,DEFAULT_FONT,buffer);
	vmwTextXYx4("MUST BE STOPPED ",64,350,0,DEFAULT_FONT,buffer);
	vmwTextXYx4("MUST BE STOPPED",80,350,16,DEFAULT_FONT,buffer);
	pi_graphics_update(buffer,pal);

	sleep(3);

	/*******************************************************/
	/* text collapse				       */
	/*******************************************************/

	/* re-draw text */
	console_update(buffer,pal,1);

	sleep(1);

	console_text_collapse(5,120,buffer,pal);

	sleep(1);

	/*******************************************************/
	/* moving pi					       */
	/*******************************************************/

	vmwClearScreen(0, buffer);

	/* Set palette for rasterbars */
	for(i=0;i<8;i++) {
		/* red */
		pal->red[i+16]=0xff-(0x10*i);
		pal->green[i+16]=0x00;
		pal->blue[i+16]=0x00;
		/* orange */
		pal->red[i+24]=0xff-(0x10*i);
		pal->green[i+24]=0xaa-(0x10*i);
		pal->blue[i+24]=0x80-(0x10*i);
		/* yellow */
		pal->red[i+32]=0xff-(0x10*i);
		pal->green[i+32]=0xff-(0x10*i);
		pal->blue[i+32]=0x00;
		/* green */
		pal->red[i+40]=0x00;
		pal->green[i+40]=0xff-(0x10*i);
		pal->blue[i+40]=0x00;
		/* blue */
		pal->red[i+48]=0x00;
		pal->green[i+48]=0x00;
		pal->blue[i+48]=0xff-(0x10*i);
		/* purple */
		pal->red[i+56]=0xff-(0x10*i);
		pal->green[i+56]=0x00;
		pal->blue[i+56]=0xff-(0x10*i);
	}

	/* actual move the pi and rasterbars */
	red_y=0;
	orange_y=8;
	yellow_y=16;
	green_y=24;
	blue_y=32;
	purple_y=40;

	x=0; y=0;
	for(i=0;i<320;i++) {

		/* only move if <100 */
		if (i<100) { x+=3; y+=2; }

		/* slow down bars by factor of 1 */
		j=i;

		/* draw new */
		put_rasterbar(240+sine_table[(j+red_y)%128],16,buffer);
		put_rasterbar(240+sine_table[(j+orange_y)%128],24,buffer);
		put_rasterbar(240+sine_table[(j+yellow_y)%128],32,buffer);
		put_rasterbar(240+sine_table[(j+green_y)%128],40,buffer);
		put_rasterbar(240+sine_table[(j+blue_y)%128],48,buffer);
		put_rasterbar(240+sine_table[(j+purple_y)%128],56,buffer);

		put_sprite_cropped(buffer,pi_logo_sprite,x,y);
		pi_graphics_update(buffer,pal);
#ifdef VMWOS
#else
		usleep(30000);
#endif
		/* erase old */
		erase_sprite_cropped(buffer,pi_logo_sprite,x,y);

		erase_rasterbar(240+sine_table[(j+red_y)%128],buffer);
		erase_rasterbar(240+sine_table[(j+orange_y)%128],buffer);
		erase_rasterbar(240+sine_table[(j+yellow_y)%128],buffer);
		erase_rasterbar(240+sine_table[(j+green_y)%128],buffer);
		erase_rasterbar(240+sine_table[(j+blue_y)%128],buffer);
		erase_rasterbar(240+sine_table[(j+purple_y)%128],buffer);
	}
	//sleep(1);

	/*******************************************************/
	/* Load Linux Logo				       */
	/*******************************************************/

	console_clear();

	length=strlen(linuxlogo_ansi);
	console_write(linuxlogo_ansi, length,buffer,pal,0);
	sleep(1);

	for(i=0;i<32;i++) {
		pal->red[i+16]=(future_pal[i]>>16)&0xff;
		pal->green[i+16]=(future_pal[i]>>8)&0xff;
		pal->blue[i+16]=(future_pal[i])&0xff;
	}

	vmwTextXYx4("LINUX? ",208,150,0,DEFAULT_FONT,buffer);
	vmwTextXYx4("LINUX?",224,150,16,DEFAULT_FONT,buffer);
	pi_graphics_update(buffer,pal);
	sleep(1);

	vmwTextXYx4("WHERE WE'RE GOING ",32,200,0,DEFAULT_FONT,buffer);
	vmwTextXYx4("WHERE WE'RE GOING",48,200,16,DEFAULT_FONT,buffer);
	pi_graphics_update(buffer,pal);
	sleep(1);

	vmwTextXYx4("WE WON'T NEED LINUX ",0,250,0,DEFAULT_FONT,buffer);
	vmwTextXYx4("WE WON'T NEED LINUX",16,250,16,DEFAULT_FONT,buffer);
	pi_graphics_update(buffer,pal);
	sleep(3);

	/* re-draw text */
	console_update(buffer,pal,1);

	sleep(2);

	console_text_collapse(5,100,buffer,pal);

//	sleep(1);

	return 0;
}

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


int boot_intro(unsigned char *buffer, struct palette *pal) {

	int i,x,y,color;
	unsigned char *pi_logo=__pi_logo_pcx;
	int filesize=__pi_logo_pcx_len;
	int length;

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
	printf("BLAH=%d\n",pi_logo_sprite[2+(79*63)+5]);

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

	console_text_collapse(5,buffer,pal);

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

	printf("BLAH=%d\n",pi_logo_sprite[2+(79*63)+5]);

	x=0; y=0;
	for(i=0;i<100;i++) {
		x+=3; y+=2;
		put_rasterbar(10,16,buffer);
		put_rasterbar(60,24,buffer);
		put_rasterbar(110,32,buffer);
		put_rasterbar(160,40,buffer);
		put_rasterbar(210,48,buffer);
		put_rasterbar(260,56,buffer);

		put_sprite_cropped(buffer,pi_logo_sprite,x,y);
		pi_graphics_update(buffer,pal);
		usleep(30000);
	}
	sleep(8);

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

	console_text_collapse(5,buffer,pal);

	sleep(1);

	return 0;
}

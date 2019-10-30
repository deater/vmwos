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

static unsigned char pi_logo_sprite[2+(68*80)];

int boot_intro(unsigned char *buffer, struct palette *pal) {

	int i,x,y,color;
	unsigned char *pi_logo=__pi_logo_pcx;
	int filesize=__pi_logo_pcx_len;
	int length;

	/* Load the Pi logo to the buffer */
	vmwPCXLoadPalette(pi_logo, filesize-769, pal);
	vmwLoadPCX(pi_logo,0,0, buffer);

	/* load into sprite */
	pi_logo_sprite[0]=68;
	pi_logo_sprite[1]=80;
	for(y=0;y<80;y++) {
		for(x=0;x<68;x++) {
			color=buffer[y*XSIZE+x];
			if (color==243) color=0xff;
			pi_logo_sprite[2+(y*68)+x]=color;
		}
	}

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

	x=0; y=0;
	for(i=0;i<100;i++) {
		x+=3; y+=2;
		put_sprite_cropped(buffer,pi_logo_sprite,x,y);
		pi_graphics_update(buffer,pal);
		usleep(30000);
	}
	sleep(3);

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

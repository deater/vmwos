/* based on info at https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/screen01.html */
/* http://elinux.org/RPi_Framebuffer */

/* Note if you end up with Blue and Red switched */
/* You need to update the firmware on your Pi    */

#include <stddef.h>
#include <stdint.h>

#include "printk.h"
#include "mmio.h"

#include "mailbox.h"
#include "framebuffer.h"

static int framebuffer_initialized=0;

static struct frame_buffer_info_type current_fb;
static unsigned char offscreen[800*600*3];


struct frame_buffer_info_type {
	int phys_x,phys_y;	/* IN: Physical Width / Height*/
	int virt_x,virt_y;	/* IN: Virtual Width / Height */
	int pitch;		/* OUT: bytes per row */
	int depth;		/* IN: bits per pixel */
	int x,y;		/* IN: offset to skip when copying fb */
	int pointer;		/* OUT: pointer to the framebuffer */
	int size;		/* OUT: size of the framebuffer */
};


int framebuffer_ready(void) {

	return framebuffer_initialized;

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

	framebuffer_initialized=1;

	return (char *)(fb_info.pointer);
}

int framebuffer_hline(int color, int x0, int x1, int y) {

	int x;
	int r,g,b;

	unsigned char *fb=offscreen;

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

int framebuffer_vline(int color, int y0, int y1, int x) {

	int y;
	int r,g,b;

	unsigned char *fb=offscreen;

	r=(color&0xff0000)>>16;
	g=(color&0x00ff00)>>8;
	b=color&0x0000ff;

	for(y=y0;y<y1;y++) {
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

	unsigned char *fb=offscreen;

	r=(color&0xff0000)>>16;
	g=(color&0x00ff00)>>8;
	b=color&0x0000ff;

	fb[(y*current_fb.pitch)+(x*3)+0]=r;
	fb[(y*current_fb.pitch)+(x*3)+1]=g;
	fb[(y*current_fb.pitch)+(x*3)+2]=b;

	return 0;

}

void *memcpy(void *dest, const void *src, size_t n) {

	int i;

	int *d=dest;
	const int *s=src;

	for(i=0;i<n/4;i++) {
		*d=*s;
		d++; s++;
	}

	return dest;
}

int framebuffer_push(void) {
	memcpy((unsigned char *)current_fb.pointer,
		offscreen,800*600*3);

	return 0;

}

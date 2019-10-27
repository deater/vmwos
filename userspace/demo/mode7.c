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

static unsigned char flying_map[16][16]= {
	{0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,},
	{0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,},
	{0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,},
	{0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,},

	{4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,},
	{4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,},
	{4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,},
	{4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,},

	{8,8,8,8,9,9,9,9,10,10,10,10,11,11,11,11,},
	{8,8,8,8,9,9,9,9,10,10,10,10,11,11,11,11,},
	{8,8,8,8,9,9,9,9,10,10,10,10,11,11,11,11,},
	{8,8,8,8,9,9,9,9,10,10,10,10,11,11,11,11,},

	{12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,},
	{12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,},
	{12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,},
	{12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,},

};

static double sin_lookup_table[32]={
	 0.000000, 0.098017, 0.195090, 0.290285,
	 0.382683, 0.471397, 0.555570, 0.634393,
	 0.707107, 0.773010, 0.831470, 0.881921,
	 0.923880, 0.956940, 0.980785, 0.995185,
	 1.000000, 0.995185, 0.980785, 0.956940,
	 0.923880, 0.881921, 0.831470, 0.773010,
	 0.707107, 0.634393, 0.555570, 0.471397,
	 0.382683, 0.290285, 0.195090, 0.098017,
};

	/* 0...63 */
static double sin_lookup(int which) {

	which=(which%64);
	if (which<32) return sin_lookup_table[which];
	else return -sin_lookup_table[which-32];
}

static double cos_lookup(int which) {

	which=which+16;
	return sin_lookup(which);
}


static unsigned int apple2_color[16]={
        0,              /*  0 black */
        0xe31e60,       /*  1 magenta */
        0x604ebd,       /*  2 dark blue */
        0xff44fd,       /*  3 purple */
        0x00a360,       /*  4 dark green */
        0x9c9c9c,       /*  5 grey 1 */
        0x14cffd,       /*  6 medium blue */
        0xd0c3ff,       /*  7 light blue */
        0x607203,       /*  8 brown */
        0xff6a3c,       /*  9 orange */
        0x9d9d9d,       /* 10 grey 2 */
        0xffa0d0,       /* 11 pink */
        0x14f53c,       /* 12 bright green */
        0xd0dd8d,       /* 13 yellow */
        0x72ffd0,       /* 14 aqua */
        0xffffff,       /* 15 white */
};

void apple2_load_palette(struct palette *pal) {

	int i;

	for(i=0;i<16;i++) {
		pal->red[i]=(apple2_color[i]>>16)&0xff;
		pal->green[i]=(apple2_color[i]>>8)&0xff;
		pal->blue[i]=(apple2_color[i])&0xff;
	}

}


/* Ship Sprites */
static unsigned char ship_shadow[]={
	10,12,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x0a,0x0a,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x0a,0x0a,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x0a,0x0a,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x0a,0x0a,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x0a,0x0a,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x0a,0x0a,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x0a,0x0a,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x0a,0x0a,0xff,0xff,0xff,0xff,
};

static unsigned char ship_forward[]={
	10,12,
	0xff,0xff,0xff,0xff,0x07,0x07,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x07,0x07,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x07,0x07,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x07,0x07,0xff,0xff,0xff,0xff,
	0x05,0x05,0x05,0x05,0xff,0xff,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x05,0xff,0xff,0x05,0x05,0x05,0x05,
	0xff,0xff,0x05,0x05,0x07,0x07,0x05,0x05,0xff,0xff,
	0xff,0xff,0x05,0x05,0x07,0x07,0x05,0x05,0xff,0xff,
	0xff,0xff,0xff,0xff,0x07,0x07,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x07,0x07,0xff,0xff,0xff,0xff,
	0x01,0x01,0xff,0xff,0xff,0xff,0xff,0xff,0x01,0x01,
	0x01,0x01,0xff,0xff,0xff,0xff,0xff,0xff,0x01,0x01,
};

static unsigned char ship_right[]={
	0x5,0x6,
	0x50,0xff,0x70,0x77,0xff,
	0x50,0xff,0x70,0x77,0xff,
	0x01,0x55,0x77,0x55,0x50,
	0x01,0x55,0x77,0x55,0x50,
	0xff,0x77,0x07,0xff,0x15,
	0xff,0x77,0x07,0xff,0x15,
};

static unsigned char ship_left[]={
	0x5,0x6,
	0xff,0x77,0x70,0xff,0x50,
	0xff,0x77,0x70,0xff,0x50,
	0x50,0x55,0x77,0x55,0x01,
	0x50,0x55,0x77,0x55,0x01,
	0x15,0xff,0x07,0x77,0xff,
	0x15,0xff,0x07,0x77,0xff,
};

struct fixed_type {
	char i;
	unsigned char f;
};


static void double_to_fixed(double d, struct fixed_type *f) {

	int temp;
	temp=d*256;
	f->i=(temp>>8)&0xff;
	f->f=temp&0xff;
}

static void print_fixed(struct fixed_type *f) {
	printf("0x%02X,0x%02X",f->i,f->f);
}

static double fixed_to_double(struct fixed_type *f) {

	double d;

	d=f->i;
	d+=((double)(f->f))/256.0;

//	printf("(%02x.%02x)=%lf\n",f->i,f->f,*d);

	return d;
}

static void fixed_add(struct fixed_type *x, struct fixed_type *y, struct fixed_type *z) {
	int carry;
	short sum;

	sum=(short)(x->f)+(short)(y->f);

	if (sum>=256) carry=1;
	else carry=0;

	z->f=sum&0xff;

	z->i=x->i+y->i+carry;
}

static void fixed_mul(struct fixed_type *x, struct fixed_type *y, struct fixed_type *z) {

	int a,b,c;

	a=((x->i)<<8)+(x->f);
	b=((y->i)<<8)+(y->f);

	c=a*b;
//	printf("%x %x %x\n",a,b,c);

	c>>=8;

	z->i=(c>>8);
	z->f=(c&0xff);
}

static int tile_w=16,tile_h=16;


/* http://www.helixsoft.nl/articles/circle/sincos.htm */

/* height of the camera above the plane */
static struct fixed_type space_z={2,128}; /* 2.5 */
/* number of pixels line 0 is below the horizon */
static int horizon=-68;		// -2
/* scale of space coordinates to screen coordinates */
#define SCALE_X 320
#define SCALE_Y 240
#define BMP_W	640
#define BMP_H	480

double BETA=-0.6;

void draw_background_mode7(int angle, double cx, double cy,
		unsigned char *buffer) {

	int color;

	// current screen position
	int screen_x, screen_y;

	// the distance and horizontal scale of the line we are drawing
	double distance, horizontal_scale;

	// masks to make sure we don't read pixels outside the tile
	int mask_x = (tile_w - 1);
	int mask_y = (tile_h - 1);

	// step for points in space between two pixels on a horizontal line
	double  line_dx, line_dy;

	// current space position
	double space_x, space_y;

//	clear_screens();

	for (screen_y = 72; screen_y < BMP_H; screen_y++) {
		// first calculate the distance of the line we are drawing
		distance = (fixed_to_double(&space_z) * SCALE_Y) / (screen_y + horizon);

		// then calculate the horizontal scale, or the distance between
		// space points on this horizontal line
		horizontal_scale = (distance / SCALE_X);

		// calculate the dx and dy of points in space when we step
		// through all points on this line
		line_dx = -sin_lookup(angle) * horizontal_scale;
		line_dy = cos_lookup(angle) * horizontal_scale;

		// calculate the starting position
		space_x = cx + (distance * cos_lookup(angle)) - (BMP_W/2) * line_dx;
		space_y = cy + (distance * sin_lookup(angle)) - (BMP_W/2) * line_dy;

		space_x+=(BETA*fixed_to_double(&space_z)*cos_lookup(angle));
		space_y+=(BETA*fixed_to_double(&space_z)*sin_lookup(angle));


		// go through all points in this screen line
		for (screen_x = 0; screen_x < BMP_W; screen_x++) {
			// get a pixel from the tile and put it on the screen

			color=(flying_map[(int)space_x & mask_x]
					[(int)space_y&mask_y]);

			vmwPlot(screen_x,screen_y,color,buffer);

			// advance to the next position in space
			space_x += line_dx;
			space_y += line_dy;
		}
	}
}


int flying(unsigned char *buffer, struct palette *pal) {

	int i,color;
	unsigned char ch;
	int xx,yy;
	int turning=0;
	double flyx=0,flyy=0;
	int our_angle=0;
	double dy,dx,speed=0;

	/************************************************/
	/* Flying					*/
	/************************************************/

	xx=318;	yy=312;

	/* Make sky */
	color=APPLE2_COLOR_MEDIUMBLUE;
	for(i=0;i<72;i++) {
		vmwHlin(0, 640, i, color, buffer);
	}

	while(1) {
		ch=pi_graphics_input();

		if ((ch=='q') || (ch==27))  break;

		/* up */
		if ((ch=='i') || (ch==11)) {
			if (yy>192) {
				yy-=24;
				space_z.i+=1;
			}

//			printf("Z=%lf\n",space_z);
		}
		/* down */
		if ((ch=='m') || (ch==10)) {
			if (yy<360) {
				yy+=24;
				space_z.i-=1;
			}
//			printf("Z=%lf\n",space_z);
		}
		/* left */
		if ((ch=='j') || (ch==8)) {
			if (turning>0) {
				turning=0;
			}
			else {
				turning=-20;

				our_angle-=1;
				if (our_angle<0) our_angle+=64;
			}
		//	printf("Angle %lf\n",our_angle);
		}
		/* right */
		if ((ch=='k') || (ch==21)) {
			if (turning<0) {
				turning=0;
			}
			else {
				turning=20;
				our_angle+=1;
				if (our_angle>63) our_angle-=64;
			}


		}

		if (ch=='z') {
			speed+=0.05;
		}

		if (ch=='x') {
			speed-=0.05;
		}

		if (ch==' ') {
			speed=0;
		}

		dx = speed * cos_lookup (our_angle);
		dy = speed * sin_lookup (our_angle);

		flyx += dx;
		flyy += dy;

		draw_background_mode7(our_angle, flyx, flyy,buffer);

		put_sprite_cropped(buffer,ship_shadow,xx,360);

		if (turning==0) {
			put_sprite_cropped(buffer,ship_forward,xx,yy);
		}
		if (turning<0) {
			put_sprite_cropped(buffer,ship_left,xx,yy);
			turning++;
		}
		if (turning>0) {
			put_sprite_cropped(buffer,ship_right,xx,yy);
			turning--;
		}

		pi_graphics_update(buffer,pal);

		usleep(20000);

	}
	return 0;
}


int mode7_flying(unsigned char *buffer, struct palette *pal) {

	apple2_load_palette(pal);
	flying(buffer,pal);

	return 0;
}

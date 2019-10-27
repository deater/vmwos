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
	0x5,0x3,
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0xaa,0x00,0x00,
	0x00,0x00,0xaa,0x00,0x00,
};

static unsigned char ship_forward[]={
	0x5,0x3,
	0x00,0x00,0x77,0x00,0x00,
	0x50,0x55,0x77,0x55,0x50,
	0x01,0x00,0x07,0x00,0x01,
};

static unsigned char ship_right[]={
	0x5,0x3,
	0x50,0x00,0x70,0x77,0x00,
	0x01,0x55,0x77,0x55,0x50,
	0x00,0x77,0x07,0x00,0x15,
};

static unsigned char ship_left[]={
	0x5,0x3,
	0x00,0x77,0x70,0x00,0x50,
	0x50,0x55,0x77,0x55,0x01,
	0x15,0x00,0x07,0x77,0x00,
};



static int tile_w=16,tile_h=16;


/* http://www.helixsoft.nl/articles/circle/sincos.htm */

static double space_z=2.5; // height of the camera above the plane
static int horizon=-2;    // number of pixels line 0 is below the horizon
static double scale_x=320, scale_y=240;
		// scale of space coordinates to screen coordinates
static double bmp_w=640, bmp_h=480;

//void mode_7 (BITMAP *bmp, BITMAP *tile, fixed angle, fixed cx, fixed cy, MODE_7_PARAMS params)

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

	for (screen_y = 6; screen_y < bmp_h; screen_y++) {
		// first calculate the distance of the line we are drawing
		distance = (space_z * scale_y) / (screen_y + horizon);

		// then calculate the horizontal scale, or the distance between
		// space points on this horizontal line
		horizontal_scale = (distance / scale_x);

		// calculate the dx and dy of points in space when we step
		// through all points on this line
		line_dx = -sin_lookup(angle) * horizontal_scale;
		line_dy = cos_lookup(angle) * horizontal_scale;

		// calculate the starting position
		space_x = cx + (distance * cos_lookup(angle)) - bmp_w/2 * line_dx;
		space_y = cy + (distance * sin_lookup(angle)) - bmp_w/2 * line_dy;

		space_x+=(BETA*space_z*cos_lookup(angle));
		space_y+=(BETA*space_z*sin_lookup(angle));


		// go through all points in this screen line
		for (screen_x = 0; screen_x < bmp_w; screen_x++) {
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

	xx=17;	yy=26;

	color=APPLE2_COLOR_BLACK;

	color=APPLE2_COLOR_MEDIUMBLUE;

	for(i=0;i<20;i++) {
		vmwHlin(0, 40, i, color, buffer);
	}

	color=APPLE2_COLOR_DARKBLUE;
	for(i=20;i<48;i++) {
		vmwHlin(0, 40, i, color, buffer);
	}

	while(1) {
		ch=pi_graphics_input();

		if ((ch=='q') || (ch==27))  break;

		if (ch=='g') {
			BETA+=0.1;
			printf("Horizon=%lf\n",BETA);
		}
		if (ch=='h') {
			BETA-=0.1;
			printf("Horizon=%lf\n",BETA);
		}

		if (ch=='s') {
			scale_x++;
			scale_y++;
			printf("Scale=%lf\n",scale_x);
		}

		/* up */
		if ((ch=='i') || (ch==11)) {
			if (yy>16) {
				yy-=2;
				space_z+=1;
			}

			printf("Z=%lf\n",space_z);
		}
		/* down */
		if ((ch=='m') || (ch==10)) {
			if (yy<30) {
				yy+=2;
				space_z-=1;
			}
			printf("Z=%lf\n",space_z);
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


		{
			dx = speed * cos_lookup (our_angle);
			dy = speed * sin_lookup (our_angle);

		        flyx += dx;
        		flyy += dy;
		}

//		gr_copy(0x800,0x400);

		draw_background_mode7(our_angle, flyx, flyy,buffer);

//		grsim_put_sprite_page(0,ship_shadow,xx,30);

//		if (turning==0) grsim_put_sprite_page(0,ship_forward,xx,yy);
//		if (turning<0) {
//			grsim_put_sprite_page(0,ship_left,xx,yy);
//			turning++;
//		}
//		if (turning>0) {
//			grsim_put_sprite_page(0,ship_right,xx,yy);
//			turning--;
//		}

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

#include <stddef.h>
#include <stdint.h>

//#define RECORD_FLIGHT 1

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

#include "pi_actual.h"
#include "pi_ship.h"
#include "deep_field.h"

#include "flight_replay.h"

/* 24.8 fixed point */
//#define FIXEDSHIFT	8
/* 20.12 fixed point */
#define FIXEDSHIFT	12

static unsigned char flying_map[320][320];


#if FIXEDSHIFT==8
static int sin_lookup_fixed[32]={
	0x00,0x19,0x31,0x4a,
	0x61,0x78,0x8e,0xa2,
	0xb5,0xc5,0xd4,0xe1,
	0xec,0xf4,0xfb,0xfe,
	0x100,0xfe,0xfb,0xf4,
	0xec,0xe1,0xd4,0xc5,
	0xb5,0xa2,0x8e,0x78,
	0x61,0x4a,0x31,0x19,
};
#else
static int sin_lookup_fixed[32]={
	0x000,0x191,0x31f,0x4a5,
	0x61f,0x78a,0x8e3,0xa26,
	0xb50,0xc5e,0xd4d,0xe1c,
	0xec8,0xf4f,0xfb1,0xfec,
	0x1000,0xfec,0xfb1,0xf4f,
	0xec8,0xe1c,0xd4d,0xc5e,
	0xb50,0xa26,0x8e3,0x78a,
	0x61f,0x4a5,0x31f,0x191,
};

#endif



	/* 0...63 */
static int sin_fixed(int which) {

	which=(which%64);
	if (which<32) return sin_lookup_fixed[which];
	else return -sin_lookup_fixed[which-32];
}

static int cos_fixed(int which) {

	which=which+16;
	return sin_fixed(which);
}



/* Ship Sprites */
static unsigned char ship_shadow[2+52*32];
static unsigned char ship_forward[2+52*32];
static unsigned char ship_right[2+52*32];
static unsigned char ship_left[2+52*32];

#define DEEP_XSIZE	2560
#define DEEP_YSIZE	73

static unsigned char deep_field[DEEP_XSIZE*DEEP_YSIZE];

static int fixed_mul(int a, int b) {

	long long c;

	c=(long long)a*(long long)b;

	c>>=FIXEDSHIFT;

	return c;
}

static int fixed_div(int a, int b) {

	int c;

	if (b==0) {
		printf("Divide by zero!\n");
		return 0;
	}

	/*   25 8000 */
	/* 2580 0000 */
	/* 5800 0000 */

	/* HACK, if not fit in 12-bit then calculate in 8 bit */
	c=a<<FIXEDSHIFT;
	if ((c>>FIXEDSHIFT)!=a) {
		//printf("problem A=%x C=%x\n",a,c);
		c=a<<8;
		c/=b;
		c<<=4;
	}
	else {
		c/=b;
	}

	return c;
}



/* http://www.helixsoft.nl/articles/circle/sincos.htm */

/* height of the camera above the plane */

#if (FIXEDSHIFT==8)
#define DELTAV	0xC
#else
#define DELTAV  0xC0
#endif

#if (FIXEDSHIFT==8)
#define SPACEZ	0x0280
#else
#define SPACEZ  0x2800
#endif

static int space_z=SPACEZ; /* 2.5 */

/* number of pixels line 0 is below the horizon */
static int horizon=-68;		// -2
/* scale of space coordinates to screen coordinates */
#define SCALE_X 320
#define SCALE_Y 240
#define BMP_W	640
#define BMP_H	480

/* -0.6 = ffffff67 */
#if (FIXEDSHIFT==8)
#define BETAF 0xffffff67;
#else
//#define BETAF 0xfffff667;
#define BETAF 0xfffff367;
#endif

static int BETA=BETAF;

void draw_background_mode7(int angle, int cx, int cy,
		unsigned char *buffer) {

	int color;

	// current screen position
	int screen_x, screen_y;

	// the distance and horizontal scale of the line we are drawing
	int distance,horizontal_scale;

	// masks to make sure we don't read pixels outside the tile

	// step for points in space between two pixels on a horizontal line
	int  line_dx, line_dy;

	// current space position
	int space_x, space_y;

//	clear_screens();

	for (screen_y = 72; screen_y < BMP_H; screen_y++) {
		// first calculate the distance of the line we are drawing
		distance =
			fixed_div(fixed_mul(space_z, SCALE_Y<<FIXEDSHIFT),
			( (screen_y<<FIXEDSHIFT) + (horizon<<FIXEDSHIFT)));

		// then calculate the horizontal scale, or the distance between
		// space points on this horizontal line
		horizontal_scale = fixed_div(distance,SCALE_X<<FIXEDSHIFT);

		line_dx = fixed_mul(-sin_fixed(angle) , horizontal_scale);
		line_dy = fixed_mul(cos_fixed(angle) , horizontal_scale);

		// calculate the starting position
		space_x = cx +
			fixed_mul(distance , cos_fixed(angle)) -
			fixed_mul( (BMP_W/2)<<FIXEDSHIFT,line_dx);
		space_y = cy +
			fixed_mul(distance , sin_fixed(angle)) -
			fixed_mul( (BMP_W/2)<<FIXEDSHIFT,line_dy);

		space_x+=fixed_mul(BETA,fixed_mul(space_z,cos_fixed(angle)));
		space_y+=fixed_mul(BETA,fixed_mul(space_z,sin_fixed(angle)));


		// go through all points in this screen line
		for (screen_x = 0; screen_x < BMP_W; screen_x++) {
			// get a pixel from the tile and put it on the screen

			int tempx,tempy;

			tempx=space_x>>FIXEDSHIFT;
			tempy=space_y>>FIXEDSHIFT;

			if ((tempx>319) || (tempx<0) ||
				(tempy>193) || (tempy<0)) {
				color=16+((tempx&0x3)<<2)+((tempy&0x3));
//				if (((tempx&0xf)==0xf) || ((tempy&0xf)==0xf)) {
//					color=15;
//				}
//				else {
//					color=0;
//				}
			}
			else {
				color=(flying_map[tempx][tempy]);
			}
			vmwPlot(screen_x,screen_y,color,buffer);

			// advance to the next position in space
			space_x += line_dx;
			space_y += line_dy;
		}
	}
}

static void update_sky(int angle, unsigned char *buffer) {
	int i;

	/* FIXME: should properly wrap when we go past edge */

	for(i=0;i<DEEP_YSIZE;i++) {
		memcpy(&buffer[i*XSIZE],
			&deep_field[i*DEEP_XSIZE+(40*angle)],XSIZE);
	}
}

int flying(unsigned char *buffer, struct palette *pal) {

	unsigned char ch;
	int xx,yy;
	int turning=0;
	// 1034880
	int flyx=611693,flyy=3363;
	int our_angle=0;
	int dx,dy,speed=0;
	int framecount=0,lastframe=0;
	int input_offset=0;

	/************************************************/
	/* Flying					*/
	/************************************************/

	xx=318;	yy=312;

	/* Make sky */
//	color=3;
//	for(i=0;i<72;i++) {
//		vmwHlin(0, 640, i, color, buffer);
//	}

	update_sky(our_angle,buffer);

	lastframe=flight_replay[input_offset].frames;

	while(1) {
#ifdef RECORD_FLIGHT
		ch=pi_graphics_input();

		if (ch!=0) {
			printf("\t{%d,\'%c\'},\n",framecount-lastframe,ch);
			lastframe=framecount;
		}
#else

		if (lastframe==0) {
			ch=flight_replay[input_offset].keypress;
			input_offset++;
			lastframe=flight_replay[input_offset].frames;
		}
		else {
			ch=0;
		}
#endif
		if ((ch=='q') || (ch==27))  break;

		/* up */
		if ((ch=='i') || (ch==11)) {
			if (yy>192) {
				yy-=24;
				space_z+=0x1<<FIXEDSHIFT;
			}

//			printf("Z=%lf\n",space_z);
		}
		/* down */
		if ((ch=='m') || (ch==10)) {
			if (yy<360) {
				yy+=24;
				space_z-=0x1<<FIXEDSHIFT;
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
				update_sky(our_angle,buffer);
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
				update_sky(our_angle,buffer);
			}


		}

		if (ch=='z') {
			speed+=DELTAV;	/* 0.05 roughly */
		}

		if (ch=='x') {
			speed-=DELTAV;	/* 0.05 roughly */
		}

		if (ch==' ') {
			printf("Location: %d,%d\n",flyx,flyy);
			speed=0;
		}

		dx = fixed_mul( speed, cos_fixed(our_angle));
		dy = fixed_mul( speed, sin_fixed(our_angle));

		flyx = flyx+dx;
		flyy = flyy+dy;

		draw_background_mode7(our_angle,
			flyx,
			flyy,
			buffer);

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

#ifdef RECORD_FLIGHT
#else
		lastframe--;
#endif
		framecount++;
		pi_graphics_update(buffer,pal);
#ifdef VMWOS
#else
		usleep(33000);
#endif
	}
	return 0;
}


int mode7_flying(unsigned char *buffer, struct palette *pal) {

	int x,y;

	vmwPCXLoadPalette(pi_actual_pcx, pi_actual_pcx_len-769, pal);
        vmwLoadPCX(pi_actual_pcx,0,0, buffer, XSIZE);
	for(y=0;y<194;y++) {
		for(x=0;x<320;x++) {
			flying_map[x][y]=buffer[y*XSIZE+x];
		}
	}

	/* load ship images */
	ship_shadow[0]=52; 	ship_shadow[1]=32;
	ship_forward[0]=52;	ship_forward[1]=32;
	ship_right[0]=52;	ship_right[1]=32;
	ship_left[0]=52;	ship_left[1]=32;

        vmwLoadPCX(pi_ship_pcx,0,0, buffer, XSIZE);

	for(y=0;y<32;y++) {
		for(x=0;x<52;x++) {
			ship_left[2+(y*52)+x]=buffer[y*XSIZE+x];
			ship_forward[2+(y*52)+x]=buffer[y*XSIZE+x+52];
			ship_right[2+(y*52)+x]=buffer[y*XSIZE+x+104];
			ship_shadow[2+(y*52)+x]=buffer[y*XSIZE+x+156];
		}
	}

	/* load deep field image */
        vmwLoadPCX(deep_field_pcx, 0,0, deep_field, DEEP_XSIZE);

//	{
//		int q,a;
//
//		for(a=0;a<72;a++) {
//			for(q=0;q<640;q++) {
//				buffer[a*XSIZE+q]=deep_field[a*DEEP_XSIZE+q];
//			}
//		}

//		pi_graphics_update(buffer,pal);
//		sleep(10);
//	}

	flying(buffer,pal);

	vmwFadeToBlack(buffer, pal);

	return 0;
}

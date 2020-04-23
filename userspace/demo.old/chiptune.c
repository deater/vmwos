/* Play PT3 chiptunes on a Raspberry Pi */

#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#define vmwos_malloc malloc
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "pt3_lib.h"
#include "ayemu.h"

#include "svmwgraph.h"
#include "pi-graphics.h"

#define FREQ	44100
#define CHANS	1
#define BITS	16

/* global variables */
//volatile uint32_t TimeDelay;
//volatile uint32_t overflows=0;

static ayemu_ay_t ay,ay2;
static struct pt3_song_t pt3,pt3_2;

static ayemu_ay_reg_frame_t frame;
static ayemu_ay_reg_frame_t frame2;


#define MAX_SONGS 1
#include "dotd.h"

static int which_song=0;
static int song_done=0;

//static struct pt3_image_t pt3_image[] ={
//	[0] = {	.data=__I2_PT3,	.length=__I2_PT3_len, },
//};

static int line=0,subframe=0,current_pattern=0;

static void change_song(void) {

	which_song=0;

	struct pt3_image_t pt3_image;
	pt3_image.data=__dya_dotd_pt3;
	pt3_image.length=__dya_dotd_pt3_len;
//	[0] = {	.data=__I2_PT3,	.length=__I2_PT3_len, },
//};
	pt3_load_song(&pt3_image, &pt3, &pt3_2);

	current_pattern=0;
	line=0;
	subframe=0;
}

/* mono (2 channel), 16-bit (2 bytes), play at 50Hz */
#define AUDIO_BUFSIZ (FREQ*CHANS*(BITS/8) / 50)
#define NUM_SAMPLES (AUDIO_BUFSIZ/CHANS/(BITS/8))
#define COUNTDOWN_RESET (FREQ/50)

static unsigned char audio_buf[AUDIO_BUFSIZ];
static unsigned char audio_buf2[AUDIO_BUFSIZ];
static int output_bufsize=8*1024*1024,totalsize=0;
static uint32_t *output_buffer;

/* Interrupt Handlers */
static void NextBuffer(int which_half) {

	int line_decode_result=0;

	/* Decode next frame */
	if ((line==0) && (subframe==0)) {
		if (current_pattern==pt3.music_len) {
			song_done=1;
			return;
		}
		pt3_set_pattern(current_pattern,&pt3);
		if (pt3_2.valid) pt3_set_pattern(current_pattern,&pt3_2);
	}

	if (subframe==0) {
		line_decode_result=pt3_decode_line(&pt3);
		if (pt3_2.valid) pt3_decode_line(&pt3_2);
	}

	if (line_decode_result==1) {
		/* line done early? */
		current_pattern++;
		line=0;
		subframe=0;
	}
	else {
		subframe++;
		if (subframe==pt3.speed) {
			subframe=0;
			line++;
			if (line==64) {
				current_pattern++;
				line=0;
			}
		}
	}

	pt3_make_frame(&pt3,frame);
	if (pt3_2.valid) {
		pt3_make_frame(&pt3_2,frame2);
	}

	/* Update AY buffer */
	ayemu_set_regs(&ay,frame);
	ayemu_set_regs(&ay2,frame2);

//	printf("Current pattern: %d / %d\n",current_pattern, line);

	/* Generate sound buffer */
	ayemu_gen_sound (&ay, audio_buf, AUDIO_BUFSIZ);
	if (pt3_2.valid) {
		ayemu_gen_sound (&ay2, audio_buf2, AUDIO_BUFSIZ);
	}

}



int start_playing_pt3(unsigned char *buffer, struct palette *pal)  {

	int old_pattern=0,sound_started=0;
	struct timespec t;
	char tbuffer[3];

	/* Init first song */
	printf("Loading song\n");
	change_song();

	/* Init ay code */
	printf("Init AY library\n");
	ayemu_init(&ay);
	ayemu_init(&ay2);
	// 44100, 1, 16 -- freq, channels, bits
	ayemu_set_sound_format(&ay, FREQ, CHANS, BITS);
	ayemu_set_sound_format(&ay2, FREQ, CHANS, BITS);

	printf("Reset AY library\n");
	ayemu_reset(&ay);
	ayemu_reset(&ay2);
	ayemu_set_chip_type(&ay, AYEMU_AY, NULL);
	ayemu_set_chip_type(&ay2, AYEMU_AY, NULL);
	/* Assume mockingboard/VMW-chiptune freq */
	/* pt3_lib assumes output is 1773400 of zx spectrum */
	ayemu_set_chip_freq(&ay, 1773400);
	ayemu_set_stereo(&ay, AYEMU_MONO, NULL);

	ayemu_set_chip_freq(&ay2, 1773400);
	ayemu_set_stereo(&ay2, AYEMU_MONO, NULL);

	printf("Allocate RAM\n");
	output_buffer=vmwos_malloc(output_bufsize*sizeof(uint32_t)*2);
	if (output_buffer==NULL) {
		printf("Error allocating memory!\n");
		return -1;
	}
	printf("Allocated %d bytes at %p\n",
			(int)(output_bufsize*sizeof(uint32_t)),output_buffer);

	printf("Decode Song\n");
	while(!song_done) {
		int i,temp,temp2;
		NextBuffer(0);
		if (current_pattern!=old_pattern) {
			clock_gettime(CLOCK_REALTIME,&t);
			tbuffer[0]='0'+(30-current_pattern)/10;
			tbuffer[1]='0'+(30-current_pattern)%10;
			tbuffer[2]=0;

			vmwTextXYx4(tbuffer,288,400,0,DEFAULT_FONT,buffer);
			vmwTextXYx4(tbuffer,288,400,150,DEFAULT_FONT,buffer);

			pi_graphics_update(buffer,pal);

			printf("New pattern: %d at time %d\n",
				current_pattern,(int)t.tv_sec);
			old_pattern=current_pattern;

#ifdef VMWOS
			/* Start playing early */
			if (!sound_started) {
				//vmwos_play_sound(output_buffer,totalsize*4*2,1);
				vmwos_play_sound(output_buffer,5081202*4*2,1);
				sound_started=1;
			}
#else
			usleep(50000);
#endif

		}

		for(i=0;i<AUDIO_BUFSIZ/2;i++) {
			/* 14-bit? */
			temp=(audio_buf[i*2]&0xff);
			temp|=audio_buf[(i*2)+1]<<8;
			if (pt3_2.valid) {
				temp2=(audio_buf2[i*2]&0xff);
				temp2|=audio_buf2[(i*2)+1]<<8;
			}
			else {
				temp2=temp;
			}
			/* write left (?) channel */
			output_buffer[(i+totalsize)*2]=temp>>2;
			/* write right (?) channel */
			output_buffer[((i+totalsize)*2)+1]=temp2>>2;
		}
		//memcpy(output_buffer+totalsize,audio_buf,AUDIO_BUFSIZ);
		totalsize+=AUDIO_BUFSIZ/2;



	}


	printf("Total size=%d\n",totalsize);


	printf("After play\n");


//	FILE *fff;

//	fff=fopen("out.bin","w");
//	if (fff==NULL) {
//		fprintf(stderr,"Error opening\n");
//		return -1;
//	}
//	fwrite(output_buffer,totalsize,1,fff);
//	fclose(fff);

	return 0;
}

/* Play PT3 chiptunes on a Raspberry Pi */

#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "pt3_lib.h"
#include "ayemu.h"

#define FREQ	44100
#define CHANS	1
#define BITS	16

/* global variables */
volatile uint32_t TimeDelay;
volatile uint32_t overflows=0;

static ayemu_ay_t ay;
static struct pt3_song_t pt3,pt3_2;

static ayemu_ay_reg_frame_t frame;
//static unsigned char frame[14];

#define MAX_SONGS 1
#include "i2_pt3.h"

static int which_song=0;
static int song_done=0;

//static struct pt3_image_t pt3_image[] ={
//	[0] = {	.data=__I2_PT3,	.length=__I2_PT3_len, },
//};

static int line=0,subframe=0,current_pattern=0;

static struct pt3_image_t pt3_image;

static void change_song(void) {

	which_song=0;

	pt3_image.data=__I2_PT3;
	pt3_image.length=__I2_PT3_len;
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
	}

	if (subframe==0) {
		line_decode_result=pt3_decode_line(&pt3);
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

	/* Update AY buffer */
	ayemu_set_regs(&ay,frame);

//	printf("Current pattern: %d / %d\n",current_pattern, line);

	/* Generate sound buffer */
	ayemu_gen_sound (&ay, audio_buf, AUDIO_BUFSIZ);

}

static unsigned char pt3file[8192];

int main(int argc, char **argv) {

	int old_pattern=0;
	struct timespec t;
	int fd,result;

	/* Init first song */
	printf("Loading song\n");

	if (argc<2) {
		change_song();
	}
	else {
		fd=open(argv[1],O_RDONLY,0);
		if (fd<0) {
			printf("Error opening %s\n",argv[1]);
			return -1;
		}
		result=read(fd,pt3file,8192);
		if (result<0) {
			printf("Error reading!\n");
			return -1;
		}
		close(fd);

		printf("Read %d bytes from %s\n",result,argv[1]);

		which_song=0;
		pt3_image.data=pt3file;
		pt3_image.length=result;
		pt3_load_song(&pt3_image, &pt3, &pt3_2);

		current_pattern=0;
		line=0;
		subframe=0;
	}


	/* Init ay code */
	printf("Init AY library\n");
	ayemu_init(&ay);
	// 44100, 1, 16 -- freq, channels, bits
	ayemu_set_sound_format(&ay, FREQ, CHANS, BITS);

	printf("Reset AY library\n");
	ayemu_reset(&ay);
	ayemu_set_chip_type(&ay, AYEMU_AY, NULL);
	/* Assume mockingboard/VMW-chiptune freq */
	/* pt3_lib assumes output is 1773400 of zx spectrum */
	ayemu_set_chip_freq(&ay, 1773400);
//	ayemu_set_chip_freq(&ay, 1000000);
	ayemu_set_stereo(&ay, AYEMU_MONO, NULL);

	printf("Allocate RAM\n");
	output_buffer=vmwos_malloc(output_bufsize*sizeof(uint32_t)*2);
	if (output_buffer==NULL) {
		printf("Error allocating memory!\n");
		return -1;
	}
	printf("Allocated %d bytes at %p\n",output_bufsize*sizeof(uint32_t),
			output_buffer);

	printf("Decode Song\n");
	while(!song_done) {
		int i,temp;
		NextBuffer(0);
		if (current_pattern!=old_pattern) {
			clock_gettime(CLOCK_REALTIME,&t);
			printf("New pattern: %d at time %d\n",
				current_pattern,t.tv_sec);
			old_pattern=current_pattern;
		}

		for(i=0;i<AUDIO_BUFSIZ/2;i++) {
			/* 14-bit? */
			temp=(audio_buf[i*2]&0xff);
			temp|=audio_buf[(i*2)+1]<<8;
			/* write left (?) channel */
			output_buffer[(i+totalsize)*2]=temp>>2;
			/* write right (?) channel */
			output_buffer[((i+totalsize)*2)+1]=temp>>2;
		}
		//memcpy(output_buffer+totalsize,audio_buf,AUDIO_BUFSIZ);
		totalsize+=AUDIO_BUFSIZ/2;
	}


	printf("Total size=%d\n",totalsize);


	vmwos_play_sound(output_buffer,totalsize*4*2,1);

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

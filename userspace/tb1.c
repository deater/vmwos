#define XSPEED	10
#define YSPEED	10
#define EXPLOSION_LENGTH 10
#define NUM_MISSILES 10

struct missile_type {
	int x,y;
	int exploding;
	int out;
} missiles[NUM_MISSILES];

struct explosion_type {
	int x,y;
	int out;
} explosions[NUM_MISSILES];

int framebuffer_tb1(void) {

	char ch;
	int x=400,y=550;
	int xspeed=0;
	int i;

	for(i=0;i<NUM_MISSILES;i++) {
		missiles[i].exploding=0;
		missiles[i].out=0;
		explosions[i].out=0;
	}

	while(1) {

		ch=uart_getc_noblock();
		framebuffer_console_push();	/* does framebuffer push */

		framebuffer_clear_screen(0);


		switch(ch) {
			case ' ':
				for(i=0;i<NUM_MISSILES;i++) {
					if (!missiles[i].out) {
						missiles[i].x=x;
						missiles[i].y=y;
						missiles[i].out=1;
						break;
					}
				}
				break;
			case 'j':
				xspeed-=XSPEED;
				break;
			case 'k':
				xspeed+=XSPEED;
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

		framebuffer_console_putchar(0xffffff,0x0,'A',x,y);

		for(i=0;i<NUM_MISSILES;i++) {
			if (missiles[i].out) {

#if 0
			int u;

			u=framebuffer_console_val(missiles[i].x/8,
							missiles[i].y/16);

			framebuffer_console_putchar(0xff0000,0x0,
				(u%10)+'0',
				100,100);

			u/=10;

			framebuffer_console_putchar(0xff0000,0x0,
				(u%10)+'0',
				110,110);

			u/=10;

			framebuffer_console_putchar(0xff0000,0x0,
				(u%10)+'0',
				120,120);
#endif
			/* Move missiles */

			missiles[i].y-=YSPEED;

			if (missiles[i].y<0) {
				missiles[i].out=0;
			}
			else {
				framebuffer_console_putchar(0x00ff,
					0x0,'!',missiles[i].x,missiles[i].y);

			/* check for collision */
			if (framebuffer_console_val(missiles[i].x/8,
						missiles[i].y/16)!=' ') {
				missiles[i].out=0;
				explosions[i].out=EXPLOSION_LENGTH;
				explosions[i].x=missiles[i].x;
				explosions[i].y=missiles[i].y;
				text_console[(missiles[i].x/8)+
						(missiles[i].y/16)*CONSOLE_X]=' ';
			}

			}
			}

		if (explosions[i].out) {
			int explosion_color;

			explosion_color=(explosions[i].out*256)/EXPLOSION_LENGTH;
			explosion_color=explosion_color+(explosion_color<<8);
			explosion_color<<=8;
			explosions[i].out--;
			framebuffer_console_putchar(explosion_color,0x0,
					'*',explosions[i].x,explosions[i].y);
		}
		}

	}

	return 0;
}

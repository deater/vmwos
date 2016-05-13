#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define FORE_BLACK      0x0
#define FORE_BLUE       0x1
#define FORE_GREEN      0x2
#define FORE_CYAN       0x3
#define FORE_RED        0x4
#define FORE_PURPLE     0x5
#define FORE_BROWN      0x6
#define FORE_GREY       0x7
#define FORE_DGREY      0x8
#define FORE_LBLUE      0x9
#define FORE_LGREEN     0xa
#define FORE_LCYAN      0xb
#define FORE_LRED       0xc
#define FORE_PINK       0xd
#define FORE_YELLOW     0xe
#define FORE_WHITE      0xf

#define BACK_BLACK      0
#define BACK_BLUE       1
#define BACK_GREEN      2
#define BACK_CYAN       3
#define BACK_RED        4
#define BACK_PURPLE     5
#define BACK_BROWN      6
#define BACK_GREY       7

#define ANSI_BLACK      0
#define ANSI_RED        1
#define ANSI_GREEN      2
#define ANSI_BLUE       4
#define ANSI_GREY       7

int console_fore_color,console_back_color;
int console_x=0,console_y=0;
int console_fore_bright=0;

char *text_console,*text_color;

#define CONSOLE_X 80
#define CONSOLE_Y 25

/* Arbitrary, can't find a spec although more than 3 shouldn't be necessary */
#define ANSI_MAX_NUMBERS 10

#define ANSI_STATE_NORMAL	0
#define ANSI_STATE_ESCAPE	1
#define ANSI_STATE_NUMBER	2
#define ANSI_STATE_COMPLETE	3

#define ANSI_DEFAULT		0xffffffff

static uint32_t numbers[ANSI_MAX_NUMBERS];
static int32_t which_number=-1;
static uint32_t ansi_state=ANSI_STATE_NORMAL;
static uint32_t ansi_command;

int framebuffer_console_write(const char *buffer, int length) {

	int i=0,x;
	int refresh_screen=0;
	int distance;
	int c;

	while(1) {
	//	printf("!AS=%d,%c!",ansi_state,buffer[i]);

		if (ansi_state==ANSI_STATE_NORMAL) {
			if (buffer[i]=='\r') {
				/* console_x=0; */
			} else if (buffer[i]=='\n') {
				console_x=0;
				console_y++;
				printf("\n");
			} else if (buffer[i]=='\t') {
				console_x=(console_x+8)&(~0x7);
			} else if (buffer[i]=='\b') {
				console_x--;
				if (console_x<0) console_x=0;
				refresh_screen=1;
			} else if (buffer[i]==27) {
				printf("ESC");
				ansi_state=ANSI_STATE_ESCAPE;
			}

			else {
				if (buffer[i]<128) printf("%c",buffer[i]);
				else printf("<%x>",buffer[i]);

				/* Write out the characters */
				/* If overwriting previous char, need to refresh */
				if (text_console[console_x+(console_y*CONSOLE_X)]!=' ') {
					refresh_screen=1;
				}
				text_console[console_x+(console_y*CONSOLE_X)]=buffer[i];
				text_color[console_x+(console_y*CONSOLE_X)]=
					(console_back_color<<4 |
					(console_fore_color&0xf));

				console_x++;

			}



		}
		else if (ansi_state==ANSI_STATE_ESCAPE) {
			if (buffer[i]=='[') {
				printf("[");
				which_number=-1;
				numbers[0]=ANSI_DEFAULT;
				ansi_state=ANSI_STATE_NUMBER;
			}
			else {
				ansi_state=ANSI_STATE_NORMAL;
			}
		}
		else if (ansi_state==ANSI_STATE_NUMBER) {
			int val;
			val=buffer[i];

			/* If not a number */
			if ((val<'0') || (val>'9')) {
				/* ; separated list, move to next number */
				if (val==';') {
					which_number++;
					if (which_number==ANSI_MAX_NUMBERS) {
						ansi_state=ANSI_STATE_NORMAL;
						printf("ANSI: Too many numbers!\n");
						break;
					}
					numbers[which_number]=0;
				}
				else {
					which_number++;
					ansi_command=val;
					ansi_state=ANSI_STATE_COMPLETE;
				}
			}

			else {
				if (which_number==-1) {
					which_number=0;
					numbers[0]=0;
				}
				numbers[which_number]*=10;
				numbers[which_number]+=val-'0';
			}
		}

		if (ansi_state==ANSI_STATE_COMPLETE) {

			switch(ansi_command) {
				case 'A':
					/* cursor up */
					if (numbers[0]==ANSI_DEFAULT) {
						distance=1;
						printf("A");
					}
					else {
						distance=numbers[0];
						printf("%dA",distance);
					}
					console_y-=distance;
					break;
				case 'B':
					/* cursor down */
					if (numbers[0]==ANSI_DEFAULT) {
						distance=1;
						printf("B");
					}
					else {
						distance=numbers[0];
						printf("%dB",distance);
					}
					console_y+=distance;
					break;
				case 'C':
					/* cursor forward */
					if (numbers[0]==ANSI_DEFAULT) {
						distance=1;
						printf("C");
					}
					else {
						distance=numbers[0];
						printf("%dC",distance);
					}
					console_x+=distance;
					break;
				case 'D':
					/* cursor backward */
					if (numbers[0]==ANSI_DEFAULT) distance=1;
					else distance=numbers[0];
					console_x-=distance;
					printf("%dD",distance);
					break;
				case 'H':
					/* GotoXY */
					if (numbers[0]==ANSI_DEFAULT) {
						console_x=0;
						console_y=0;
						printf("H");
					}
					else if (which_number==0) {
						/* 1-based */
						if (numbers[0]<1) {
							printf("Errror!\n");
						} else {
							console_y=numbers[0]-1;
						}
						printf("%dH",numbers[0]);
					}
					else {
						/* 1-based */
						if (numbers[0]<1) {
							printf("Error!\n");
						}
						else {
							console_y=numbers[0]-1;
						}
						if (numbers[1]<1) {
							printf("Error!\n");
						}
						else {
							console_x=numbers[1]-1;
						}
						printf("%d;%dH",numbers[0],
								numbers[1]);

					}
					break;
				case 'J':
					switch(numbers[0]) {

					case 0:
					case ANSI_DEFAULT:
						/* clear to end of screen */
					case 1:
						/* clear to beginning of screen */
					default:
						printf("ANSI: unknown clear %d\n",numbers[0]);
						break;
					case 2:
						/* clear all of screen */
						//framebuffer_console_clear();
						//framebuffer_console_home();
						break;
					}

					printf("%dJ",numbers[0]);
					break;
				case 'm':
					/* colors */
					for(c=0;c<which_number;c++) {
						printf("%d",numbers[c]);
						if (c<which_number-1) printf(";");

						if ((numbers[c]==0) ||
							(numbers[c]==ANSI_DEFAULT)) {

							console_fore_color=ANSI_GREY;
							console_back_color=ANSI_BLACK;
							console_fore_bright=0;
						}
						if (numbers[c]==1) {
							console_fore_bright=1;
							console_fore_color|=(1<<3);
                                                }

						/* Foreground Colors */
						if ((numbers[c]>=30)&&(numbers[c]<=37)) {
							console_fore_color=(numbers[c]-30)|(console_fore_bright<<3);
						}

						/* FIXME Color 38 used for 24-bit color support */

						/* Background Colors */
						if ((numbers[c]>=40)&&(numbers[c]<=47)) {
							console_back_color=numbers[c]-40;
						}
					}
					printf("m");
					break;
				default:
					printf("Unknown ansi command \'%c\'",
							ansi_command);
						break;
			}

			/* reset state */
			which_number=-1;
			ansi_state=ANSI_STATE_NORMAL;
		}
		printf("=%d,%d=\n",console_x,console_y);
		/* Bounds check */
		if (console_x<0) console_x=0;

		if (console_y<0) console_y=0;

		if (console_x>=CONSOLE_X) {
//			printf("\nWrapping x=%d\n",console_x);
			console_x=console_x%CONSOLE_X;
			if (console_x>CONSOLE_X) {
				printf("console x too big!\n");
				exit(-1);
			}
			console_y++;
		}

		if (console_y>=CONSOLE_Y) {


			/* scroll up a line */

			refresh_screen=1;

			memcpy(&(text_console[0]),&(text_console[CONSOLE_X]),
				(CONSOLE_Y-1)*CONSOLE_X*sizeof(unsigned char));
			memcpy(&(text_color[0]),&(text_color[CONSOLE_X]),
				(CONSOLE_Y-1)*CONSOLE_X*sizeof(unsigned char));

			for(x=0;x<CONSOLE_X;x++) {
				text_console[x+(CONSOLE_Y-1)*CONSOLE_X]=' ';
				text_color[x+(CONSOLE_Y-1)*CONSOLE_X]=FORE_GREY|BACK_BLACK;
			}

			console_y--;
		}

		/* Go to next character */
		i++;
		if (i==length) break;

	}

	if (refresh_screen) {
//		framebuffer_clear_screen(0);
	}

//	framebuffer_console_push();

	return 0;
}

#define BUFFERSIZE 128

int main(int argc, char **argv) {

	char buffer[BUFFERSIZE];
	int result;

	text_console=calloc(CONSOLE_X*CONSOLE_Y,sizeof(char));
	text_color=calloc(CONSOLE_X*CONSOLE_Y,sizeof(char));

	while(1) {
		result=read(0,buffer,BUFFERSIZE);
		if (result<1) break;
		framebuffer_console_write(buffer,result);
	}

	free(text_console);
	free(text_color);

	return 0;
}

#include <stddef.h>
#include <stdint.h>
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"

//#include <stdio.h>
//#include <string.h>
//#include <termios.h>

static int parse_input(char *string);

#define VERSION "10.0"

int main(int argc, char **argv) {

	char input_string[256];
	int input_pointer,result;
	int ch,done=0;

	struct termios oldt, newt;

//	register long sp asm ("sp");

//	printf("Our sp=%x\r\n",sp);

	tcgetattr( 0, &oldt);
	newt = oldt;

	cfmakeraw(&newt);

	tcsetattr(0, TCSANOW, &newt);

	while (1) {
		input_pointer=0;

		printf("] ");

		while(1) {

			while(1) {
				ch=getchar();
				if (ch) break;

				asm volatile(
					"mov r1,#65536\n"
					"a_loop:\n"
					"subs   r1,r1,#1\n"
					"bne    a_loop\n"
					::: "r1", "memory");

			}

//			printf("VMW: input %d\n",input_pointer);

//			printf("VMW: %d\n",ch);

			if ((ch=='\n') || (ch=='\r')) {
				printf("\r\n");
				input_string[input_pointer]=0;
				result=parse_input(input_string);
				if (result==1) done=1;
				break;
			}
			if (ch==4) {
				done=1;
				break;
			}

			if ((ch==0x7f) || (ch=='\b')) {

				if (input_pointer>0) {
					input_pointer--;
					printf("\b \b");
				}
			}
			else {
				input_string[input_pointer]=ch;
				input_pointer++;
				putchar(ch);
			}
		}
		if (done) break;
	}
	tcsetattr( 0, TCSANOW, &oldt);

	return 0;
}


static int print_help(void) {

	printf("VMWos Shell Version %s\r\n\r\n",VERSION);
	printf("\tblink on/off - turns on/off heartbeat LED\r\n");
	printf("\tcls          - clears the screen\r\n");
	printf("\tcolor X      - set text to color #X\r\n");
	printf("\techo X       - prints string X\r\n");
	printf("\tfont X       - sets the font to font #X\r\n");
	printf("\tgetpid       - print current process ID\r\n");
	printf("\tgradient     - make background look cool\r\n");
	printf("\thelp         - prints this help message\r\n");
	printf("\treset        - reset the machine\r\n");
	printf("\trun X        - run program #X\r\n");
	printf("\tstop X       - stop program #X\r\n");
	printf("\ttime         - print seconds since boot\r\n");
	printf("\ttb1          - play TB1\r\n");
	printf("\tver          - print version\r\n");
	printf("\r\n");

	return 0;
}

static int parse_input(char *string) {

	int result=0;

	if (!strncmp(string,"echo",4)) {
		printf("%s\r\n",string+5);
	}
	else if (!strncmp(string,"cls",3)) {
		printf("\n\r\033[2J\r\n");
	}
	else if (!strncmp(string,"font ",5)) {
		vmwos_setfont(string[5]);
	}
	else if (!strncmp(string,"gradient",8)) {
		vmwos_gradient();
	}
	else if (!strncmp(string,"getpid",6)) {
		printf("Current pid: %d\r\n",getpid());
	}
	else if (!strncmp(string,"exit",4)) {
		result=1;
	}
	else if (!strncmp(string,"blink ",6)) {
		vmwos_blink(string[6]);
	}
	else if (!strncmp(string,"help",4)) {
		result=print_help();
	}
	else if (!strncmp(string,"run ",4)) {
		result=vmwos_run(string[4]);
	}
	else if (!strncmp(string,"stop ",5)) {
		result=vmwos_stop(string[5]);
	}
	else if (!strncmp(string,"color ",6)) {
		printf("%c[3%cm\r\n",27,string[6]);
	}
	else if (!strncmp(string,"ver",3)) {
		printf("VMWos Shell version %s\r\n",VERSION);
	}
	else if (!strncmp(string,"time",4)) {
		printf("Time since boot: %ds\r\n",time(NULL));
	}
	else if (!strncmp(string,"reset",5)) {
		printf("Resetting...\r\n");
		reboot();
	}
	else if (!strncmp(string,"tb1",3)) {
		vmwos_tb1();
	}
	else {
		printf("\r\nUnknown commmand: \"%s\"!\r\n",string);
	}

	return result;
}




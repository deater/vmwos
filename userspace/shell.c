#include <stddef.h>
#include <stdint.h>
//#include "stdio.h"
#include "syscalls.h"
#include "vlibc.h"

//#include <stdio.h>
//#include <string.h>
//#include <termios.h>



#define VERSION "4.0"

static int parse_input(char *string) {

	int result=0;

	if (!strncmp(string,"echo",4)) {
		printf("%s\r\n",string+5);
	}
	else if (!strncmp(string,"exit",4)) {
		result=1;
	}
	else if ((string[0]=='o') && (string[1]=='n')) {
//		result=syscall1(1,SYSCALL_BLINK);
	}
	else if ((string[0]=='o') && (string[1]=='f')) {
//		result=syscall1(0,SYSCALL_BLINK);
	}
	else {
		printf("\r\nUnknown commmand: \"%s\"!\r\n",string);
	}

	return result;
}



int main(int argc, char **argv) {

	char input_string[256];
	int input_pointer,result;
	int ch,done=0;

	static struct termios oldt, newt;

	tcgetattr( 0, &oldt);
	newt = oldt;

	cfmakeraw(&newt);

	tcsetattr(0, TCSANOW, &newt);


	while (1) {
		input_pointer=0;
		printf("] ");

		while(1) {
			ch=getchar();

//			printf("VMW: %d\n",ch);

			if ((ch=='\n') || (ch=='\r')) {

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

#include <stddef.h>
#include <stdint.h>
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"

//#include <stdio.h>
//#include <string.h>
//#include <termios.h>

static int parse_input(char *string);

//static int debug=0;

#define VERSION "13.1"

static int print_help(void) {

	printf("VMWos Shell Version %s\n\n",VERSION);
	printf("\tblink on/off	- turns on/off heartbeat LED\n");
	printf("\tcd		- change directory\n");
	printf("\tcls		- clears the screen\n");
	printf("\tcolor X	- set text to color #X\n");
	printf("\techo X	- prints string X\n");
	printf("\tfont X	- sets the font to font #X\n");
	printf("\tgetpid	- print current process ID\n");
	printf("\tgradient	- make background look cool\n");
	printf("\thelp		- prints this help message\n");
	printf("\trandom	- print random number\n");
	printf("\treset		- reset the machine\n");
	printf("\tsleep	X	- sleep for X seconds\n");
	printf("\ttemp		- print the temperature\n");
	printf("\tuptime	- print seconds since boot\n");
	printf("\tver		- print version\n");
	printf("\n");

	return 0;
}

#define MAX_ARGUMENTS 16

static char *arguments[MAX_ARGUMENTS];

static int create_argv(char *string, int32_t *background) {

	int count=0;
	int ptr=0;
	int i;
	int argv_debug=0;

	*background=0;

	while(1) {
		arguments[count]=&(string[ptr]);
		if (argv_debug) {
			printf("shell:argv: %d %x %s\n",count,
				(long)(&string[ptr]),&string[ptr]);
		}

		/* check if starts with ampersand */
		/* if so, truncate argv and run in background */
		if (string[ptr]=='&') {
			if (argv_debug) {
				printf("Found &, running in background!\n");
			}
			arguments[count]=NULL;
			*background=1;
			break;
		}

		/* Break at first space */
		/* TODO: handle all whitespace? */
		while((string[ptr]!=' ')&&(string[ptr]!=0)) {
			ptr++;
		}
		count++;

		if (string[ptr]==0) {
			arguments[count]=NULL;
			break;
		}

		/* NUL terminate */
		string[ptr]=0;
		ptr++;


	}

	if (argv_debug) {
		printf("Found %d arguments\n",count);
		for(i=0;i<count;i++) {
			printf("%d: %s\n",i,arguments[i]);
		}
	}

	return count;
}

/* Check if a relative path */
/* i.e., it contains a / */
static int is_relative(char *string) {

	int result=0,i;

	for(i=0;i<strlen(string);i++) if (string[i]=='/') result++;

	return result;

}


static int parse_input(char *string) {

	int result=0;
	char temp_string[256];

	if (!strncmp(string,"echo",4)) {
		if (string[4]!=0) printf("%s\n",string+5);
	}
	else if (!strncmp(string,"cls",3)) {
		printf("\n\r\033[2J");
	}
	else if (!strncmp(string,"cd",2)) {
		if (strlen(string)>3) {
			result=chdir((const char *)&(string[3]));
		}
		else {
			result=chdir("/home");
		}
		if (result<0) {
			printf("Error changing directory %s\n",strerror(result));
		}
	}
	else if (!strncmp(string,"font ",5)) {
		vmwos_setfont(string[5]);
	}
	else if (!strncmp(string,"gradient",8)) {
		vmwos_gradient();
	}
	else if (!strncmp(string,"getpid",6)) {
		printf("Current pid: %d\n",getpid());
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
	else if (!strncmp(string,"color ",6)) {
		printf("%c[3%cm\n",27,string[6]);
	}
	else if (!strncmp(string,"ver",3)) {
		printf("VMWos Shell version %s\n",VERSION);
	}
	else if (!strncmp(string,"uptime",6)) {
		struct tms buf;
		char timestring[256];

		times(&buf);

		printf("Time since boot: %s\n",
			time_pretty(time(NULL),timestring,256));
		printf("Time running: %s\n",
			time_pretty((buf.tms_utime)/64,timestring,256));
	}
	else if (!strncmp(string,"reset",5)) {
		printf("Resetting...\n");
		reboot();
	}
	else if (!strncmp(string,"sleep",5)) {
		int seconds=0;
		seconds=atoi(string+6);
		printf("Sleeping for %ds...\n",seconds);
		sleep(seconds);
	}
	else if (!strncmp(string,"temp",4)) {
		int32_t temperature;
		temperature=vmwos_get_temp();
		if (temperature<-273000) {
			printf("Invalid temperature %d\n",temperature);
		}
		else {
			printf("Current temperature %dC, %dF\n",
				temperature/1000,
				((temperature*9)/5000)+32);
		}
	}
	else if (!strncmp(string,"random",6)) {
		printf("%d\n",rand());
	}
	else if (strlen(string)==0) {
		/* do nothing */
	}
	else {
		int32_t pid,status,result,background;
		struct stat stat_buf;

		/* Convert string to argv format */
		result=create_argv(string,&background);
		if (result<0) {
			printf("Too many command line arguments\n");
		}

		else {
			if (is_relative(string)) {
				sprintf(temp_string,"%s",string);
			}
			else {
				/* Look in /bin */
				sprintf(temp_string,"/bin/%s",string);
			}

			result=stat(temp_string,&stat_buf);
			if (result<0) {
				printf("\nCommmand not found: \"%s\"!\n",
								temp_string);
			}
			else {
				/* Fork a child */
				pid=vfork();
				if (pid==0) {
					execve(temp_string,arguments,NULL);
				}
				else {
					if (background) {
					}
					else {
						printf("Waiting for %d to finish\n",pid);
						waitpid(pid,&status,0);
						printf("Child exited with %d\n",status);
					}
				}
			}
		}
	}

	return result;
}

int main(int argc, char **argv) {

	char input_string[256];
	int input_pointer,result;
	int ch,done=0;
	int32_t status;

	struct termios oldt, newt;

//	register long sp asm ("sp");

//	printf("Our sp=%x\n",sp);

	tcgetattr( 0, &oldt);
	newt = oldt;

	cfmakeraw(&newt);

	tcsetattr(0, TCSANOW, &newt);

	while (1) {
		input_pointer=0;

		printf("] ");

		while(1) {

			ch=getchar();

			if ((ch=='\n') || (ch=='\r')) {
				printf("\n");
				input_string[input_pointer]=0;
				result=parse_input(input_string);
				if (result==1) done=1;
				break;
			}

			/* ctrl-D? */
			if (ch==4) {
				done=1;
				break;
			}

			/* Backspace */
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

		/* See if any background childen have finished */
		/* We should loop here until no more reported? */
		result=waitpid(-1,&status,WNOHANG);
		if (result>0) {
			printf("Child %d exited with %d\n",result,status);
		}

		if (done) break;
	}
	tcsetattr( 0, TCSANOW, &oldt);

	return 0;
}



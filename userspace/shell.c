#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/wait.h>
#endif

//#define FIRST_RUN	1

static int parse_input(char *string);

//static int debug=0;
static int argv_debug=0;

#define VERSION "13.2"

static struct termios oldt, newt;

static int print_help(void) {

	printf("VMWos Shell Version %s\n\n",VERSION);
	printf("\tblink on/off	- turns on/off heartbeat LED\n");
	printf("\tcd		- change directory\n");
	printf("\tclear		- clears the screen\n");
	printf("\tcolor X	- set text to color #X\n");
	printf("\techo X	- prints string X\n");
	printf("\tfont X	- sets the font to font #X\n");
	printf("\tgetpid	- print current process ID\n");
	printf("\tgradient	- make background look cool\n");
	printf("\thelp		- prints this help message\n");
	printf("\trandom	- print random number\n");
	printf("\treboot	- reboot the machine\n");
	printf("\tsleep	X	- sleep for X seconds\n");
	printf("\ttemp		- print the temperature\n");
	printf("\tuptime	- print seconds since boot\n");
	printf("\tver		- print version\n");
	printf("\n");

	return 0;
}

#define MAX_ARGUMENTS 16

static char *arguments[MAX_ARGUMENTS];

#define REDIRECT_OVERWRITE	0
#define REDIRECT_APPEND		1

static char *redirect_stdout;
static char *redirect_stdin;
static char *redirect_stderr;

static int redirect_stdout_type=REDIRECT_OVERWRITE;
static int redirect_stderr_type=REDIRECT_OVERWRITE;


/* Create argv arguments */
/* Also look for: */
/*    "&" (to run in background) */
/*    ">" or ">>" (to redirect stdout) */
/*    "2>" or "2>>" (to redirect stderr) */
/*    "<" (to redirect stdin) */


static char *get_next_arg(char *string,int *ptr) {

	char *result=NULL;

	/* strip off leading whitespace */
	while(1) {
		if (string[*ptr]==0) {
			return NULL;
		}
		if (string[*ptr]!=' ') break;

		(*ptr)++;
	}

	if (string[*ptr]==0) {
		result=NULL;
	}
	else {
		result=&string[*ptr];
	}
	return result;
}

/* argv is a pointer to a list of arguments */
/* argv[0] should be the executable name */
/* there should be a NULL pointer last on the list */

static int create_argv(char *string, int32_t *background) {

	int argc=0;
	int ptr=0;
	int i;
	int done_with_normal_args=0;
	char *next_arg;

	*background=0;
	redirect_stdout=NULL;
	redirect_stdin=NULL;
	redirect_stderr=NULL;

	while(1) {

		/* get next argument */
		next_arg=get_next_arg(string,&ptr);

		/* If no more, then we are done */
		if (next_arg==NULL) {
			break;
		}

		if (argv_debug) {
			printf("shell:argv: %d %p %s\n",argc,
				&string[ptr],&string[ptr]);
		}

		/* check if starts with ampersand */
		/* if so, truncate argv and run in background */
		if (string[ptr]=='&') {
			if (argv_debug) {
				printf("Found &, running in background!\n");
			}
			arguments[argc]=NULL;
			*background=1;
			break;
		}

		/* check if starts with > */
		/* if so, done with normal args and next is destination */
		if (string[ptr]=='>') {
			if (argv_debug) {
				printf("Found >, redirecting stdout!\n");
			}
			ptr++;

			if (string[ptr]=='>') {
				redirect_stdout_type=REDIRECT_APPEND;
				ptr++;
			}
			else {
				redirect_stdout_type=REDIRECT_OVERWRITE;
			}

			redirect_stdout=get_next_arg(string,&ptr);
			arguments[argc]=NULL;
			done_with_normal_args=1;
		}

		/* check if starts with 2> */
		/* if so, done with normal args and next is stderr */
		if (string[ptr]=='2') {
			if (string[ptr+1]=='>') {
				if (argv_debug) {
					printf("Found 2>, redirecting stderr!\n");
				}
				ptr+=2;

				if (string[ptr]=='>') {
					redirect_stderr_type=REDIRECT_APPEND;
					ptr++;
				}
				else {
					redirect_stderr_type=REDIRECT_OVERWRITE;
				}

				redirect_stderr=get_next_arg(string,&ptr);
				arguments[argc]=NULL;
				done_with_normal_args=1;
			}
		}

		/* check if starts with < */
		/* if so, done with normal args and next is destination */
		if (string[ptr]=='<') {
			if (argv_debug) {
				printf("Found <, redirecting stdin!\n");
			}
			ptr++;
			redirect_stdin=get_next_arg(string,&ptr);
			arguments[argc]=NULL;
			done_with_normal_args=1;
		}



		/* Break at first space */
		/* TODO: handle all whitespace? */
		while((string[ptr]!=' ')&&(string[ptr]!=0)) {
			ptr++;
		}

		/* NUL terminate */
		string[ptr]=0;
		ptr++;

		if (!done_with_normal_args) {
			/* Point to argument */
			arguments[argc]=next_arg;
			argc++;
		}

	}

	/* make last argument NULL */
	arguments[argc]=NULL;

	if (argv_debug) {
		printf("Found %d arguments\n",argc);
		for(i=0;i<argc;i++) {
			printf("%d: %s\n",i,arguments[i]);
		}
		if (redirect_stdout) printf("output: %s\n",redirect_stdout);
		if (redirect_stdin) printf("input: %s\n",redirect_stdin);
		if (redirect_stderr) printf("error: %s\n",redirect_stderr);
	}

	return argc;
}

/* Check if a relative path */
/* i.e., it contains a / */
static int is_relative(char *string) {

	int result=0,i;

	for(i=0;i<strlen(string);i++) if (string[i]=='/') result++;

	return result;

}

int32_t run_executable(char *string) {

	int32_t pid,status,result,background;
	struct stat stat_buf;
	char temp_string[256];
	int32_t input_fd,output_fd,error_fd;

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
			printf("\nCommmand not found: \"%s\"!\n",temp_string);
			return result;
		}

		if (redirect_stdin) {
			input_fd=open(redirect_stdin,O_RDONLY);
			if (input_fd<0) {
				printf("Can't open input source %s: %s\n",
					redirect_stdin,strerror(errno));
				return input_fd;
			}
		}

		if (redirect_stdout) {
			if (redirect_stdout_type==REDIRECT_APPEND) {
				output_fd=open(redirect_stdout,
						O_WRONLY|O_APPEND);
			}
			else {
				output_fd=open(redirect_stdout,
						O_CREAT|O_TRUNC|O_WRONLY,0666);
			}
			if (output_fd<0) {
				printf("Can't open output target %s: %s\n",
					redirect_stdout,strerror(errno));
				return output_fd;
			}
		}

		if (redirect_stderr) {
			if (redirect_stderr_type==REDIRECT_APPEND) {
				error_fd=open(redirect_stderr,
						O_APPEND|O_WRONLY);
			}
			else {
				error_fd=open(redirect_stderr,
						O_CREAT|O_TRUNC|O_WRONLY,0666);
			}
			if (error_fd<0) {
				printf("Can't open error target %s: %s\n",
					redirect_stderr,strerror(errno));
				return error_fd;
			}
		}

		/* Fork a child */
		pid=vfork();
		if (pid==0) {
			if (redirect_stdin) {
				dup2(input_fd,STDIN_FILENO);
			}
			if (redirect_stdout) {
				dup2(output_fd,STDOUT_FILENO);
			}
			if (redirect_stderr) {
				dup2(error_fd,STDERR_FILENO);
			}
			execve(temp_string,arguments,NULL);
		}
		else {
			if (background) {
			}
			else {
				//printf("Waiting for %d to finish\n",pid);
				waitpid(pid,&status,0);
				//printf("Child exited with %d\n",status);
			}
		}
		if (redirect_stdout) {
			close(output_fd);
		}
		if (redirect_stdin) {
			close(input_fd);
		}
		if (redirect_stderr) {
			close(error_fd);
		}
	}

	return result;
}




static void clear_screen(void) {
	/* clear */
	printf("\n\r\033[2J");
	/* move cursor to top */
	printf("\033[H");
}

static int parse_input(char *string) {

	int result=0;

	if (!strncmp(string,"echo",4)) {
		if (string[4]!=0) printf("%s\n",string+5);
	}
	else if (!strncmp(string,"clear",5)) {
		clear_screen();
	}
	else if (!strncmp(string,"cd",2)) {
		if (strlen(string)>3) {
			result=chdir((const char *)&(string[3]));
		}
		else {
			result=chdir("/home");
		}
		if (result<0) {
			printf("Error changing directory %s\n",
							strerror(result));
		}
	}
#ifdef VMWOS
	else if (!strncmp(string,"font ",5)) {
		vmwos_setfont(string[5]);
	}
	else if (!strncmp(string,"gradient",8)) {
		vmwos_gradient(0);
	}
	else if (!strncmp(string,"blink ",6)) {
		vmwos_blink(string[6]);
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
	else if (!strncmp(string,"reboot",6)) {
		printf("Rebooting...\n");
		reboot();
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
#endif
	else if (!strncmp(string,"getpid",6)) {
		printf("Current pid: %d\n",getpid());
	}
	else if (!strncmp(string,"exit",4)) {
		result=1;
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
	else if (!strncmp(string,"sleep",5)) {
		int seconds=0;
		seconds=atoi(string+6);
		printf("Sleeping for %ds...\n",seconds);
		sleep(seconds);
	}
	else if (!strncmp(string,"random",6)) {
		printf("%d\n",rand());
	}
	else if (!strncmp(string,"remainder",9)) {
		int j;
		for(j=0;j<100;j++) {
			printf("%d q=%d r=%d\n",j,j/13,j%13);
		}
	}
	else if (strlen(string)==0) {
		/* do nothing */
	}
	else {
		/* set to default term settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

		result=run_executable(string);

		/* set back to our terminal settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	}

	return result;
}


static void draw_prompt(char *input_string) {

	printf("] ");
	if (strlen(input_string)>0) printf("%s",input_string);

}

int main(int argc, char **argv) {

	char input_string[256];
	int input_pointer,result;
	int ch,done=0;
	int32_t status;

#ifdef FIRST_RUN
	int firstrun_done=0;
#endif


//	register long sp asm ("sp");
//	printf("Our sp=%x\n",sp);

	tcgetattr( STDIN_FILENO, &oldt);
	newt = oldt;

	/* Turn off echo, turn off cannonical */
	newt.c_lflag &= ~(ECHO | ICANON);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	while (1) {


		input_pointer=0;

		draw_prompt(input_string);

		while(1) {


#ifdef FIRST_RUN
			if (!firstrun_done) {
				printf("Running DEMOSPLASH\n");
				strncpy(input_string,"demosplash2019",15);
				result=parse_input(input_string);
				firstrun_done=1;
				ch='\n';
			}

#endif
			ch=getchar();


			if ((ch=='\n') || (ch=='\r')) {
				printf("\n");
				result=parse_input(input_string);
				memset(input_string,0,sizeof(input_string));
				if (result==1) done=1;
				break;
			}

			/* ctrl-D? */
			if (ch==4) {
				done=1;
				break;
			}

			/* ctrl-L (form feed) */
			if (ch==12) {
				clear_screen();
				draw_prompt(input_string);
			} else
			/* Backspace */
			if ((ch==0x7f) || (ch=='\b')) {

				if (input_pointer>0) {
					input_pointer--;
					input_string[input_pointer]=0;
					printf("\b \b");
				}
			}
			else {
				input_string[input_pointer]=ch;
				input_pointer++;
				input_string[input_pointer]=0;
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

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return 0;
}

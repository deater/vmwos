#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"

int putchar(int c) {

	return write(1,&c,1);
}

int getchar(void) {

	int c=0;

	read(0,&c,1);

	return c;

}


unsigned int sleep(unsigned int seconds) {

	struct timespec t;

	t.tv_sec=seconds;
	t.tv_nsec=0;

	nanosleep(&t, NULL);

	return 0;
}




#define MAX_PRINT_SIZE 256

int printf(char *string,...) {

	va_list ap;

	char buffer[MAX_PRINT_SIZE];
	char int_buffer[10];
	int int_pointer=0;

	int buffer_pointer=0;
	int i;
	unsigned long x;

	va_start(ap, string);

	while(1) {
		if (*string==0) break;

		if (*string=='%') {
			string++;
			if (*string=='d') {
				string++;
				x=va_arg(ap, unsigned long);
				if (x&0x80000000) {
					x=~x;
					x+=1;
					buffer[buffer_pointer]='-';
					buffer_pointer++;
				}
				int_pointer=9;
				do {
					int_buffer[int_pointer]=(x%10)+'0';
					int_pointer--;
					x/=10;
				} while(x!=0);
				for(i=int_pointer+1;i<10;i++) {
					buffer[buffer_pointer]=int_buffer[i];
					buffer_pointer++;
				}

			}
			else if (*string=='x') {
				string++;
				x=va_arg(ap, unsigned long);
				int_pointer=9;
				do {
					if ((x%16)<10) {
						int_buffer[int_pointer]=(x%16)+'0';
					}
					else {
						int_buffer[int_pointer]=(x%16)-10+'a';
					}
					int_pointer--;
					x/=16;
				} while(x!=0);
				for(i=int_pointer+1;i<10;i++) {
					buffer[buffer_pointer]=int_buffer[i];
					buffer_pointer++;
				}
			}
			else if (*string=='c') {
				string++;
				x=va_arg(ap, unsigned long);
				buffer[buffer_pointer]=x;
				buffer_pointer++;
			}
			else if (*string=='s') {
				char *s;
				string++;
				s=(char *)va_arg(ap, long);
				while(*s) {
					buffer[buffer_pointer]=*s;
					s++;
					buffer_pointer++;
				}
			}
		}
		else {
			buffer[buffer_pointer]=*string;
			buffer_pointer++;
			string++;
		}
		if (buffer_pointer==MAX_PRINT_SIZE-1) break;
	}

	va_end(ap);

	write(1,buffer,buffer_pointer);

	return buffer_pointer;
}


int strncmp(const char *s1, const char *s2, uint32_t n) {

	int i=0,r;

	while(1) {

		if (i==n) return 0;

		r=s1[i]-s2[i];
		if (r!=0) return r;

		i++;
	}

	return 0;
}

void cfmakeraw(struct termios *termios_p) {

	termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP |INLCR|IGNCR|ICRNL|IXON);
	termios_p->c_oflag &= ~OPOST;
	termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	termios_p->c_cflag &= ~(CSIZE|PARENB);
	termios_p->c_cflag |= CS8;
}

int tcgetattr(int fd, struct termios *termios_p) {

	return ioctl3(fd,TCGETS,(long)termios_p);

}

int tcsetattr(int fd, int optional_actions,
                     const struct termios *termios_p) {

        return ioctl3(fd,TCSETS,(long)termios_p);
}

int time(int *t) {

	int our_time;

	our_time=sys_time();

	if (t!=NULL) *t=our_time;

	return our_time;
}

int reboot(void) {
	return sys_reboot();
}

/* ARMV6 has no division instruction	*/
/* calculate  q=(dividend/divisor)	*/
/* Not necessarily meant to be speedy 	*/
uint32_t __aeabi_uidiv(uint32_t dividend, uint32_t divisor) {

	uint32_t q;
	uint32_t new_d;

	if (divisor==0) {
		printf("Division by zero!\n");
		return 0;
	}

	q=0;
	new_d=divisor;
	while(1) {
		if (new_d>dividend) break;

		q++;
		new_d+=divisor;
	}

	return q;
}

static char strerror_string[]="Error!";
int errno=0;

char *strerror(int errnum) {
	return strerror_string;
}

int32_t rand(void) {

	uint32_t buffer;
	int32_t result;

	result=vmwos_random(&buffer);

	if (result<4) {
		printf("Error!\n");
		return -1;
	}

	return buffer;
}

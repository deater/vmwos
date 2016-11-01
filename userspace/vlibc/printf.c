#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"

#define MAX_PRINT_SIZE 256

int vsprintf(char *buffer, char *string, va_list ap) {

	char int_buffer[10];
	int int_pointer=0;

	int buffer_pointer=0;
	int i;
	unsigned long x;

	while(1) {
		if (*string==0) break;

		if (*string=='%') {
			string++;
			if ((*string=='d') || (*string=='i')) {
				string++;
				x=va_arg(ap, unsigned long);
				if (x&0x80000000) {
					buffer[buffer_pointer]='-';
					buffer_pointer++;
					if (buffer_pointer==MAX_PRINT_SIZE) break;

					x=~x;
					x++;
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
					if (buffer_pointer==MAX_PRINT_SIZE) break;
				}

			}
			else if ((*string=='x') || (*string=='p')) {
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
					if (buffer_pointer==MAX_PRINT_SIZE) break;
				}
			}
			else if (*string=='c') {
				string++;
				x=va_arg(ap, unsigned long);
				buffer[buffer_pointer]=x;
				buffer_pointer++;
				if (buffer_pointer==MAX_PRINT_SIZE) break;
			}
			else if (*string=='s') {
				char *s;
				string++;
				s=(char *)va_arg(ap, long);
				while(*s) {
					buffer[buffer_pointer]=*s;
					s++;
					buffer_pointer++;
					if (buffer_pointer==MAX_PRINT_SIZE) break;
				}
			}
		}
		else {
			buffer[buffer_pointer]=*string;
			buffer_pointer++;
			if (buffer_pointer==MAX_PRINT_SIZE) break;
			string++;
		}
		if (buffer_pointer==MAX_PRINT_SIZE-1) break;
	}

	return buffer_pointer;
}

int printf(char *string,...) {

	char buffer[MAX_PRINT_SIZE];
	int result;

	va_list argp;
	va_start(argp, string);

	result=vsprintf(buffer,string,argp);

	va_end(argp);

	write(1,buffer,result);

        return result;

}

int sprintf(char *string, char *fmt, ...) {

	int result;

	va_list argp;
	va_start(argp, fmt);

	result=vsprintf(string,fmt,argp);

	va_end(argp);

	/* NUL terminate */
	string[result]='\0';

	return result;

}


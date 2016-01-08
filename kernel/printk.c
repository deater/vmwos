#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "console_io.h"

#define MAX_PRINT_SIZE 256

int printk(char *string,...) {

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
					if (buffer_pointer==MAX_PRINT_SIZE) break;
				}
			}
			else if (*string=='c') {
				string++;
				x=va_arg(ap, int);
				buffer[buffer_pointer]=x;
				buffer_pointer++;
				if (buffer_pointer==MAX_PRINT_SIZE) break;
			}
			else if (*string=='s') {
				char *s;
				string++;
				s=(char *)va_arg(ap, int);
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
		if (buffer_pointer==MAX_PRINT_SIZE) break;
	}

	va_end(ap);

	console_write(buffer,buffer_pointer);

	return buffer_pointer;
}

#if 0

int main(int argc, char **argv) {


	printk("Hello %d %x World!\n",4321,0xdec25);

	return 0;
}

#endif

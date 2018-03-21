#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"
#include "vlibc.h"

int putchar(int c) {

	return write(1,&c,1);
}

int puts(char *s) {
	int32_t len;
	unsigned char lf='\n';

	len=strlen(s);
	write(1,s,len);
	write(1,&lf,1);

	return 1;
}

int getchar(void) {

	int c=0;

	read(0,&c,1);

	return c;

}

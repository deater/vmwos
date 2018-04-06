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

/* FIXME */
#define MAXFILES	16
static FILE open_files[MAXFILES];

FILE stdin = {	.fd = 0,	};
FILE stdout = {	.fd = 1,	};
FILE stderr = { .fd = 2,	};


FILE *fopen(const char *pathname, const char *mode) {

	int fd;

	fd=open(pathname,O_RDONLY,0);
	if (fd<0) return NULL;

	open_files[0].fd=fd;

	return &open_files[0];

}

char *fgets(char *s, int size, FILE *stream) {

	char ch;
	int32_t result,count=0;

	while(1) {
		result=read(stream->fd,&ch,1);
		if (result<1) break;

		if (count>=size) break;

		s[count]=ch;

		if (ch=='\n') break;
		count++;
	}
	if (count==0) return NULL;
	s[count+1]=0;

	return s;
}

int fclose(FILE *stream) {

	return close(stream->fd);

}

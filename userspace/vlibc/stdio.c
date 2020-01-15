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

static FILE stdin_actual  = { .fd = 0, .eof=0	};
static FILE stdout_actual = { .fd = 1, .eof=0	};
static FILE stderr_actual = { .fd = 2, .eof=0	};

FILE *stdin=&stdin_actual;
FILE *stdout=&stdout_actual;
FILE *stderr=&stderr_actual;

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

int32_t fgetc(FILE *stream) {

	unsigned char ch;
	int result;

	result=read(stream->fd,&ch,1);

	/* FIXME: might not be eof */
	if (result==0) {
		stream->eof=1;
		return -1;
	}

	if (result<0) {
		return -1;
	}
	return ch;
}

int32_t feof(FILE *stream) {

	return stream->eof;

}


int32_t getdelim(char **buf, size_t *n, int delim, FILE *stream) {

	int c;
	char *ptr, *endptr;
	int32_t diff;
	char *newbuf;
	size_t newbufsize;

	/* If nothing there yet, we need to allocate space */
	if ((*buf == NULL) || (*n == 0)) {
		*n = BUFSIZ;
		*buf=malloc(*n);
		if (*buf==NULL) return -1;
	}

	ptr=*buf;
	endptr=*buf+*n;

	while(1) {
		/* get next char */
		c = fgetc(stream);

		/* handle end-of-file */
		if (c == -1) {
			if (feof(stream)) {
				/* NUL terminate if we can */
				diff = (ptr - *buf);
				if (diff != 0) {
					*ptr = '\0';
					return diff;
				}
			}
			return -1;
		}

		/* set the value */
		*ptr = c;
		ptr++;

		/* If we hit delimeter, we're at the end */
		if (c == delim) {
			/* NUL terminate */
			*ptr = '\0';
			return ptr - *buf;
		}

		/* see if we are too big for buffer */
		/* if so, grow by doubling size */
		if (ptr + 2 >= endptr) {
			newbufsize = *n * 2;
			diff = ptr - *buf;
			/* Try to allocate more space */
			newbuf = realloc(*buf, newbufsize);
			if (newbuf == NULL) {
				return -1;
			}
			*buf = newbuf;
			*n = newbufsize;
			endptr = newbuf + newbufsize;
			ptr = newbuf + diff;
		}
	}
}

/* Get line of text from FILE */
/* lineptr is malloc()ed or realloc()ed as necessary and should be free()d */
int32_t getline(char **buf, size_t *n, FILE *stream) {
	return getdelim(buf, n, '\n', stream);
}


#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"


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

static char strerror_string[]="Error!";
int errno=0;

char *strerror(int errnum) {
	return strerror_string;
}

int strlen(const char *s) {

	int length=0;

	while(s[length]) length++;

	return length;
}

int32_t atoi(char *string) {

	int result=0;
	char *ptr;

	ptr=string;

	while(*ptr!=0) {
		result*=10;
		result+=(*ptr)-'0';
		ptr++;
	}

	return result;
}


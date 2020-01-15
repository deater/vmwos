#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "lib/string.h"

int strncmp(const char *s1, const char *s2, uint32_t n) {

	int i=0,r;

	while(n>0) {

		r=s1[i]-s2[i];
		if (r!=0) return r;
		else if (s1[i]==0) return 0;

		i++;
		n--;
	}

	return 0;
}

int memcmp(const char *s1, const char *s2, uint32_t n) {

	int i=0,r;

	while(1) {

		if (i==n) return 0;

		r=s1[i]-s2[i];
		if (r!=0) return r;

		i++;
	}

	return 0;
}

/* At most n bytes of src are copied to dest */
/* If no nul in the first n bytes of src, dest will *not* be nul terminated */
/* If length of src less than n, nuls are padded */
char *strncpy(char *dest, const char *src, uint32_t n) {

	uint32_t i;

	for(i=0; i<n; i++) {
		dest[i]=src[i];
		if (src[i]=='\0') break;
	}
	for(i=i;i<n;i++) {
		dest[i]='\0';
	}

	return dest;

}

int32_t strlcpy(char *dest, const char *src, uint32_t n) {

	uint32_t i;

	for(i=0; i<n-1; i++) {
		dest[i]=src[i];
		if (src[i]=='\0') break;
	}
	dest[i]='\0';

	return i;
}

char *strncat(char *dest, const char *src, uint32_t n) {

	uint32_t i,dest_len;

	dest_len=strlen(dest);

	for(i=0; i<n; i++) {
		dest[dest_len+i]=src[i];
		if (src[i]=='\0') break;
	}
	dest[dest_len+i]='\0';

	return dest;

}

int strlen(const char *s1) {

	int i=0;

	while(s1[i]!=0) i++;

	return i;

}

/* FIXME: not optimized */
void *memmove(void *dest, const void *src, uint32_t n) {

	int i;
	char *d = dest;
	const char *s = src;

	/* If dest and src same, just return destination */
	if (d==s) {
		return d;
	}

	/* If no overlap, just run memcpy */
	if ((uintptr_t)s-(uintptr_t)d-n <= -2*n) {
		return memcpy(d, s, n);
	}

	/* if desitnation less than src, run forward */
	/* otherwise, copy backwards */
	if (d<s) {
		for(i=0;i<n;i++) {
			*d++ = *s++;
		}
	} else {
		for(i=n-1;i>=0;i--) {
			d[i] = s[i];
		}
	}

	return dest;
}

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>


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

void *memcpy(void *dest, const void *src, size_t n) {

        int i;

        char *d=dest;
        const char *s=src;

        for(i=0;i<n;i++) {
                *d=*s;
                d++; s++;
        }

        return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {

	size_t i;

	for(i=0; i<n; i++) {
		dest[i]=src[i];
		if (src[i]=='\0') break;
	}

	return dest;

}

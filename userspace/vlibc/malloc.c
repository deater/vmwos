#include <stdint.h>
#include <stddef.h>

#include "string.h"

void *malloc(uint32_t size) {
	return NULL;
}

void free(void *ptr) {
	return;
}

void *calloc(uint32_t nmemb, uint32_t size) {
	return NULL;
}

void *realloc(void *ptr, uint32_t size) {
	return NULL;
}

char *strdup(const char *s) {

	int64_t string_size;
	char *new_string=NULL;

	string_size=strlen(s);

	new_string=malloc(string_size+1);
	if (new_string!=NULL) {
		memcpy(new_string,s,string_size);
		new_string[string_size]='\0';
	}

	return new_string;
}

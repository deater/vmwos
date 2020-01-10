#include <stdint.h>

/* Detects if it's a control character */
/* This seems to be 0-31, as well as delete (127) */
int32_t iscntrl(uint32_t c) {

	if ((c<32)||(c==127)) return 1;
	return 0;
}

/* Detects if it's a base10 digit */
int32_t isdigit(uint32_t c) {

	if ((c>='0')&&(c<='9')) return 1;
	return 0;
}


/* Detects if it's a space or not */
int32_t isspace(int32_t c) {

	if (	(c==' ') ||	/* space */
		(c=='\f') ||	/* formfeed */
		(c=='\n') ||	/* newline */
		(c=='\r') ||	/* carriage return */
		(c=='\t') ||	/* horizontal tab */
		(c=='\v')) {	/* vertical tab */

		return 1;
	}
	return 0;
}

#include <stdint.h>

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

#include <stddef.h>
#include <stdint.h>

#include "../version.h"
#include "syscalls/uname.h"
#include "lib/string.h"

#include "../date.h"

int32_t uname(struct utsname *buf) {

	strlcpy(buf->sysname,"vmwOS",UTSNAME_SIZE);
	strlcpy(buf->nodename,"pi",UTSNAME_SIZE);
	strlcpy(buf->release,VERSION,UTSNAME_SIZE);
	strlcpy(buf->version,COMPILE_DATE,UTSNAME_SIZE);
#ifdef ARMV7
	strlcpy(buf->machine,"armv7",UTSNAME_SIZE);
#else
	strlcpy(buf->machine,"armv6",UTSNAME_SIZE);
#endif
	strlcpy(buf->domainname,"none",UTSNAME_SIZE);

	return 0;

}

#include <stddef.h>
#include <stdint.h>

#include "../version.h"
#include "syscalls/uname.h"
#include "lib/string.h"

#include "date.h"

int32_t uname(struct utsname *buf) {

	strlcpy(buf->sysname,"vmwOS",UTSNAME_SIZE);
	strlcpy(buf->nodename,"pi",UTSNAME_SIZE);
	strlcpy(buf->release,VERSION,UTSNAME_SIZE);
	strlcpy(buf->version,COMPILE_DATE,UTSNAME_SIZE);
	strlcpy(buf->machine,"arm6",UTSNAME_SIZE);
	strlcpy(buf->domainname,"none",UTSNAME_SIZE);

	return 0;

}



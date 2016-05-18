#include <stddef.h>
#include <stdint.h>

#include "../version.h"
#include "syscalls/uname.h"
#include "lib/string.h"

#include "date.h"

int32_t uname(struct utsname *buf) {

	strncpy(buf->sysname,"vmwOS",5);
	strncpy(buf->nodename,"pi",2);
	strncpy(buf->release,VERSION,strlen(VERSION));
	strncpy(buf->version,COMPILE_DATE,65);
	strncpy(buf->machine,"arm6",3);
	strncpy(buf->domainname,"none",4);

	return 0;

}



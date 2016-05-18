#define UTSNAME_SIZE	65

struct utsname {
	char sysname[UTSNAME_SIZE];
	char nodename[UTSNAME_SIZE];
	char release[UTSNAME_SIZE];
	char version[UTSNAME_SIZE];
	char machine[UTSNAME_SIZE];
	char domainname[UTSNAME_SIZE];
};

int32_t uname(struct utsname *buf);

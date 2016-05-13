#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>

#define linux_dirent vmwos_dirent

struct linux_dirent {
        long            d_ino;
        off_t           d_off;
        unsigned short  d_reclen;
        char            d_name[];
};

int getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count) {
	return syscall(SYS_getdents, fd, dirp, count);
}




#endif

#define BUF_SIZE 1024

int print_date(long int s) {

	int year,month,day,hour,min,seconds;
	int leap_year;
	int seconds_in_year,days_in_month,seconds_in_month,seconds_in_day;
	int seconds_in_hour,seconds_in_minute;
	int month_days[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	char month_names[12][4]={"Jan","Feb","Mar","Apr",
				"May","Jun","Jul","Aug",
				"Sep","Oct","Nov","Dec"};

	/* Calculate Year */
	year=1970;
	while(1) {
		leap_year = ((year % 4 == 0) &&
				(year % 100 != 0 || year % 400 == 0));

		seconds_in_year=(365+leap_year)*24*60*60;

		if (s<seconds_in_year) break;

		s-=seconds_in_year;
		year++;
	}

	/* Calculate Month */
	month=0;
	leap_year = ((year % 4 == 0) &&
			(year % 100 != 0 || year % 400 == 0));

	while(1) {

		days_in_month=month_days[month];
		if ((month==1) && (leap_year)) days_in_month++;
		seconds_in_month=days_in_month*(24*60*60);

		if (s<seconds_in_month) break;

		s-=seconds_in_month;
		month++;
	}

	day=0;
	seconds_in_day=24*60*60;

	while(1) {
		if (s<seconds_in_day) break;
		s-=seconds_in_day;
		day++;
	}
	hour=0;
	seconds_in_hour=60*60;
	while(1) {
		if (s<seconds_in_hour) break;
		s-=seconds_in_hour;
		hour++;
	}

	min=0;
	seconds_in_minute=60;
	while(1) {
		if (s<seconds_in_minute) break;
		s-=seconds_in_minute;
		min++;
	}

	seconds=s;

	printf("%d %s %d %d:%d:%d",
		day+1,month_names[month],year,hour,min,seconds);

	return 0;
}

int print_permissions(int mode) {

	int temp,i,type;

	type=mode>>12;
	switch(type) {
		case 1:	printf("f"); break;
		case 2:	printf("c"); break;
		case 4: printf("d"); break;
		case 6: printf("b"); break;
		case 8: printf("-"); break;
		case 10:printf("l"); break;
		case 12:printf("s"); break;
		default:
			printf("?");
	}

	temp=mode&0x1ff;

	for(i=6;i>=0;i-=3) {
		if (temp&(0x4<<i)) printf("r"); else printf("-");
		if (temp&(0x2<<i)) printf("w"); else printf("-");
		if (temp&(0x1<<i)) printf("x"); else printf("-");

	}

	return 0;
}

static void list_file(char *name) {

	struct stat stat_buf;

	stat(name,&stat_buf);
	print_permissions(stat_buf.st_mode);
	printf(" %d %d %d %d ",
		stat_buf.st_nlink,
		stat_buf.st_uid,
		stat_buf.st_gid,
		stat_buf.st_size);
	print_date(stat_buf.st_mtime);
	printf(" %s\n",name);
}

int ls(char *path) {

	int fd,result;
	char buf[BUF_SIZE];
	int nread;
	int offset;
	struct vmwos_dirent *d;
	struct stat stat_buf;

	result=stat(path,&stat_buf);
	if (result<0) {
		printf("Cannot access %s: no such file or directory!\n",path);
		return -1;
	}

//	printf("Mode: %x\n",stat_buf.st_mode);

	/* handle if it's not a directory */
	if ( (stat_buf.st_mode&S_IFMT)!=S_IFDIR) {
		list_file(path);
		return 0;
	}

	/* handle if it's a directory */
	fd=open(path,O_RDONLY,0);
	if (fd<0) {
		printf("Error! %s\n",strerror(errno));
	}

	while(1) {
		nread = getdents (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
			printf("Error! %s\n",strerror(errno));
			break;
		}
		if (nread==0) break;

		offset=0;
		while(offset<nread) {
			d=(struct vmwos_dirent *)(buf+offset);
//			printf("Inode: %ld\n",d->d_ino);
			list_file(d->d_name);
			offset+=d->d_reclen;
		}
	}


	close(fd);


	return 0;
}

static int debug=0;

int main(int argc, char **argv) {

	int i;

	if (debug) {
		printf("ls: argc=%d\n",argc);
		for(i=0;i<argc;i++) {
			printf("\targ%d = %s\n",i,argv[i]);
		}
	}

	if (argc<2) {
		if (debug) printf("Listing current dir\n");
		ls(".");
	}
	else {
		if (debug) printf("Listing %s\n",argv[1]);
		ls(argv[1]);
	}
	if (debug) printf("Ready to return\n");

	return 0;
}

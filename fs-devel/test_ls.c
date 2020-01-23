#include <stddef.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>

#if 1
/* hack */
static int errno;

#include "test_glue.h"

static int get_errno(int value) { return value; }

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

static int get_errno(int value) { return errno; }

#endif

#define LS_EXE_COLOR	"\033[1;32m"	/* bright green */
#define LS_DIR_COLOR	"\033[1;34m"	/* bright blue */
#define LS_NORMAL_COLOR	"\033[0m"	/* normal */



#define BUF_SIZE 1024

#define MAX_PATH 256

static int debug=0;

static int print_date(long int s) {

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

	printf("%2d %s %d %02d:%02d:%02d",
		day+1,month_names[month],year,hour,min,seconds);

	return 0;
}

static int print_permissions(int mode) {

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

static void print_file_color(int mode, char *name) {

	/* FIXME: check isatty() before printing color */
	if ((mode&S_IFMT)==S_IFDIR) {
		printf("%s%s%s",LS_DIR_COLOR,name,LS_NORMAL_COLOR);
	}
	else if (mode & 0x1) {
		printf("%s%s%s",LS_EXE_COLOR,name,LS_NORMAL_COLOR);
	}
	else {
		printf("%s",name);
	}
}

static unsigned int ilog10(unsigned int value) {

	return	(value >= 1000000000) ? 9 :
		(value >= 100000000) ? 8 :
		(value >= 10000000) ? 7 :
		(value >= 1000000) ? 6 :
		(value >= 100000) ? 5 :
		(value >= 10000) ? 4 :
		(value >= 1000) ? 3 :
		(value >= 100) ? 2 :
		(value >= 10) ? 1 :
		0;
}

static void list_file_long(char *name, char *path, int maxsize) {

	struct vmwos_stat stat_buf;
	int padding,i;
	char full_path[MAX_PATH];

	if (path==NULL) {
		snprintf(full_path,MAX_PATH,"%s",name);
	}
	else {
		snprintf(full_path,MAX_PATH,"%s/%s",path,name);
	}

	if (debug) {
		printf("Running stat on %s\n",full_path);
	}

	stat_syscall(full_path,&stat_buf);

	if (debug) {
		printf("\tpermissions=%x\n",stat_buf.st_mode);
	}

	print_permissions(stat_buf.st_mode);
	printf(" %2d %d %d ",
		stat_buf.st_nlink,
		stat_buf.st_uid,
		stat_buf.st_gid);

	padding=ilog10(maxsize)-ilog10(stat_buf.st_size);
	for(i=0;i<padding;i++) printf(" ");

	printf("%lld ",stat_buf.st_size);

	print_date(stat_buf.st_mtime);

	printf(" ");

	print_file_color(stat_buf.st_mode,name);
	printf("\n");

}

static int ls_long(char *path) {

	int fd,result;
	char buf[BUF_SIZE];
	char full_path[MAX_PATH];
	int nread;
	int offset;
	struct vmwos_dirent *d;
	struct vmwos_stat stat_buf;
	int max_len=0;

	printf("ls long\n");

	result=stat_syscall(path,&stat_buf);
	if (result<0) {
		printf("Cannot access %s: no such file or directory!\n",path);
		return -1;
	}

//	printf("Mode: %x\n",stat_buf.st_mode);

	/* handle if it's not a directory */
	if ( (stat_buf.st_mode&S_IFMT)!=S_IFDIR) {
		list_file_long(path,NULL,stat_buf.st_size);
		return 0;
	}

	/* handle if it's a directory */
	fd=open_syscall(path,O_RDONLY,0);
	if (fd<0) {
		errno=get_errno(fd);
		printf("Error opening dir %s! %s\n",path,strerror(errno));
	}

	/* First, find maxsize */
	while(1) {
		nread = getdents_syscall (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
			errno=get_errno(nread);
			printf("Error getdents! %s\n",strerror(errno));
			break;
		}
		if (nread==0) break;

		offset=0;
		while(offset<nread) {
			d=(struct vmwos_dirent *)(buf+offset);
			snprintf(full_path,MAX_PATH,"%s/%s",path,d->d_name);
			stat_syscall(full_path,&stat_buf);
			if (stat_buf.st_size>max_len) max_len=stat_buf.st_size;

			offset+=d->d_reclen;
		}
	}

//	FIXME: implement lseek
//	lseek(fd,0,SEEK_SET);

	close_syscall(fd);
	fd=open_syscall(path,O_RDONLY,0);


	/* Then actually print */
	while(1) {
		nread = getdents_syscall (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
			errno=get_errno(nread);
			printf("Error getdents again fd=%d! %s\n",
				fd,strerror(errno));
			break;
		}
		if (nread==0) break;

		offset=0;
		while(offset<nread) {
			d=(struct vmwos_dirent *)(buf+offset);
//			printf("Inode: %ld\n",d->d_ino);
			list_file_long(d->d_name,path,max_len);
			offset+=d->d_reclen;
		}
	}


	close_syscall(fd);


	return 0;
}

static int ls_plain(char *path) {

	int fd,result;
	char buf[BUF_SIZE];
	char full_path[MAX_PATH];
	int nread;
	int offset;
	struct vmwos_dirent *d;
	struct vmwos_stat stat_buf;
	unsigned int max_filename_len=0;
	unsigned int columns,colwidth,i,whichcol=0;

	if (debug) {
		printf("Listing plain\n");
	}

	result=stat_syscall(path,&stat_buf);
	if (result<0) {
		printf("Cannot access %s: no such file or directory!\n",path);
		return -1;
	}

	if (debug) {
		printf("Mode: %x\n",stat_buf.st_mode);
	}

	/* handle if it's not a directory */
	if ( (stat_buf.st_mode&S_IFMT)!=S_IFDIR) {
		print_file_color(stat_buf.st_mode, path);
		printf("\n");
		return 0;
	}

	/* handle if it's a directory */
	fd=open_syscall(path,O_RDONLY,0);
	if (fd<0) {
		errno=get_errno(fd);
		printf("Error opening directory %s! %s\n",
					path,strerror(errno));
	}

	/* get maximum filename size */
	while(1) {
		nread = getdents_syscall (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
			errno=get_errno(nread);
			printf("Error getdents! %s\n",strerror(errno));
			break;
		}
		if (nread==0) break;

		offset=0;
		while(offset<nread) {
			d=(struct vmwos_dirent *)(buf+offset);
			if (strlen(d->d_name)>max_filename_len) {
				max_filename_len=strlen(d->d_name);
			}
			offset+=d->d_reclen;
		}
	}

	columns=80/(max_filename_len+1);
	colwidth=80/columns;

//	FIXME: implement lseek
//	lseek(fd,0,SEEK_SET);

	close_syscall(fd);
	fd=open_syscall(path,O_RDONLY,0);

	/* Print files */
	while(1) {
		nread = getdents_syscall (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
			errno=get_errno(fd);
			printf("Error getdents again! %s\n",strerror(errno));
			break;
		}
		if (nread==0) break;

		offset=0;
		while(offset<nread) {
			d=(struct vmwos_dirent *)(buf+offset);
			snprintf(full_path,MAX_PATH,"%s/%s",path,d->d_name);
			stat_syscall(full_path,&stat_buf);
			print_file_color(stat_buf.st_mode, d->d_name);
			offset+=d->d_reclen;
			whichcol++;
			for(i=0;i<colwidth-strlen(d->d_name);i++) {
				printf(" ");
			}
			if (whichcol>=columns) {
				whichcol=0;
				printf("\n");
			}
		}
	}

	close_syscall(fd);
	printf("\n");

	return 0;
}



int main(int argc, char **argv) {

	int i;
	char *file_to_list=NULL;
	int list_long=0;

	test_glue_setup();

//	printf("%d\n",errno);

	if (debug) {
		printf("ls: argc=%d\n",argc);
		for(i=0;i<argc;i++) {
			printf("\targ%d = %s\n",i,argv[i]);
		}
	}

	for(i=1;i<argc;i++) {
		if (argv[i][0]=='-') {
			/* command line arg */
			if (argv[i][1]=='l') list_long=1;
		}
		else {
			file_to_list=argv[i];
		}

	}

	if (file_to_list==NULL) {
		if (debug) printf("Listing current dir\n");
		if (list_long) ls_long(".");
		else ls_plain(".");
	}
	else {
		if (debug) printf("Listing %s\n",file_to_list);
		if (list_long) ls_long(file_to_list);
		else ls_plain(file_to_list);
	}
	if (debug) printf("Ready to return\n");

	return 0;
}

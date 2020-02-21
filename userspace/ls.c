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

#define LS_EXE_COLOR	"\033[1;32m"	/* bright green */
#define LS_DIR_COLOR	"\033[1;34m"	/* bright blue */
#define LS_DEV_COLOR	"\033[1;33m"	/* bright yellow */
#define LS_LINK_COLOR	"\033[1;36m"	/* bright cyan */
#define LS_NORMAL_COLOR	"\033[0m"	/* normal */


#define MAX_PATH 256
#define BUF_SIZE 1024

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

static void print_file_color(int raw_mode, char *name) {

	int mode;

	mode=raw_mode&S_IFMT;

	/* FIXME: check isatty() before printing color */


	/* Dir */
	if (mode==S_IFDIR) {
		printf("%s%s%s",LS_DIR_COLOR,name,LS_NORMAL_COLOR);
	}
	/* Executable */
	else if (raw_mode & 0x1) {
		printf("%s%s%s",LS_EXE_COLOR,name,LS_NORMAL_COLOR);
	}
	/* Device */
	else if ((mode==S_IFBLK) || (mode==S_IFCHR)) {
		printf("%s%s%s",LS_DEV_COLOR,name,LS_NORMAL_COLOR);
	}
	/* Link */
	else if (mode==S_IFLNK) {
		printf("%s%s%s",LS_LINK_COLOR,name,LS_NORMAL_COLOR);
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

	struct stat stat_buf;
	int padding,i;
	char full_path[MAX_PATH];
	int mode;

	if (path==NULL) {
		snprintf(full_path,MAX_PATH,"%s",name);
	}
	else {
		snprintf(full_path,MAX_PATH,"%s/%s",path,name);
	}

	stat(full_path,&stat_buf);
	mode=stat_buf.st_mode&S_IFMT;

	print_permissions(stat_buf.st_mode);
	printf(" %2d %d %d ",
		stat_buf.st_nlink,
		stat_buf.st_uid,
		stat_buf.st_gid);

	if ((mode==S_IFCHR) || (mode==S_IFBLK)) {
		printf("%d, %d ",(stat_buf.st_dev)>>16,
			(stat_buf.st_dev&0xffff));
	}
	else {
		padding=ilog10(maxsize)-ilog10(stat_buf.st_size);
		for(i=0;i<padding;i++) printf(" ");

		printf("%lld ",stat_buf.st_size);
	}

	print_date(stat_buf.st_mtime);

	printf(" ");

	print_file_color(stat_buf.st_mode,name);
	printf("\n");

}

static int ls_long(char *path) {

	int fd,result;
	char full_path[MAX_PATH];
	char buf[BUF_SIZE];
	int nread;
	int offset;
	struct vmwos_dirent *d;
	struct stat stat_buf;
	int max_len=0;
	int mode;


	result=stat(path,&stat_buf);
	if (result<0) {
		printf("Cannot access %s: no such file or directory!\n",path);
		return -1;
	}

//	printf("Mode: %x\n",stat_buf.st_mode);

	mode=stat_buf.st_mode&S_IFMT;

	if ( mode!=S_IFDIR) {
		/* handle if it's a regular file */
		list_file_long(path,NULL,stat_buf.st_size);
		return 0;
	}

	/* handle if it's a directory */
	fd=open(path,O_RDONLY,0);
	if (fd<0) {
		printf("Error opening dir %s! %s\n",
						path,strerror(errno));
	}

	/* First, find maxsize */
	while(1) {
		nread = getdents (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
			printf("Error getdents! %s\n",strerror(errno));
			break;
		}
		if (nread==0) break;

		offset=0;
		while(offset<nread) {
			d=(struct vmwos_dirent *)(buf+offset);
			snprintf(full_path,MAX_PATH,"%s/%s",
							path,d->d_name);
			stat(full_path,&stat_buf);
			if (stat_buf.st_size>max_len) {
				max_len=stat_buf.st_size;
			}
			offset+=d->d_reclen;
		}
	}


	//	FIXME: implement lseek
	//	lseek(fd,0,SEEK_SET);

	close(fd);
	fd=open(path,O_RDONLY,0);

	/* Then actually print */
	while(1) {
		nread = getdents (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
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

	close(fd);


	return 0;
}

static int ls_inode(char *path) {

	int fd,result;
	char full_path[MAX_PATH];
	char buf[BUF_SIZE];
	int nread;
	int offset;
	struct vmwos_dirent *d;
	struct stat stat_buf;
	unsigned int max_filename_len=0;
	unsigned int columns,colwidth,i,whichcol=0;

	result=stat(path,&stat_buf);
	if (result<0) {
		printf("Cannot access %s: no such file or directory!\n",path);
		return -1;
	}

//	printf("Mode: %x\n",stat_buf.st_mode);

	/* handle if it's not a directory */
	if ( (stat_buf.st_mode&S_IFMT)!=S_IFDIR) {
		print_file_color(stat_buf.st_mode, path);
		printf("\n");
		return 0;
	}

	/* handle if it's a directory */
	fd=open(path,O_RDONLY,0);
	if (fd<0) {
		printf("Error opening directory %s! %s\n",
					path,strerror(errno));
	}

	/* get maximum filename size */
	while(1) {
		nread = getdents (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
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

	/* make room for inode size */
	max_filename_len+=9;

	columns=80/(max_filename_len+1);
	colwidth=80/columns;

//	FIXME: implement lseek
//	lseek(fd,0,SEEK_SET);

	close(fd);
	fd=open(path,O_RDONLY,0);

	/* Print files */
	while(1) {
		nread = getdents (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
			printf("Error getdents again! %s\n",strerror(errno));
			break;
		}
		if (nread==0) break;

		offset=0;
		while(offset<nread) {
			d=(struct vmwos_dirent *)(buf+offset);
			snprintf(full_path,MAX_PATH,"%s/%s",path,d->d_name);
			stat(full_path,&stat_buf);
			printf("%08x ",d->d_ino);
			print_file_color(stat_buf.st_mode, d->d_name);
			offset+=d->d_reclen;
			whichcol++;
			for(i=0;i<colwidth-(strlen(d->d_name)+9);i++) {
				printf(" ");
			}
			if (whichcol>=columns) {
				whichcol=0;
				printf("\n");
			}
		}
	}

	close(fd);
	printf("\n");

	return 0;
}


static int ls_plain(char *path) {

	int fd,result;
	char full_path[MAX_PATH];
	char buf[BUF_SIZE];
	int nread;
	int offset;
	struct vmwos_dirent *d;
	struct stat stat_buf;
	unsigned int max_filename_len=0;
	unsigned int columns,colwidth,i,whichcol=0;

	result=stat(path,&stat_buf);
	if (result<0) {
		printf("Cannot access %s: no such file or directory!\n",path);
		return -1;
	}

//	printf("Mode: %x\n",stat_buf.st_mode);

	/* handle if it's not a directory */
	if ( (stat_buf.st_mode&S_IFMT)!=S_IFDIR) {
		print_file_color(stat_buf.st_mode, path);
		printf("\n");
		return 0;
	}

	/* handle if it's a directory */
	fd=open(path,O_RDONLY,0);
	if (fd<0) {
		printf("Error opening directory %s! %s\n",
					path,strerror(errno));
	}

	/* get maximum filename size */
	while(1) {
		nread = getdents (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
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

	close(fd);
	fd=open(path,O_RDONLY,0);

	/* Print files */
	while(1) {
		nread = getdents (fd, (struct vmwos_dirent *)buf, BUF_SIZE);
		if (nread<0) {
			printf("Error getdents again! %s\n",strerror(errno));
			break;
		}
		if (nread==0) break;

		offset=0;
		while(offset<nread) {
			d=(struct vmwos_dirent *)(buf+offset);
			snprintf(full_path,MAX_PATH,"%s/%s",path,d->d_name);
			stat(full_path,&stat_buf);
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

	close(fd);
	printf("\n");

	return 0;
}


static int debug=0;

int main(int argc, char **argv) {

	int i;
	char *file_to_list=NULL;
	int list_long=0;
	int list_inode=0;

//	printf("%d\n",errno);

	if (debug) {
		printf("ls: argc=%d\n",argc);
		for(i=0;i<argc;i++) {
			printf("\targ%d = %s\n",i,argv[i]);
		}
	}

	for(i=1;i<argc;i++) {
		if (argv[i][0]=='-') {
			/* list long */
			if (argv[i][1]=='l') list_long=1;
			/* list inodes */
			if (argv[i][1]=='i') list_inode=1;
		}
		else {
			file_to_list=argv[i];
		}

	}

	if (file_to_list==NULL) {
		if (debug) printf("Listing current dir\n");
		if (list_inode) ls_inode(".");
		else if (list_long) ls_long(".");
		else ls_plain(".");
	}
	else {
		if (debug) printf("Listing %s\n",file_to_list);
		if (list_inode) ls_inode(file_to_list);
		else if (list_long) ls_long(file_to_list);
		else ls_plain(file_to_list);
	}
	if (debug) printf("Ready to return\n");

	return 0;
}

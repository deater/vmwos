
#define S_IFSOCK	0140000   /* socket		*/
#define S_IFLNK		0120000   /* symbolic link	*/
#define S_IFREG		0100000   /* regular file	*/
#define S_IFBLK		0060000   /* block device	*/
#define S_IFDIR		0040000   /* directory		*/
#define S_IFCHR		0020000   /* character device	*/
#define S_IFIFO		0010000   /* FIFO		*/

struct stat {
	uint32_t	st_dev;		/* ID of dev containing file */
	uint32_t	st_ino;		/* inode */
	uint32_t	st_mode;	/* file type and mode */
	uint32_t	st_nlink;	/* number of hard links */
	uint32_t	st_uid;		/* user ID of owner */
	uint32_t	st_gid;		/* group ID of owner */
	uint32_t	st_rdev;	/* device ID for special file */
	uint32_t	st_size;	/* file size, in bytes */
	uint32_t	st_blksize;	/* blocksize for file I/O */
	uint32_t	st_blocks;	/* number of 512B blocks */
	uint32_t	st_atime;	/* last access time */
	uint32_t	st_mtime;	/* last modify time */
	uint32_t	st_ctime;	/* last status change time */
};

uint32_t stat(const char *pathname, struct stat *buf);


struct vmwos_dirent {
	uint32_t	d_ino;
	uint32_t	d_off;
	uint32_t	d_reclen;
	char		d_name[];
};

uint32_t getdents(uint32_t fd, struct vmwos_dirent *dirp, uint32_t count);

struct timespec {
	uint32_t tv_sec;        /* seconds */
	long   tv_nsec;       /* nanoseconds */
};

int nanosleep(const struct timespec *req, struct timespec *rem);

/* See /usr/include/bits/fcntl-linux.h */
#define O_RDONLY             00
#define O_WRONLY             01
#define O_RDWR               02

uint32_t write(int fd, const void *buf, uint32_t count);
uint32_t read(int fd, void *buf, size_t count);
uint32_t open(char *filename, uint32_t flags, uint32_t mode);
uint32_t close(uint32_t fd);

#define NCCS 32

#define TCSANOW 0

/* c_iflag bits */
#define IGNBRK  0000001
#define BRKINT  0000002
#define IGNPAR  0000004
#define PARMRK  0000010
#define INPCK   0000020
#define ISTRIP  0000040
#define INLCR   0000100
#define IGNCR   0000200
#define ICRNL   0000400
#define IUCLC   0001000
#define IXON    0002000
#define IXANY   0004000
#define IXOFF   0010000
#define IMAXBEL 0020000
#define IUTF8   0040000

/* c_lflag bits */
#define ISIG    0000001
#define ICANON  0000002
#define ECHO    0000010
#define ECHOE   0000020
#define ECHOK   0000040
#define ECHONL  0000100
#define NOFLSH  0000200
#define TOSTOP  0000400
#define IEXTEN  0100000

/* c_oflag bits */
#define OPOST   0000001
#define OLCUC   0000002
#define ONLCR   0000004
#define OCRNL   0000010
#define ONOCR   0000020
#define ONLRET  0000040
#define OFILL   0000100
#define OFDEL   0000200

#define CSIZE   0000060
#define   CS5   0000000
#define   CS6   0000020
#define   CS7   0000040
#define   CS8   0000060
#define CSTOPB  0000100
#define CREAD   0000200
#define PARENB  0000400


typedef unsigned char   cc_t;
typedef unsigned int    speed_t;
typedef unsigned int    tcflag_t;

struct termios {
	tcflag_t c_iflag;      /* input modes */
        tcflag_t c_oflag;      /* output modes */
        tcflag_t c_cflag;      /* control modes */
        tcflag_t c_lflag;      /* local modes */
	cc_t c_line;                        /* line discipline */
	cc_t c_cc[NCCS];            /* control characters */
	speed_t c_ispeed;           /* input speed */
	speed_t c_ospeed;           /* output speed */
};

int tcgetattr(int fd, struct termios *termios_p);
int tcsetattr(int fd, int optional_actions,
                     const struct termios *termios_p);


#define TCSETS 0x5402
#define TCGETS 0x5401

int ioctl3(int d, unsigned long request, unsigned long req2);
int ioctl4(int d, unsigned long request, unsigned long req2, unsigned long req3);

int getpid(void);
int sys_time(void);
int sys_reboot(void);

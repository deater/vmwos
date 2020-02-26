/* major number of console char device */
#define CONSOLE_MAJOR	1

/* ioctl values */
#define TCGETS	0x5401
#define TCSETS	0x5402

#define NCCS 32

#define TCSANOW 0

/* c_iflag bits */
/* Linux default is 042400 */
#define IGNBRK	0000001		/* ignore BREAK on input */
#define BRKINT	0000002		/* have BREAK clear input queue */
#define IGNPAR	0000004		/* ignore parity errors */
#define PARMRK	0000010		/* mark parity */
#define INPCK	0000020		/* enable input parity checking */
#define ISTRIP	0000040		/* strip off 8th bit */
#define INLCR	0000100		/* trans newline to carriage return on input */
#define IGNCR	0000200		/* ignore carriage return on input */
#define ICRNL	0000400		/* trans carriage return to newline on input */
#define IUCLC	0001000		/* map uppercase to lowercase on input */
#define IXON	0002000		/* enable xon/xoff flow control on output */
#define IXANY	0004000		/* any char will restart transmit */
#define IXOFF	0010000		/* enable XON/XOFF flow control on input */
#define IMAXBEL	0020000		/* ring bell when queue is full */
#define IUTF8	0040000		/* input is UTF8 (erase works cooked mode) */

/* c_lflag bits */
/* Linux default is 105073 */
#define ISIG	0000001		/* generate INTR, QUIT, SUSP, signals */
#define ICANON	0000002		/* Enable canonical mode */
#define XCASE	0000004		/* convert input to lowecase */
#define ECHO	0000010		/* Echo input characters */
#define ECHOE	0000020		/* If ICANON, erase char/word */
#define ECHOK	0000040		/* If ICANON, kill char kills whole line */
#define ECHONL	0000100		/* If ICANON, echo new-line */
#define NOFLSH	0000200		/* Disable flushing when sending signal */
#define TOSTOP	0000400		/* Set SIGTTOU if bg process tries to write */
#define ECHOCTL	0001000		/* Echo control chars as ^X where X is char */
#define ECHOPRT	0002000		/* Print chars as being erased */
#define ECHOKE	0004000		/* Kill created by echoing each char on line */
#define FLUSHO	0010000		/* Output is being flushed */
#define PENDIN	0040000		/* All chars reprinted when next is read */
#define IEXTEN	0100000		/* implementation-defined output processing */

/* c_oflag bits */
/* Linux default is 05 */
#define OPOST	0000001		/* enable impl-defined output processing */
#define OLCUC	0000002		/* map lower to uppercase on output */
#define ONLCR	0000004		/* map new-line to new-line/CR on output */
#define OCRNL	0000010		/* map carriage return to new-line on output */
#define ONOCR	0000020		/* don't output CR on column 0 */
#define ONLRET	0000040		/* doun't output CR */
#define OFILL	0000100		/* send fill char for delay */
#define OFDEL	0000200		/* fill char is ascii DEL rather than 0 */

/* c_cflag bits */
/* Linux default is 0277 */
#define B38400	0000017
#define CSIZE   0000060
#define   CS5   0000000
#define   CS6   0000020
#define   CS7   0000040
#define   CS8   0000060
#define CSTOPB  0000100		/* Set two stop bits */
#define CREAD   0000200		/* Enable receiver */
#define PARENB  0000400		/* Enable parity generation */

/* c_cc bits */
/* c_cc characters */
#define VINTR		0	/* Interrupt character (^C) */
#define VQUIT		1	/* Quit character (^\) */
#define VERASE		2	/* Erase character (^H) del */
#define VKILL		3	/* Kill character (^U) */
#define VEOF		4	/* End-of-file char (^D) */
#define VTIME		5	/* Timeout for non-canon read (deci-seconds) */
#define VMIN		6	/* Min chars in non-canon read */
#define VSWTC		7	/* Switch characters */
#define VSTART		8	/* Restart char (^Q) */
#define VSTOP		9	/* Stop char (^S) */
#define VSUSP		10	/* Suspend char (^Z) */
#define VEOL		11	/* Additional EOL char */
#define VREPRINT	12	/* Reprint unread chars (^R) */
#define VDISCARD	13	/* Discard chars (^O) */
#define VWERASE		14	/* Word erase (^W) */
#define VLNEXT		15	/* Quote next char (^V) */
#define VEOL2		16	/* Yet another end-of-line */


typedef unsigned char   cc_t;
typedef unsigned int    speed_t;
typedef unsigned int    tcflag_t;

struct termios {
	tcflag_t c_iflag;	/* input modes */
	tcflag_t c_oflag;	/* output modes */
	tcflag_t c_cflag;	/* control modes */
	tcflag_t c_lflag;	/* local modes */
	cc_t c_line;		/* line discipline */
	cc_t c_cc[NCCS];	/* control characters */
	speed_t c_ispeed;	/* input speed */
	speed_t c_ospeed;	/* output speed */
};

#define TCSAFLUSH       2




int console_write(const void *buf, size_t count);
//int console_read(void *buf, size_t count, int non_blocking);
int console_insert_char(int ch);

void console_enable_locking(void);

struct char_dev_type *console_init(void);

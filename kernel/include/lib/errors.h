/* For compatibility reasons, based on Linux values	*/
/* /usr/include/asm-generic/errno.h			*/
/* /usr/include/asm-generic/errno-base.h		*/

#define ENOENT	2	/* File not found..... */
#define E2BIG	7	/* Argument too big... */
#define EBADF	9	/* Bad file descriptor */
#define ECHILD	10      /* No child process... */
#define ENOMEM	12	/* Not enough memory.. */
#define EBUSY	16      /* Resource busy ..... */
#define ENODEV	19	/* No such device..... */
#define ENOTDIR	20	/* Not a directory.... */
#define EISDIR	21	/* Is a directory..... */
#define EINVAL	22	/* Invalid argument... */
#define ENFILE	23	/* Not enough fds..... */
#define ENOTTY	25	/* Unhandled ioctl.... */
#define ENOSPC	28	/* No space left...... */
#define EROFS	30	/* Read-only file sys. */
#define ERANGE	34	/* Result out of range */
#define ENOSYS	38	/* No such system call */


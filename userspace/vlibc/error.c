#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"
#include "vlibc.h"

static const char error_none[]=		"No error";
static const char error_enoent[]=	"ENOENT: File not found";
static const char error_e2big[]=	"E2BIG: Too big";
static const char error_ebadf[]=	"EBADF: Bad file descriptor";
static const char error_enomem[]=	"ENOMEM: Not enougn memory";
static const char error_enodev[]=	"ENODEV: No such device";
static const char error_enotdir[]=	"ENOTDIR: Not a directory";
static const char error_eisdir[]=	"EISDIR: Is a directory";
static const char error_enfile[]=	"ENFILE: Not enough fds";
static const char error_enotty[]=	"ENOTTY: Unhandled ioctl";
static const char error_enospc[]=	"ENOSPC: No more space";
static const char error_erofs[]=	"EROFS: Read only file system";
static const char error_erange[]=	"ERANGE: Result out of range";
static const char error_enosys[]=	"ENOSYS: No such system call";
static const char error_generic[]=	"Unknown error";



static const char *error_table[MAX_ERRNO]={
	error_none,	/* 0 */
	error_generic,	/* 1 */
	error_enoent,	/* 2 ENOENT */
	error_generic,	/* 3 */
	error_generic,	/* 4 */
	error_generic,	/* 5 */
	error_generic,	/* 6 */
	error_e2big,	/* 7 E2BIG */
	error_generic,	/* 8 */
	error_ebadf,	/* 9 EBADF */
	error_generic,	/* 10 */
	error_generic,	/* 11 */
	error_enomem,	/* 12 ENOMEM */
	error_generic,	/* 13 */
	error_generic,	/* 14 */
	error_generic,	/* 15 */
	error_generic,	/* 16 */
	error_generic,	/* 17 */
	error_generic,	/* 18 */
	error_enodev,	/* 19 ENODEV */
	error_enotdir,	/* 20 ENOTDIR */
	error_eisdir,	/* 21 EISDIR */
	error_generic,	/* 22 */
	error_enfile,	/* 23 ENFILE */
	error_generic,	/* 24 */
	error_enotty,	/* 25 ENOTTY */
	error_generic,	/* 26 */
	error_generic,	/* 27 */
	error_enospc,	/* 28 ENOSPC */
	error_generic,	/* 29 */
	error_erofs,	/* 30 EROFS */
	error_generic,	/* 31 */
	error_generic,	/* 32 */
	error_generic,	/* 33 */
	error_erange,	/* 34 ERANGE */
	error_generic,	/* 35 */
	error_generic,	/* 36 */
	error_generic,	/* 37 */
	error_enosys,	/* 38 ENOSYS */
	error_generic,	/* 39 */
};




const char *strerror(int errnum) {

	if (errnum<0) errnum=-errnum;

	if (errnum>=MAX_ERRNO) {
		return error_generic;
	}
	else return error_table[errnum];
}

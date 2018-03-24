#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"

#define MAX_ERRNO	40

#define ENOENT	2	/* File not found..... */
#define EBADF	9	/* Bad file descriptor */
#define ENOMEM	12	/* Not enough memory.. */
#define ENODEV	19	/* No such device..... */
#define ENOTDIR	20	/* Not a directory.... */
#define ENFILE	23	/* Not enough fds..... */
#define ERANGE	34	/* Result out of range */
#define ENOSYS	38	/* No such system call */

//int errno=0;

static const char error_none[]=	"No error";
static const char error_enoent[]=	"File not found";
static const char error_ebadf[]=	"Bad file descriptor";
static const char error_enomem[]=	"Not enougn memory";
static const char error_enodev[]=	"No such device";
static const char error_enotdir[]=	"Not a directory";
static const char error_enfile[]=	"Not enough fds";
static const char error_erange[]=	"Result out of range";
static const char error_enosys[]=	"No such system call";
static const char error_generic[]=	"Unknown error";


static const char *error_table[MAX_ERRNO]={
	error_none,	/* 0 */
	error_generic,	/* 1 */
	error_enoent,	/* 2 ENOENT */
	error_generic,	/* 3 */
	error_generic,	/* 4 */
	error_generic,	/* 5 */
	error_generic,	/* 6 */
	error_generic,	/* 7 */
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
	error_generic,	/* 21 */
	error_generic,	/* 22 */
	error_enfile,	/* 23 ENFILE */
	error_generic,	/* 24 */
	error_generic,	/* 25 */
	error_generic,	/* 26 */
	error_generic,	/* 27 */
	error_generic,	/* 28 */
	error_generic,	/* 29 */
	error_generic,	/* 30 */
	error_generic,	/* 31 */
	error_generic,	/* 32 */
	error_generic,	/* 33 */
	error_erange,	/* 34 ERANGE */
	error_generic,	/* 35 */
	error_generic,	/* 36 */
	error_generic,	/* 37 */
	error_generic,	/* 38 */
	error_enosys,	/* 39 ENOSYS */
};




const char *strerror(int errnum) {

	if (errnum<0) errnum=-errnum;

	if (errnum>=MAX_ERRNO) {
		return error_generic;
	}
	else return error_table[errnum];
}

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"

int putchar(int c) {

	return write(1,&c,1);
}

int getchar(void) {

	int c=0;

	read(0,&c,1);

	return c;

}

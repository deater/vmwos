#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vlibc.h"

void cfmakeraw(struct termios *termios_p) {

	termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP |INLCR|IGNCR|ICRNL|IXON);
	termios_p->c_oflag &= ~OPOST;
	termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	termios_p->c_cflag &= ~(CSIZE|PARENB);
	termios_p->c_cflag |= CS8;
}

int tcgetattr(int fd, struct termios *termios_p) {

	return ioctl3(fd,TCGETS,(long)termios_p);

}

int tcsetattr(int fd, int optional_actions,
                     const struct termios *termios_p) {

        return ioctl3(fd,TCSETS,(long)termios_p);
}



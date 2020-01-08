/* Based heavily on kilo and the "Build Your Own Text Editor" tutorial  */
/* https://viewsourcecode.org/snaptoken/kilo/ */
/* kilo by antirez, tutorial by paileyq */
/* Modified to be more similar to the nano editor */

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define VMW_NANO_VERSION "0.0.1"

enum editor_keys {
	ARROW_LEFT = 0x1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN,
};

struct editor_row {
	int size;
	char *chars;
};

struct editor_config {
	struct termios orig_termios;
	int screenrows;
	int screencols;
	int cx,cy;
	int rowoff;
	int numrows;
	struct editor_row *row;
};

static struct editor_config config;

#define ABUF_MAX	4096

struct abuf {
	char b[ABUF_MAX];
	int len;
};

static void abuf_append(struct abuf *ab, const char *s, int len) {

	if (ab->len+len>=ABUF_MAX) return;

	memcpy(&ab->b[ab->len],s,len);
	ab->len+=len;
}

static void abuf_free(struct abuf *ab) {

	ab->len=0;
}

static void enable_raw_mode(void) {

	struct termios raw;

	tcgetattr(STDIN_FILENO, &config.orig_termios);

	raw=config.orig_termios;

	/* Turn off echo, turn off cannonical */
	raw.c_lflag &= ~(ECHO | ICANON);

	/* Turn off ^Z and ^C */
//	raw.c_lflag &= ~(ISIG);

	/* Turn off ^S and ^Q */
//	raw.c_iflag &= ~(IXON);

	/* Turn off ^M to lf mapping */
//	raw.c_iflag &= ~(ICRNL);

	/* Turn off ^V */
//	raw.c_lflag&= ~(IEXTEN);

	/* Turn off cr/lf expansion */
//	raw.c_oflag&= ~(OPOST);

	/* Other misc flags for RAW mode */
//	...

	/* Set to not wait for input, timeout after 1/10s */
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;


	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

}

static void disable_raw_mode(void) {

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &config.orig_termios);

}


static void safe_exit(int value, char *string) {

	/* clear screen */
	write(STDOUT_FILENO, "\x1b[2J",4);
	write(STDOUT_FILENO, "\x1b[H",3);

	/* restore raw mode */
	disable_raw_mode();

	if (value) {
		printf("\n%s\n\n",string);
	}

	/* exit */
	exit(value);
}


static void editor_append_row(char *s, size_t len) {

	int at;

	at=config.numrows;
	config.row=realloc(config.row,sizeof(struct editor_row)*
			(config.numrows+1));
	config.row[at].size=len;
	config.row[at].chars=malloc(len+1);
	memcpy(config.row[at].chars,s,len);
	config.row[at].chars[len]='\0';
	config.numrows++;
}

static void editor_open(char *filename) {

	FILE *fff;
	char *line=NULL;
	size_t linecap=0;
	ssize_t linelen;

	fff=fopen(filename,"r");
	if (!fff) {
		safe_exit(1,"Unable to open file");
	}

	while(1) {
		linelen=getline(&line,&linecap,fff);

		if (linelen==-1) break;

		while ((linelen>0) && ((line[linelen-1]=='\n')
				|| (line[linelen-1]=='\r'))) {

			linelen--;
		}
		editor_append_row(line,linelen);
	}
	free(line);
	fclose(fff);
}

static int editor_read_key(void) {

	int nread;
	char c;
	char seq[3];

	while (1) {
		nread = read(STDIN_FILENO,&c,1);
		if (nread) break;
	}
	if (nread==-1) {
		safe_exit(1,"Error reading");
	}

	/* special case escape */
	if (c=='\x1b') {
		if (read(STDIN_FILENO, &seq[0],1)!=1) return '\x1b';
		if (read(STDIN_FILENO, &seq[1],1)!=1) return '\x1b';

		if (seq[0]=='[') {

			if ((seq[1]>='0') && (seq[1]<='9')) {
				if (read(STDIN_FILENO,&seq[2],1)!=1) return '\x1b';
				if (seq[2]=='~') {
					switch(seq[1]) {
						case '1': return HOME_KEY;
						case '3': return DEL_KEY;
						case '4': return END_KEY;
						case '5': return PAGE_UP;
						case '6': return PAGE_DOWN;
						case '7': return HOME_KEY;
						case '8': return END_KEY;
					}
				}
			} else {
				switch(seq[1]) {
					case 'A': return ARROW_UP;
					case 'B': return ARROW_DOWN;
					case 'C': return ARROW_RIGHT;
					case 'D': return ARROW_LEFT;
					case 'H': return HOME_KEY;
					case 'F': return END_KEY;
				}
			}
		} else if (seq[0]=='O') {
			switch(seq[1]) {
				case 'H': return HOME_KEY;
				case 'F': return END_KEY;
			}
		}
		return '\x1b';
	}

	return c;
}

static void editor_move_cursor(int key) {

	switch(key) {
		case ARROW_LEFT:
			if (config.cx!=0) {
				config.cx--;
			}
			break;
		case ARROW_RIGHT:
			if (config.cx!=config.screencols-1) {
				config.cx++;
			}
			break;
		case ARROW_UP:
			if (config.cy!=0) {
				config.cy--;
			}
			break;
		case ARROW_DOWN:
			if (config.cy < config.numrows) {
				config.cy++;
			}
			break;
	}
}

static void editor_process_key(void) {

	int c;
	int times;

	c=editor_read_key();

	switch(c) {
		case 'x'&0x1f:
			safe_exit(0,NULL);
			break;
		case HOME_KEY:
			config.cx=0;
			break;
		case END_KEY:
			config.cx=config.screencols-1;
			break;
		case PAGE_UP:
		case PAGE_DOWN:
			times=config.screenrows;
			while(times--)
				editor_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
			break;

		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_LEFT:
		case ARROW_RIGHT:
			editor_move_cursor(c);
			break;
	}
}

static void editor_draw_rows(struct abuf *ab) {

	int y;
	char welcome[80];
	int welcomelen,padding,len;
	int filerow;

	welcomelen = snprintf(welcome, sizeof(welcome),
		"VMW Nano editor -- version %s",VMW_NANO_VERSION);
	if (welcomelen > config.screencols) welcomelen = config.screencols;
	padding=(config.screencols-welcomelen)/2;

	for(y=0;y<config.screenrows;y++) {
		filerow=y+config.rowoff;

		if (filerow>=config.numrows) {

			if ((config.numrows==0) && (y==config.screenrows/4)) {
				if (padding) {
					abuf_append(ab,"~",1);
					padding--;
				}
				while (padding--) abuf_append(ab," ",1);
				abuf_append(ab,welcome,welcomelen);
			} else {
				abuf_append(ab,"~",1);
			}
		} else {
			len=config.row[filerow].size;
			if (len>config.screencols) len=config.screencols;
			abuf_append(ab, config.row[filerow].chars,len);

		}
		/* clear to end of line */
		abuf_append(ab,"\x1b[K",3);

		if (y<config.screenrows-1) {
			abuf_append(ab,"\r\n",2);
		}
	}
}

static void editor_scroll(void) {

	/* check if cursor above visible window */
	if (config.cy < config.rowoff) {
		config.rowoff=config.cy;
	}

	/* check if cursor below visible window */
	if (config.cy>=config.rowoff+config.screenrows) {
		config.rowoff=config.cy-config.screenrows+1;
	}
}

static void editor_refresh_screen(void) {

	struct abuf ab;
	char buf[32];

	editor_scroll();

	ab.len=0;

	/* hide cursor */
	abuf_append(&ab, "\x1b[?25l",6);
	/* move to top left */
	abuf_append(&ab, "\x1b[H",3);

	editor_draw_rows(&ab);

	/* move to x,y */
	snprintf(buf, sizeof(buf), "\x1b[%d;%dH",
		(config.cy+1-config.rowoff),
		config.cx+1);
	abuf_append(&ab, buf, strlen(buf));

	/* un-hide cursor */
	abuf_append(&ab, "\x1b[?25h",6);

	write(STDOUT_FILENO, ab.b, ab.len);

	abuf_free(&ab);

}


static int get_cursor_position(int *rows, int *cols) {

	char buf[32];
	unsigned int i = 0;

	/* Ask for cursor position */
	if (write(STDOUT_FILENO, "\x1b[6n",4)!=4) {
		return -1;
	}

	/* return value looks something like <esc>[24;80R */
	while (1<sizeof(buf)-1) {
		if (read(STDIN_FILENO, &buf[i], 1)!=1) break;
		if (buf[i]=='R') break;
		i++;
	}
	buf[i]='\0';

	if ((buf[0] != '\x1b') || (buf[1] !='[')) return -1;
	if (sscanf(&buf[2],"%d;%d",rows,cols)!=2) return -1;

	return 0;
}


static int get_window_size(int *rows, int *cols) {

	struct winsize ws;
	int result;

	/* First try ioctl() */
	result=ioctl(STDOUT_FILENO,TIOCGWINSZ, &ws);
	if ((result==-1) || (ws.ws_col==0)) {
		/* try asking terminal direct */

		/* move cursor to bottom right */
		write(STDOUT_FILENO,"\x1b[999C\x1b[999B",12);

		return get_cursor_position(rows,cols);

	}
	else {
		*cols=ws.ws_col;
		*rows=ws.ws_row;
	}
	return 0;
}


static void editor_init(void) {

	int result;

	config.cx=0;
	config.cy=0;
	config.rowoff=0;
	config.numrows=0;
	config.row=NULL;

	result=get_window_size(&config.screenrows,&config.screencols);
	if (result==-1) {
		safe_exit(1,"Error getting window size");
	}
}

int main(int argc, char **argv) {

	enable_raw_mode();
	editor_init();

	if (argc>=2) {
		editor_open(argv[1]);
	}

	while(1) {
		editor_refresh_screen();
		editor_process_key();
	}

	safe_exit(0,NULL);

	return 0;
}

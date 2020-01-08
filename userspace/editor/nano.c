/* Based heavily on kilo and the "Build Your Own Text Editor" tutorial  */
/* https://viewsourcecode.org/snaptoken/kilo/ */
/* kilo by antirez, tutorial by paileyq */
/* Modified to be more similar to the nano editor */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>

#define VMW_NANO_VERSION "0.0.1"
#define NANO_TAB_STOP	8
#define NANO_QUIT_TIMES	3

#define CTRL_KEY(X) (X&0x1f)

enum editor_keys {
	BACKSPACE = 127,
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
	int rsize;
	char *chars;
	char *render;
};

struct editor_config {
	int screenrows;
	int screencols;
	int cx,cy;
	int rx;		/* rendered X location */
	int rowoff;
	int coloff;
	int numrows;
	struct editor_row *row;
	int dirty;
	char *filename;
	char statusmsg[80];
	time_t statusmsg_time;
	struct termios orig_termios;

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
	raw.c_lflag &= ~(ISIG);

	/* Turn off ^S and ^Q */
	raw.c_iflag &= ~(IXON);

	/* Turn off ^M to lf mapping */
	raw.c_iflag &= ~(ICRNL);

	/* Turn off ^V */
	raw.c_lflag&= ~(IEXTEN);

	/* Turn off cr/lf expansion */
	raw.c_oflag&= ~(OPOST);

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

static int editor_row_cx_to_rx(struct editor_row *row, int cx) {
	int rx=0;
	int j;

	for(j=0;j<cx;j++) {
		if (row->chars[j]=='\t') {
			rx+=(NANO_TAB_STOP-1)-(rx%NANO_TAB_STOP);
		}
		rx++;
	}
	return rx;
}

static void editor_update_row(struct editor_row *row) {
	int tabs = 0;
	int j,idx=0;

	for(j=0;j<row->size;j++) {
		if (row->chars[j]=='\t') tabs++;
	}

	free(row->render);
	row->render = malloc(row->size+ tabs*(NANO_TAB_STOP-1) + 1);

	for(j=0;j<row->size;j++) {
		if (row->chars[j]=='\t') {
			row->render[idx++] = ' ';
			while (idx % NANO_TAB_STOP != 0) row->render[idx++] = ' ';
		}
		else {
			row->render[idx++] = row->chars[j];
		}
	}
	row->render[idx]='\0';
	row->rsize=idx;

}

static void editor_insert_row(int at, char *s, size_t len) {

	if ((at<0) || (at>config.numrows)) return;

	config.row=realloc(config.row,sizeof(struct editor_row)*
			(config.numrows+1));
	memmove(&config.row[at+1],&config.row[at],
			sizeof(struct editor_row)*(config.numrows-at));

	config.row[at].size=len;
	config.row[at].chars=malloc(len+1);
	memcpy(config.row[at].chars,s,len);
	config.row[at].chars[len]='\0';

	config.row[at].rsize=0;
	config.row[at].render=NULL;
	editor_update_row(&config.row[at]);

	config.numrows++;
	config.dirty++;
}

static void editor_free_row(struct editor_row *row) {
	free(row->render);
	free(row->chars);
}

static void editor_delete_row(int at) {

	if ((at<0) || (at>=config.numrows)) return;

	editor_free_row(&config.row[at]);
	memmove(&config.row[at],&config.row[at+1],
		sizeof(struct editor_row) * (config.numrows - at - 1));
	config.numrows--;
	config.dirty++;
}

static void editor_row_insert_char(struct editor_row *row, int at, int c) {

	if ((at<0) || (at>row->size)) at=row->size;
	row->chars=realloc(row->chars,row->size+2);
	memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
	row->size++;
	row->chars[at] = c;
	editor_update_row(row);
	config.dirty++;
}

static void editor_row_append_string(struct editor_row *row, char *s, size_t len) {

	row->chars = realloc(row->chars, row->size+len+1);
	memcpy(&row->chars[row->size],s,len);
	row->size+=len;
	row->chars[row->size]='\0';
	editor_update_row(row);
	config.dirty++;
}

static void editor_row_delete_char(struct editor_row *row, int at) {

	if ((at<0) || (at>=row->size)) return;
	memmove(&row->chars[at],&row->chars[at+1],row->size-at);
	row->size--;
	editor_update_row(row);
	config.dirty++;
}

static void editor_insert_char(int c) {
	if (config.cy==config.numrows) {
		editor_insert_row(config.numrows,"",0);
	}
	editor_row_insert_char(&config.row[config.cy],
		config.cx,c);
	config.cx++;
}

static void editor_insert_newline(void) {

	struct editor_row *row;

	if (config.cx==0) {
		editor_insert_row(config.cy,"",0);
	}
	else {
		row=&config.row[config.cy];
		editor_insert_row(config.cy+1,&row->chars[config.cx],
				row->size-config.cx);
		row=&config.row[config.cy];
		row->size=config.cx;
		row->chars[row->size]='\0';
		editor_update_row(row);
	}
	config.cy++;
	config.cx=0;
}

static void editor_delete_char(void) {

	struct editor_row *row;

	/* If at end of file, do nothing */
	if (config.cy == config.numrows) return;

	/* If at very beginning of file, do nothing */
	if ((config.cx==0) && (config.cy==0)) return;

	row=&config.row[config.cy];
	if (config.cx>0) {
		editor_row_delete_char(row,config.cx-1);
		config.cx--;
	} else {
		/* Handle case of deleting at beginning of line */
		/* delete line and merge contents with prev line */
		config.cx=config.row[config.cy-1].size;
		editor_row_append_string(&config.row[config.cy-1],
			row->chars,row->size);
		editor_delete_row(config.cy);
		config.cy--;
	}
}

static char *editor_rows_to_string(int *buflen) {

	int totlen=0;
	int j;
	char *buf,*p;

	for(j=0;j<config.numrows;j++) {
		totlen+=config.row[j].size+1;
	}
	*buflen=totlen;

	buf=malloc(totlen);
	p=buf;

	for(j=0;j<config.numrows;j++) {
		memcpy(p,config.row[j].chars,config.row[j].size);
		p+=config.row[j].size;
		*p='\n';
		p++;
	}
	return buf;
}

static void editor_open(char *filename) {

	FILE *fff;
	char *line=NULL;
	size_t linecap=0;
	ssize_t linelen;

	free(config.filename);
	config.filename=strdup(filename);

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
		editor_insert_row(config.numrows,line,linelen);
	}
	free(line);
	fclose(fff);

	config.dirty=0;
}

static void editor_set_status_message(const char *fmt, ...) {

	va_list ap;
	va_start(ap,fmt);
	vsnprintf(config.statusmsg, sizeof(config.statusmsg), fmt, ap);
	va_end(ap);
	config.statusmsg_time = time(NULL);
}

static void editor_save(void) {

	int len;
	char *buf;
	int fd;
	if (config.filename==NULL) return;

	buf=editor_rows_to_string(&len);

	fd=open(config.filename, O_RDWR | O_CREAT, 0644);
	if (fd!=-1) {
		if (ftruncate(fd,len)!=-1) {
			if (write(fd,buf,len)==len) {
				close(fd);
				free(buf);
				config.dirty=0;
				editor_set_status_message("%d bytes written to disk",len);
				return;
			}
		}
		close(fd);
	}
	free(buf);
	editor_set_status_message("Can't save! I/O error: %s",strerror(errno));
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

	struct editor_row *row;
	int rowlen;

	row=(config.cy>config.numrows)?NULL:&config.row[config.cy];

	switch(key) {
		case ARROW_LEFT:
			if (config.cx!=0) {
				config.cx--;
			} else if (config.cy>0) {
				config.cy--;
				config.cx=config.row[config.cy].size;
			}
			break;
		case ARROW_RIGHT:
			if ( (row) && (config.cx < row->size)) {
				config.cx++;
			} else if ( (row) && (config.cx == row->size)) {
				config.cy++;
				config.cx=0;
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

	/* If we move to a new line, be sure we aren't off the end */
	row=(config.cy >= config.numrows) ? NULL:&config.row[config.cy];
	rowlen=row?row->size:0;
	if (config.cx > rowlen) {
		config.cx = rowlen;
	}
}

static void editor_process_key(void) {

	int c;
	int times;
	static int quit_times=NANO_QUIT_TIMES;

	c=editor_read_key();

	switch(c) {
		case '\r':
			editor_insert_newline();
			break;
		case CTRL_KEY('x'):
			if ((config.dirty) && (quit_times>0)) {
				editor_set_status_message("WARNING!! "
					"file has unsaved changes. "
					"Press ^X %d more times to quit.",
					quit_times);
				quit_times--;
				return;
			}
			safe_exit(0,NULL);
			break;
		case CTRL_KEY('o'):
			editor_save();
			break;
		case HOME_KEY:
			config.cx=0;
			break;
		case END_KEY:
			if (config.cy<config.numrows) {
				config.cx=config.row[config.cy].size;
			}
			break;

		case BACKSPACE:
		case CTRL_KEY('h'):
		case DEL_KEY:
			if (c==DEL_KEY) editor_move_cursor(ARROW_RIGHT);
			editor_delete_char();
			break;

		case PAGE_UP:
		case PAGE_DOWN:
			if (c==PAGE_UP) {
				config.cy=config.rowoff;
			}
			else if (c==PAGE_DOWN) {
				config.cy=config.rowoff+config.screenrows-1;
				if (config.cy>config.numrows) config.cy=config.numrows;
			}

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

		case CTRL_KEY('l'):	// ignore refresh for now
			break;

		case '\x1b':		// ignore escape being pressed
			break;

		default:
			editor_insert_char(c);
			break;
	}
	quit_times = NANO_QUIT_TIMES;
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
			len=config.row[filerow].rsize-config.coloff;
			if (len<0) len=0;
			if (len>config.screencols) len=config.screencols;
			abuf_append(ab, &config.row[filerow].render[config.coloff],len);

		}
		/* clear to end of line */
		abuf_append(ab,"\x1b[K",3);

		abuf_append(ab,"\r\n",2);

	}
}

static void editor_scroll(void) {

	config.rx=0;

	if (config.cy < config.numrows) {
		config.rx=editor_row_cx_to_rx(&config.row[config.cy],config.cx);
	}

	/* check if cursor above visible window */
	if (config.cy < config.rowoff) {
		config.rowoff=config.cy;
	}

	/* check if cursor below visible window */
	if (config.cy>=config.rowoff+config.screenrows) {
		config.rowoff=config.cy-config.screenrows+1;
	}

	if (config.rx < config.coloff) {
		config.coloff=config.rx;
	}
	if (config.rx >= config.coloff + config.screencols) {
		config.coloff = config.rx - config.screencols+1;
	}
}

static void editor_draw_status_bar(struct abuf *ab) {

	int len=0,rlen;
	char status[80],rstatus[80];

	/* make it inverted text */
	abuf_append(ab,"\x1b[7m",4);

	len=snprintf(status,sizeof(status),"%.20s - %d lines %s",
		config.filename?config.filename:"[No Name]",
		config.numrows,
		config.dirty?"(modified)":"");
	rlen=snprintf(rstatus,sizeof(rstatus),"%d/%d",
		config.cy+1,config.numrows);

	if (len>config.screencols) len=config.screencols;
	abuf_append(ab, status, len);

	while(len< config.screencols) {
		if (config.screencols-len==rlen) {
			abuf_append(ab,rstatus,rlen);
			break;
		} else {
			abuf_append(ab," ",1);
			len++;
		}
	}
	abuf_append(ab,"\x1b[m",3);
	abuf_append(ab,"\r\n",2);

}

static void editor_draw_message_bar(struct abuf *ab) {
	int msglen;
	abuf_append(ab,"\x1b[K",3);
	msglen=strlen(config.statusmsg);
	if (msglen>config.screencols) msglen=config.screencols;
	if ( (msglen) && (time(NULL) - config.statusmsg_time<5)) {
		abuf_append(ab,config.statusmsg,msglen);
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

	editor_draw_status_bar(&ab);
	editor_draw_message_bar(&ab);

	/* move to x,y */
	snprintf(buf, sizeof(buf), "\x1b[%d;%dH",
		(config.cy-config.rowoff)+1,
		(config.rx-config.coloff)+1);
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
	config.rx=0;
	config.rowoff=0;
	config.coloff=0;
	config.numrows=0;
	config.row=NULL;
	config.dirty=0;
	config.filename=NULL;
	config.statusmsg[0]='\0';
	config.statusmsg_time=0;

	result=get_window_size(&config.screenrows,&config.screencols);
	if (result==-1) {
		safe_exit(1,"Error getting window size");
	}

	/* Make room for status bar */
	config.screenrows-=2;
}

int main(int argc, char **argv) {

	enable_raw_mode();
	editor_init();

	if (argc>=2) {
		editor_open(argv[1]);
	}

	editor_set_status_message("HELP: ^O = save | ^X = quit");

	while(1) {
		editor_refresh_screen();
		editor_process_key();
	}

	safe_exit(0,NULL);

	return 0;
}

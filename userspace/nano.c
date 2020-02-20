/* Based heavily on kilo and the "Build Your Own Text Editor" tutorial  */
/* https://viewsourcecode.org/snaptoken/kilo/ */
/* kilo by antirez, tutorial by paileyq */
/* Modified to be more similar to the nano editor */

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef VMWOS
typedef uint64_t time_t;
typedef int32_t ssize_t;

#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else

#include <stdio.h>
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

#endif

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

enum editor_highlight {
	HL_NORMAL = 0,
	HL_COMMENT,
	HL_MLCOMMENT,
	HL_KEYWORD1,
	HL_KEYWORD2,
	HL_STRING,
	HL_NUMBER,
	HL_MATCH,
};

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

struct editor_syntax {
	char *filetype;
	char **filematch;
	char **keywords;
	char *singleline_comment_start;
	char *multiline_comment_start;
	char *multiline_comment_end;
	int flags;
};

static char *C_HL_extensions[] = { ".c", ".h", ".cpp", NULL};

/* TODO */
/* constants (all caps+underscores) -> red */
/* enum/etc moved to type2, green, also things ending in _t */
/* control flow: break/continue/return are purple */
/* if/else/while/for = yellow */
/* sizeof */
/* space at end of line highlighted */

static char *C_HL_keywords[] = {
	"switch", "if", "while", "for", "break", "continue", "return",
	"else", "struct", "union", "typedef", "static",
	"enum", "class", "case",

	"int|", "long|", "double|", "float|", "char|", "unsigned|",
	"signed|", "void|", NULL,
};

static struct editor_syntax HLDB[]={
	{ "c", C_HL_extensions,
		C_HL_keywords,
		"//","/*","*/",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))



struct editor_row {
	int idx;
	int size;
	int rsize;
	char *chars;
	char *render;
	unsigned char *hl;	/* highlight */
	int hl_open_comment;
};

struct editor_config {
	int screenrows;		/* Row height of screen */
	int screencols;		/* Column width of screen */
	int cx,cy;		/* Location in file */
	int rx;			/* rendered X location */
	int rowoff;
	int coloff;
	int numrows;
	struct editor_row *row;
	int dirty;
	char *filename;
	char *last_search;
	char statusmsg[80];
	time_t statusmsg_time;
	struct editor_syntax *syntax;
	struct termios orig_termios;

};

static struct editor_config config;

#define ABUF_MAX	4096

struct abuf {
	char b[ABUF_MAX];
	int len;
};

	/* FIXME: use realloc and all that */
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

static int editor_row_rx_to_cx(struct editor_row *row, int rx) {
	int cur_rx=0;
	int cx;

	for (cx=0;cx<row->size;cx++) {
		if (row->chars[cx]=='\t') {
			cur_rx+=(NANO_TAB_STOP-1)-(cur_rx&NANO_TAB_STOP);
		}
		cur_rx++;
		if (cur_rx>rx) return cx;
	}
	return cx;
}

static int is_separator(int c) {
	int result;

	result= (( isspace(c)) || (c=='\0') ||
			(strchr(",.()+-/*=~%<>[];", c) != NULL));


	/* */

	return result;
}

static void editor_update_syntax(struct editor_row *row) {

	int i,j;
	char c;
	int prev_sep,in_string,in_comment;
	unsigned char prev_hl;
	char *scs,*mcs,*mce;
	int scs_len,mcs_len,mce_len;
	char **keywords;
	int klen,kw2;
	int changed;

	row->hl = realloc(row->hl, row->rsize);
	memset(row->hl, HL_NORMAL, row->rsize);

	if (config.syntax==NULL) return;

	keywords = config.syntax->keywords;

	scs = config.syntax->singleline_comment_start;
	mcs = config.syntax->multiline_comment_start;
	mce = config.syntax->multiline_comment_end;

	scs_len = scs?strlen(scs):0;
	mcs_len = mcs?strlen(mcs):0;
	mce_len = mce?strlen(mce):0;

	prev_sep=1;
	in_string=0;
	in_comment=(row->idx>0 && config.row[row->idx-1].hl_open_comment);

	i=0;
	while (i < row->rsize) {
		c = row->render[i];

		prev_hl = (i>0)?row->hl[i-1]:HL_NORMAL;

		if (scs_len && !in_string && !in_comment) {
			if (!strncmp(&row->render[i],scs,scs_len)) {
				memset(&row->hl[i],HL_COMMENT,row->rsize-i);
				break;
			}
		}

		if (mcs_len && mce_len && !in_string) {
			if (in_comment) {
				row->hl[i] = HL_MLCOMMENT;
				if (!strncmp(&row->render[i], mce, mce_len)) {
					memset(&row->hl[i], HL_MLCOMMENT, mce_len);
					i += mce_len;
					in_comment = 0;
					prev_sep = 1;
					continue;
				} else {
					i++;
					continue;
				}
			} else if (!strncmp(&row->render[i], mcs, mcs_len)) {
				memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
				i += mcs_len;
				in_comment = 1;
				continue;
			}
		}

		if (config.syntax->flags & HL_HIGHLIGHT_STRINGS) {
			if (in_string) {
				row->hl[i] = HL_STRING;
				if (c == '\\' && i + 1 < row->rsize) {
					row->hl[i + 1] = HL_STRING;
					i += 2;
					continue;
				}
				if (c == in_string) in_string = 0;
				i++;
				prev_sep = 1;
				continue;
			} else {
				if (c == '"' || c == '\'') {
				in_string = c;
				row->hl[i] = HL_STRING;
				i++;
				continue;
			}
		}
	}

		if (config.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
			if ((isdigit(c) && (prev_sep||prev_hl==HL_NUMBER)) ||
				(c=='.' && prev_hl == HL_NUMBER)) {
				row->hl[i] = HL_NUMBER;
				i++;
				prev_sep=0;
				continue;
			}
		}

		if (prev_sep) {
			for (j = 0; keywords[j]; j++) {
				klen = strlen(keywords[j]);
				kw2 = keywords[j][klen - 1] == '|';
				if (kw2) klen--;
				if (!strncmp(&row->render[i], keywords[j], klen) &&
					is_separator(row->render[i + klen])) {
					memset(&row->hl[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
					i += klen;
					break;
				}
			}
			if (keywords[j] != NULL) {
				prev_sep = 0;
				continue;
			}
		}

		prev_sep = is_separator(c);
		i++;
	}

	changed=(row->hl_open_comment!=in_comment);
	row->hl_open_comment=in_comment;
	if (changed && row->idx+1<config.numrows) {
		editor_update_syntax(&config.row[row->idx+1]);
	}
}

static int editor_syntax_to_color(int hl) {

	switch (hl) {
		case HL_COMMENT:
		case HL_MLCOMMENT: return 36;
		case HL_KEYWORD1: return 33;
		case HL_KEYWORD2: return 32;
		case HL_STRING: return 35;
		case HL_NUMBER: return 31;
		case HL_MATCH: return 34;
		default: return 37;
	}
}


static void editor_select_syntax_highlight(void) {

	int i,j,is_ext;
	char *ext;
	struct editor_syntax *s;
	int filerow;

	config.syntax = NULL;
	if (config.filename == NULL) return;

	ext = strrchr(config.filename, '.');
	for (j = 0; j < HLDB_ENTRIES; j++) {
	s = &HLDB[j];

	i=0;
	while (s->filematch[i]) {
		is_ext = (s->filematch[i][0] == '.');
		if ((is_ext && ext &&
			!strncmp(ext, s->filematch[i],
					strlen(s->filematch[i]))) ||
			(!is_ext && strstr(config.filename, s->filematch[i]))) {
				config.syntax = s;

				for (filerow = 0; filerow < config.numrows; filerow++) {
					editor_update_syntax(&config.row[filerow]);
				}

				return;
			}
			i++;
		}
	}
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

	editor_update_syntax(row);
}

static void editor_insert_row(int at, char *s, size_t len) {

	int j;

	if ((at<0) || (at>config.numrows)) return;

	config.row=realloc(config.row,sizeof(struct editor_row)*
			(config.numrows+1));
	memmove(&config.row[at+1],&config.row[at],
			sizeof(struct editor_row)*(config.numrows-at));

	for(j=at+1;j<=config.numrows;j++) config.row[j].idx++;

	config.row[at].idx=at;

	config.row[at].size=len;
	config.row[at].chars=malloc(len+1);
	memcpy(config.row[at].chars,s,len);
	config.row[at].chars[len]='\0';

	config.row[at].rsize=0;
	config.row[at].render=NULL;
	config.row[at].hl=NULL;
	config.row[at].hl_open_comment=0;
	editor_update_row(&config.row[at]);

	config.numrows++;
	config.dirty++;
}

static void editor_free_row(struct editor_row *row) {
	free(row->render);
	free(row->chars);
	free(row->hl);
}

static void editor_delete_row(int at) {

	int j;

	if ((at<0) || (at>=config.numrows)) return;

	editor_free_row(&config.row[at]);
	memmove(&config.row[at],&config.row[at+1],
		sizeof(struct editor_row) * (config.numrows - at - 1));

	for(j=at;j<config.numrows-1;j++) config.row[j].idx--;

	config.numrows--;
	config.dirty++;
}

static void editor_row_insert_char(struct editor_row *row, int at, int c) {

	/* If somehow off screen, put this at the end */
	if ((at<0) || (at>row->size)) at=row->size;

	/* Allocate space for the updated string (with extra char and NUL) */
	row->chars=realloc(row->chars,row->size+2);

	/* Push everything past the cursor to the right */
	memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);

	/* Update the size and insert the char */
	row->size++;
	row->chars[at] = c;

	/* Update */
	editor_update_row(row);

	/* We've changed the file */
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

static void editor_set_status_message(const char *fmt, ...) {

	va_list ap;
	va_start(ap,fmt);
	vsnprintf(config.statusmsg, sizeof(config.statusmsg), fmt, ap);
	va_end(ap);
	config.statusmsg_time = time(NULL);
}



static void editor_open(char *filename) {

	FILE *fff;
	char *line=NULL;
	size_t linecap=0;
	ssize_t linelen;

	free(config.filename);
	config.filename=strdup(filename);

	editor_select_syntax_highlight();

	fff=fopen(filename,"r");
	if (!fff) {
		editor_set_status_message("New File");
		config.dirty=0;
		return;
//		safe_exit(1,"Unable to open file");
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


static void editor_draw_rows(struct abuf *ab) {

	int y,j;
	char welcome[80];
	int welcomelen,padding,len;
	int filerow;
	char *c,sym;
	unsigned char *hl;
	int color,clen;
	char buf[16];
	int current_color=-1;

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

			c = &config.row[filerow].render[config.coloff];
			hl = &config.row[filerow].hl[config.coloff];
			for (j = 0; j < len; j++) {
				/* handle control chars */
				if (iscntrl(c[j])) {
					sym=(c[j]<26)?'@'+c[j]:'?';
					abuf_append(ab,"\x1b[7m",4);
					abuf_append(ab,&sym,1);
					abuf_append(ab,"\x1b[m",3);
					if (current_color!=-1) {
						clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_color);
						abuf_append(ab,buf,clen);
					}
				}
				else if (hl[j]==HL_NORMAL) {
					if (current_color!=-1) {
						abuf_append(ab, "\x1b[39m", 5);
						current_color=-1;
					}
					abuf_append(ab, &c[j], 1);
				}
				else {
					color=editor_syntax_to_color(hl[j]);
					if (color!=current_color) {
						current_color=color;
						clen= snprintf(buf, sizeof(buf), "\x1b[%dm", color);
						abuf_append(ab, buf, clen);
					}
					abuf_append(ab, &c[j], 1);
				}
			}
			abuf_append(ab, "\x1b[39m", 5);
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
	rlen=snprintf(rstatus,sizeof(rstatus),"%s | %d/%d",
		config.syntax?config.syntax->filetype: "none",
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


/* Print a prompt in the status line */
static char *editor_prompt(char *prompt) {

	size_t bufsize=128;
	char *buf;
	size_t buflen=0;
	int c;

	buf=malloc(bufsize);
	buf[0]='\0';

	while(1) {
		editor_set_status_message(prompt,buf);
		editor_refresh_screen();

		c=editor_read_key();

		if ((c==DEL_KEY)||(c==CTRL_KEY('h')) || (c==BACKSPACE)) {
			if (buflen!=0) buf[--buflen]='\0';
		}
		/* ^C cancels */
		else if (c==CTRL_KEY('c')) {
			editor_set_status_message("");
			free(buf);
			return NULL;
		} else if (c=='\r') {
			editor_set_status_message("");
//			if (buflen!=0) {
				return buf;
//			}
		} else if ( (!iscntrl(c)) && (c<128)) {
			if (buflen==bufsize-1) {
				bufsize*=2;
				buf=realloc(buf,bufsize);
			}
			buf[buflen++]=c;
			buf[buflen]='\0';
		}
	}
}

static int get_cursor_position(int *rows, int *cols) {

	char buf[32];
	unsigned int i = 0;
	char *ptr;

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
	//if (sscanf(&buf[2],"%d;%d",rows,cols)!=2) return -1;
	ptr=&buf[2];	/* skip the escape at beginning */
	*rows=atoi(ptr);
	ptr=strchr(ptr,';');
	if (ptr==NULL) return -1;
	*cols=atoi(ptr+1);

	return 0;
}


static int get_window_size(int *rows, int *cols) {

#ifndef VMWOS
	int result;
	struct winsize ws;


	/* First try ioctl() */
	result=ioctl(STDOUT_FILENO,TIOCGWINSZ, &ws);
	if ((result==-1) || (ws.ws_col==0)) {
#endif
		/* try asking terminal direct */

		/* move cursor to bottom right */
		write(STDOUT_FILENO,"\x1b[999C\x1b[999B",12);

		return get_cursor_position(rows,cols);

#ifndef VMWOS
	}
	else {
		*cols=ws.ws_col;
		*rows=ws.ws_row;
	}
#endif
	return 0;
}

static void editor_save(void) {

	int len;
	char *buf;
	int fd;
	int reason=0,result=0;

	if (config.filename==NULL) {
		config.filename=editor_prompt("Save as: %s");
		if (config.filename==NULL) {
			editor_set_status_message("Save aborted");
			return;
		}
		editor_select_syntax_highlight();
	}

	buf=editor_rows_to_string(&len);

	fd=open(config.filename, O_RDWR | O_CREAT, 0644);
	if (fd!=-1) {
		reason=1;
		result=ftruncate(fd,len);
		if (result!=-1) {
			reason=2;
			if (write(fd,buf,len)==len) {
				reason=3;
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
	editor_set_status_message("Can't save! I/O error: %s (r=%d,re=%d,fd=%d)",
		strerror(errno),reason,result,fd);
}

/* Search / Find */
/* TODO: wrap to beginning if not found */
static void editor_find(void) {

	char *query,*actual_query,*match;
	int i,match_found=0;
	struct editor_row *row;
	char prompt[128];

	if (config.last_search==NULL) {
		query=editor_prompt("Search (^C to cancel) : %s");
	}
	else {
		snprintf(prompt,128,"Search (^C to cancel) [%s] : %%s",
			config.last_search);
		query=editor_prompt(prompt);
	}

	/* If we cancelled, exit */
	if (query==NULL) return;

	/* Update last query */
	if (strlen(query)>0) {
		free(config.last_search);
		config.last_search=strdup(query);
	}

	/* If was empty, and we have a last-value, use that */
	if (strlen(query)==0) {
		actual_query=config.last_search;
	}
	else {
		actual_query=query;
	}

	/* Note: start search from current position+1 */
	/*	 this matches nano behavior */

	for(i=config.cy;i<config.numrows;i++) {
		row=&config.row[i];

		/* If on own row, start at position+1 */
		if (i==config.cy) {
			match=strstr(row->render+1+config.cx,actual_query);
		}
		else {
			match=strstr(row->render,actual_query);
		}
		if (match) {
			config.cy=i;
			config.cx = editor_row_rx_to_cx(row,match-row->render);
			config.rowoff=config.numrows;
			match_found++;
			break;
		}

	}

	/* TODO: print "String \"Whatever\" not found */
	if (match_found==0) {
		editor_set_status_message("String not found");
	}

	free(query);
}

static void editor_process_key(void) {

	int c;
	int times;
	static int quit_times=NANO_QUIT_TIMES;

	c=editor_read_key();

	switch(c) {
		/* New line */
		case '\r':
			editor_insert_newline();
			break;

		/* Control-X = exit */
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

		/* Control-O = save */
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

		case CTRL_KEY('w'):
			editor_find();
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


static void editor_init(void) {

	int result;

	/* Current X and Y in buffer */
	config.cx=0;
	config.cy=0;

	/* Current row X */
	config.rx=0;

	/* Row and Column onscreen */
	config.rowoff=0;
	config.coloff=0;
	config.numrows=0;
	config.row=NULL;
	config.dirty=0;
	config.filename=NULL;
	config.last_search=NULL;
	config.statusmsg[0]='\0';
	config.statusmsg_time=0;
	config.syntax=NULL;

	result=get_window_size(&config.screenrows,&config.screencols);
	if (result==-1) {
		config.screencols=80;
		config.screenrows=24;
		//safe_exit(1,"Error getting window size");
	}

	/* Make room for status bar */
	config.screenrows-=2;
}

int main(int argc, char **argv) {

	/* Enable tty raw mode */
	enable_raw_mode();

	/* Setup editor */
	editor_init();

	/* Check if we want to open a pre-existing file */
	/* From the command line */
	if (argc>=2) {
		editor_open(argv[1]);
	}

	/* Print an initial HELP status message */
	/* FIXME: make this permanent line nano does? */
	editor_set_status_message("HELP: ^O = save | ^X = quit | ^W = Search");

	/* Main editing loop */
	while(1) {
		editor_refresh_screen();
		editor_process_key();
	}

	safe_exit(0,NULL);

	return 0;
}

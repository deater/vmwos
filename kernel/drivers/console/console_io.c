#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "drivers/serial/serial.h"
#include "drivers/framebuffer/framebuffer_console.h"
#include "drivers/audio/audio.h"

#include "fs/files.h"

#include "drivers/char.h"
#include "drivers/console/console_io.h"

#include "lib/string.h"
#include "lib/printk.h"
#include "lib/errors.h"
#include "lib/locks.h"
#include "lib/smp.h"

#include "processes/process.h"
#include "processes/waitqueue.h"
#include "processes/scheduler.h"

#include "debug/panic.h"

#define INPUT_BUFFER_SIZE	256

static int32_t minors_allocated=0;
static int32_t debug=0;

static char input_buffer[INPUT_BUFFER_SIZE];
static uint32_t input_buffer_head=0;
static uint32_t input_buffer_tail=0;

static uint32_t console_buffer_write_mutex = MUTEX_UNLOCKED;
//static uint32_t console_buffer_read_mutex = MUTEX_UNLOCKED;

static uint32_t console_print_mutex = MUTEX_UNLOCKED;
static uint32_t console_locking_enabled=0;


static struct termios current_termio;
static struct termios default_termio;



void console_enable_locking(void) {
	console_locking_enabled=1;
}


struct wait_queue_t console_wait_queue = {
	NULL
};





/* Inject chars into the console buffer */
static int console_actual_insert_char(int ch, int empty) {

	uint32_t new_head;

	if (empty) goto sleepers_awake;

	/* TAKE LOCK */
	/* Although this is only called from interrupt context */
	/* and we don't have re-entrant interrupts (check on that) */
	/* so in theory shouldn't be a problem */
	/* and even if we did, if an interrupt got interrupted while */
	/* the lock was held then we'd end up with deadlock.  hmmm */

	mutex_lock(&console_buffer_write_mutex);

	new_head=input_buffer_head+1;
	if (new_head>=INPUT_BUFFER_SIZE) {
		new_head=0;
	}

	/* Drop chars if buffer full */
	if (new_head==input_buffer_tail) {
		goto buffer_full;
	}

	input_buffer[input_buffer_head]=ch;
	input_buffer_head=new_head;

	/* RELEASE LOCK */
	mutex_unlock(&console_buffer_write_mutex);

sleepers_awake:
	/* Wake anyone waiting for I/O */
	wait_queue_wake(&console_wait_queue);

	return 0;

buffer_full:

	/* RELEASE LOCK */
	mutex_unlock(&console_buffer_write_mutex);

	return -1;
}

static int is_control_char(int ch) {

	if (ch>=' ') return 0;
	if (ch=='\n') return 0;
	if (ch=='\t') return 0;
	if ((ch==0x11) || (ch==0x13)) return 0; // ^Q / ^S

	return 1;

}

#define CANON_BUFFER_SIZE	256
static int canon_offset=0;
static char canon_buffer[CANON_BUFFER_SIZE];


static int console_canonical_read(int ch) {

	int i;
	int done=0;
	char control_char[]="^A";

//	printk("Starting canonical read\n");

	/* Turn carriage return into newline */
	if (current_termio.c_iflag & ICRNL) {
		if (ch=='\r') ch='\n';
	}

	/* Ignore carriage return if so requested */
	if ((current_termio.c_iflag & IGNCR) && (ch=='\r')) {
		return 0;
	}

	/* EOF */
	/* by default ^D */
	if (ch==current_termio.c_cc[VEOF]) {
		done=1;
		goto canon_no_add;
	}

	/* Delete */
	/* by default ^H and 0x7f */
	/* FIXME: does not handle multi-byte chars */
	if ((ch==0x8) || (ch==current_termio.c_cc[VERASE])) {
		if (canon_offset>0) {
			canon_offset--;
			if (current_termio.c_lflag & ECHO) {
				console_write("\b \b",3);
			}
		}
		goto canon_no_add;
	}


	/* default, add to buffer */
	canon_buffer[canon_offset]=ch;

	/* ECHO if requested */
	if (current_termio.c_lflag & ECHO) {
		if ((current_termio.c_lflag & ECHOCTL) &&
						(is_control_char(ch))) {
			control_char[1]=ch+'@';
			console_write(control_char,2);
		} else {
			console_write(&canon_buffer[canon_offset],1);
		}
	}

	canon_offset++;
	if (canon_offset==CANON_BUFFER_SIZE) {
		done=1;
	}

	/* Linefeed: FIXME check cc */
	if (ch=='\n') {
		done=1;
	}

canon_no_add:
	if (done) {
//		printk("Writing %d bytes: ",canon_offset);
		for(i=0;i<canon_offset;i++) {
			console_actual_insert_char(canon_buffer[i],0);
//			printk("%x ",canon_buffer[i]);
		}
//		printk("\n");

		/* hack */
		/* force Wake anyone waiting for I/O even if empty */
		/* otherwise ^D never processed */
		if (canon_offset==0) {
			console_actual_insert_char(0,1);
		}

		canon_offset=0;

	}

	return 0;
}


/* Inject chars into the console buffer */
/* Used by the uart and ps2 code */
int console_insert_char(int ch) {

	int result;

	/* If canonical mode, must do more complex stuff */
	if (current_termio.c_lflag & ICANON) {
		return console_canonical_read(ch);
	}


	/* Ignore carriage return if so requested */
	if ((current_termio.c_iflag & IGNCR) && (ch=='\r')) {
		return 0;
	}

	/* Turn carriage return into newline */
	if (current_termio.c_iflag & ICRNL) {
		if (ch=='\r') ch='\n';
	}

	result=console_actual_insert_char(ch,0);

	/* Emergency debug if ^B */
	if (ch==0x2) {
		dump_saved_user_state(current_proc[get_cpu()]);
	}

	/* Beep if ^G */
	if (ch==0x7) {
		audio_beep();
	}

	/* TODO: kill if ^C? */

	/* Force schedule if ^Z */
	if (ch==26) {
		scheduling_enabled=!scheduling_enabled;
	}

	return result;

}


static uint32_t console_get_char(void) {

	uint32_t result=0;

	/* TAKE LOCK */
//	lock_mutex(&console_read_mutex);

	if (input_buffer_head==input_buffer_tail) {
		result=-1;
	}
	else {
		result=input_buffer[input_buffer_tail];
		input_buffer_tail++;

		if (input_buffer_tail>=INPUT_BUFFER_SIZE) {
			input_buffer_tail=0;
		}
	}

	/* RELEASE LOCK */
//	unlock_mutex(&console_read_mutex);

	return result;

}

/* Used by printk */
int console_write(const void *buf, size_t count) {

	int result_serial;
	int result_framebuffer;

	if (console_locking_enabled) mutex_lock(&console_print_mutex);

	/* Write to Serial port */
	result_serial=serial_write(buf, count);

	/* Write to framebuffer */
	result_framebuffer=framebuffer_console_write(buf, count);
	(void)result_framebuffer;

	if (console_locking_enabled) mutex_unlock(&console_print_mutex);

	return result_serial;

}





static int console_read(void *buf, size_t count, int non_blocking) {

	int i,ch;
	unsigned char *buffer=buf;

	/* Read from input buffer */

	if (!non_blocking) {

		/* Blocking */
		/* put to sleep if no data available */
		if (input_buffer_head==input_buffer_tail) {
			wait_queue_add(&console_wait_queue,
						current_proc[get_cpu()]);
		}
	}

	for(i=0;i<count;i++) {
		ch=console_get_char();
		if (ch==-1) break;
		buffer[i]=ch;
	}

	return i;

}


static int32_t console_read_dev(struct file_object *file,
		char *buf, uint32_t count) {

	int nonblock=0;

	if (file->flags & O_NONBLOCK) nonblock=1;

	return console_read(buf,count,nonblock);

}

static int32_t console_write_dev(struct file_object *file,
		char *buf, uint32_t count) {

	return console_write(buf,count);

}

static void console_termio_update(struct termios *term) {

	int i;

	if (debug) {

	if (default_termio.c_iflag != term->c_iflag) {
		printk("term c_iflag new=%x default=%x\n",
			term->c_iflag,default_termio.c_iflag);
	}


	if (default_termio.c_oflag != term->c_oflag) {
		printk("term c_oflag new=%x default=%x\n",
			term->c_oflag,default_termio.c_oflag);
	}


	if (default_termio.c_cflag != term->c_cflag) {
		printk("term c_cflag new=%x default=%x\n",
			term->c_cflag,default_termio.c_cflag);
	}

	if (default_termio.c_lflag != term->c_lflag) {
		printk("term c_lflag new=%x default=%x\n",
			term->c_lflag,default_termio.c_lflag);
	}

	for(i=0;i<NCCS;i++) {
		if (default_termio.c_cc[i] != term->c_cc[i]) {
			printk("term c_cc[%d] new=%x default=%x\n",i,
				term->c_cc[i],default_termio.c_cc[i]);
		}
	}

	}
}

static int32_t console_ioctl(struct file_object *file,
		uint32_t cmd, uint32_t three, uint32_t four) {

	int32_t result=0;

	switch(cmd) {
		case TCGETS:
			memcpy( (struct termios *)three,
				&current_termio,
				sizeof(struct termios));
			break;
		case TCSETS:
			memcpy( &current_termio,
				(struct termios *)three,
				sizeof(struct termios));
			console_termio_update(&current_termio);
			break;
		default:
			printk("console: unhandled ioctl %x: %x %x\n",
				cmd,three,four);
			result=-ENOTTY;
			break;
	}
	return result;
}

struct char_operations console_char_ops = {
	.read = console_read_dev,
	.write = console_write_dev,
	.ioctl = console_ioctl,
};

struct char_dev_type *console_init(void) {

	struct char_dev_type *dev;

	dev=allocate_char_dev();
	if (dev==NULL) {
		return NULL;
	}

	dev->major=CONSOLE_MAJOR;
	dev->minor=minors_allocated;
	snprintf(dev->name,CHAR_NAME_LENGTH,"console%d",dev->minor);
	if (debug) printk("Allocated console %s\n",dev->name);

	minors_allocated++;

	dev->char_ops=&console_char_ops;

	/* Set up initial termio settings */
	default_termio.c_iflag= IUTF8 | IXON | ICRNL;	// 042400
	default_termio.c_lflag=	IEXTEN |
				ECHOCTL | ECHOKE | ECHOK | ECHOE | ECHO |
				ICANON | ISIG;		// 0105073
	default_termio.c_oflag = OPOST | ONLCR;		// 5
	default_termio.c_cflag = B38400 | CS8 | CREAD;	// 0277
	default_termio.c_cc[VINTR]	= 0x3;	/* ^C */
	default_termio.c_cc[VQUIT]	= 0x1c;	/* ^\ */
	default_termio.c_cc[VERASE]	= 0x7f;	/* del */
	default_termio.c_cc[VKILL]	= 0x15; /* ^U */
	default_termio.c_cc[VEOF]	= 0x04; /* ^D */
	default_termio.c_cc[VTIME]	= 0x00;
	default_termio.c_cc[VMIN]	= 0x01;
	default_termio.c_cc[VSWTC]	= 0x00;
	default_termio.c_cc[VSTART]	= 0x11;	/* ^Q */
	default_termio.c_cc[VSTOP]	= 0x13; /* ^S */
	default_termio.c_cc[VSUSP]	= 0x1a;	/* ^Z */
	default_termio.c_cc[VEOL]	= 0x00;
	default_termio.c_cc[VREPRINT]	= 0x12;	/* ^R */
	default_termio.c_cc[VDISCARD]	= 0x0f; /* ^O */
	default_termio.c_cc[VWERASE]	= 0x17;	/* ^W */
	default_termio.c_cc[VLNEXT]	= 0x16; /* ^V */

	memcpy(&current_termio,&default_termio,sizeof(struct termios));

	printk("Initialized console%d\n",dev->minor);

        return dev;

}

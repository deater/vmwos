#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "drivers/serial/serial.h"
#include "drivers/framebuffer/framebuffer_console.h"
#include "drivers/audio/audio.h"

#include "drivers/char.h"
#include "drivers/console/console_io.h"

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

void console_enable_locking(void) {
	console_locking_enabled=1;
}


struct wait_queue_t console_wait_queue = {
	NULL
};

int console_insert_char(int ch) {

	uint32_t new_head;

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

	/* Wake anyone waiting for I/O */
	wait_queue_wake(&console_wait_queue);

	return 0;

buffer_full:

	/* RELEASE LOCK */
	mutex_unlock(&console_buffer_write_mutex);

	return -1;
}


static uint32_t console_get_char(void) {

	uint32_t result=0;

	/* TAKE LOCK */
//	lock_mutex(&console_read_mutex);

	if (input_buffer_head==input_buffer_tail) {
		result=0;
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


int console_read(void *buf, size_t count, int non_blocking) {

	int i;
	unsigned char *buffer=buf;

	/* Read from input buffer */

	if (!non_blocking) {

		/* Blocking */
		/* put to sleep if no data available */
		while (input_buffer_head==input_buffer_tail) {
			wait_queue_add(&console_wait_queue,
						current_proc[get_cpu()]);
		}
	}

	for(i=0;i<count;i++) {
		buffer[i]=console_get_char();
	}

	return i;

}


static int32_t console_read_dev(struct char_dev_type *dev,
		char *buf, uint32_t count) {

	return console_read(buf,count,0);

}

static int32_t console_write_dev(struct char_dev_type *dev,
		char *buf, uint32_t count) {

	return console_write(buf,count);

}

static int32_t console_ioctl(struct char_dev_type *dev,
		uint32_t one, uint32_t two) {

	return -ENOTTY;

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

	printk("Initialized console%d\n",dev->minor);

        return dev;

}

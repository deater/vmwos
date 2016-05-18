#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "drivers/serial/pl011_uart.h"
#include "drivers/framebuffer/framebuffer_console.h"
#include "locks.h"

#include "process.h"
#include "../../panic.h"

#include "scheduler.h"

#define INPUT_BUFFER_SIZE	256

static char input_buffer[INPUT_BUFFER_SIZE];
static uint32_t input_buffer_head=0;
static uint32_t input_buffer_tail=0;

uint32_t console_write_mutex = MUTEX_UNLOCKED;
uint32_t console_read_mutex = MUTEX_UNLOCKED;

int console_insert_char(int ch) {

	uint32_t new_head;

	/* TAKE LOCK */
	/* Although this is only called from interrupt context */
	/* and we don't have re-entrant interrupts (check on that) */
	/* so in theory shouldn't be a problem */
	/* and even if we did, if an interrupt got interrupted while */
	/* the lock was held then we'd end up with deadlock.  hmmm */

//	lock_mutex(&console_write_mutex);

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
//	unlock_mutex(&console_write_mutex);

	/* Emergency debug if ^B */
	if (ch==0x2) dump_saved_user_state(current_process);

	/* Force schedule if ^Z */
	if (ch==26) scheduling_enabled=!scheduling_enabled;

	return 0;

buffer_full:

	/* RELEASE LOCK */
//	unlock_mutex(&console_write_mutex);

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

	int result;

	/* Write to framebuffer */
	result=framebuffer_console_write(buf, count);

	/* Write to UART */
	result=uart_write(buf, count);

	return result;

}


int console_read(void *buf, size_t count) {

	int i;
	unsigned char *buffer=buf;

	/* Read from input buffer */

	for(i=0;i<count;i++) {
		buffer[i]=console_get_char();
		if (buffer[i]==0) break;
	}

	return i;

}

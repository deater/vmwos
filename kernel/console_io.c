#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "serial.h"
#include "framebuffer_console.h"
#include "locks.h"

#define INPUT_BUFFER_SIZE	256

static char input_buffer[INPUT_BUFFER_SIZE];
static uint32_t input_buffer_head=0;
static uint32_t input_buffer_tail=0;

uint32_t console_mutex = MUTEX_UNLOCKED;

int console_insert_char(int ch) {

	uint32_t new_head;

	/* TAKE LOCK, we might be re-entrant from an interrupt */
	lock_mutex(&console_mutex);

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
	unlock_mutex(&console_mutex);

	return 0;

buffer_full:

	/* RELEASE LOCK */
	unlock_mutex(&console_mutex);

	return -1;
}


static uint32_t console_get_char(void) {

	uint32_t result=0;

	/* TAKE LOCK */
	lock_mutex(&console_mutex);

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
	unlock_mutex(&console_mutex);

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

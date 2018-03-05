/*
 * ps2-keyboard.c -- vmwOS driver for ps2pi PS/2 keyboard/GPIO device
 *	by Vince Weaver <vincent.weaver _at_ maine.edu>
 */

#include <stdint.h>
#include <stddef.h>

#include "lib/printk.h"

#include "drivers/gpio/gpio.h"
#include "drivers/keyboard/ps2-keyboard.h"
#include "drivers/console/console_io.h"

#include "interrupts/interrupts.h"

#include "time/time.h"

static int irq_num;

/* Default for the VMW ps2pi board */
int gpio_clk = 23;
int gpio_data = 24;

static unsigned keyup = 0;
static unsigned escape = 0;
static unsigned pause = 0;

#if 0
/* Raw SET 2 scancode table */

/* Some day we might want to be able to return raw keycodes. */
/* However, today is not that day. */

static unsigned char translate[256] = {
/* 00 */  KEY_RESERVED, KEY_F9,        KEY_RESERVED,  KEY_F5,        KEY_F3,        KEY_F1,       KEY_F2,        KEY_F12,
/* 08 */  KEY_ESC,      KEY_F10,       KEY_F8,        KEY_F6,        KEY_F4,        KEY_TAB,      KEY_GRAVE,     KEY_RESERVED,
/* 10 */  KEY_RESERVED, KEY_LEFTALT,   KEY_LEFTSHIFT, KEY_RESERVED,  KEY_LEFTCTRL,  KEY_Q,        KEY_1,         KEY_RESERVED,
/* 18 */  KEY_RESERVED, KEY_RESERVED,  KEY_Z,         KEY_S,         KEY_A,         KEY_W,        KEY_2,         KEY_RESERVED, 
/* 20 */  KEY_RESERVED, KEY_C,         KEY_X,         KEY_D,         KEY_E,         KEY_4,        KEY_3,         KEY_RESERVED,
/* 28 */  KEY_RESERVED, KEY_SPACE,     KEY_V,         KEY_F,         KEY_T,         KEY_R,        KEY_5,         KEY_RESERVED,
/* 30 */  KEY_RESERVED, KEY_N,         KEY_B,         KEY_H,         KEY_G,         KEY_Y,        KEY_6,         KEY_RESERVED,
/* 38 */  KEY_RESERVED, KEY_RIGHTALT,  KEY_M,         KEY_J,         KEY_U,         KEY_7,        KEY_8,         KEY_RESERVED,
/* 40 */  KEY_RESERVED, KEY_COMMA,     KEY_K,         KEY_I,         KEY_O,         KEY_0,        KEY_9,         KEY_RESERVED,
/* 48 */  KEY_RESERVED, KEY_DOT,       KEY_SLASH,     KEY_L,         KEY_SEMICOLON, KEY_P,        KEY_MINUS,     KEY_RESERVED,
/* 50 */  KEY_RESERVED, KEY_RESERVED,  KEY_APOSTROPHE,KEY_RESERVED,  KEY_LEFTBRACE, KEY_EQUAL,    KEY_RESERVED,  KEY_RESERVED,
/* 58 */  KEY_CAPSLOCK, KEY_RIGHTSHIFT,KEY_ENTER,     KEY_RIGHTBRACE,KEY_RESERVED,  KEY_BACKSLASH,KEY_RESERVED,  KEY_RESERVED,
/* 60 */  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_BACKSPACE, KEY_RESERVED,
/* 68 */  KEY_RESERVED, KEY_KP1,       KEY_RESERVED,  KEY_KP4,       KEY_KP7,       KEY_RESERVED, KEY_HOME,      KEY_RESERVED,
/* 70 */  KEY_KP0,      KEY_KPDOT,     KEY_KP2,       KEY_KP5,       KEY_KP6,       KEY_KP8,      KEY_ESC,       KEY_NUMLOCK,
/* 78 */  KEY_F11,      KEY_KPPLUS,    KEY_KP3,       KEY_KPMINUS,   KEY_KPASTERISK,KEY_KP9,      KEY_SCROLLLOCK,KEY_RESERVED,
/* 80 */  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,  KEY_F7,        KEY_SYSRQ,     KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* 88 */  KEY_PAUSE,    KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* 90 */  KEY_RESERVED, KEY_RIGHTALT,  KEY_RESERVED,  KEY_RESERVED,  KEY_RIGHTCTRL, KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* 98 */  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* a0 */  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* a8 */  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* b0 */  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* b8 */  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* c0 */  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* c8 */  KEY_RESERVED, KEY_RESERVED,  KEY_KPSLASH,   KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* d0 */  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* d8 */  KEY_RESERVED, KEY_RESERVED,  KEY_KPENTER,   KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* e0 */  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* e8 */  KEY_RESERVED, KEY_END,       KEY_RESERVED,  KEY_LEFT,      KEY_HOME,      KEY_RESERVED, KEY_RESERVED,  KEY_RESERVED,
/* f0 */  KEY_INSERT,   KEY_DELETE,    KEY_DOWN,      KEY_RESERVED,  KEY_RIGHT,     KEY_UP,       KEY_RESERVED,  KEY_RESERVED,
/* f8 */  KEY_RESERVED, KEY_RESERVED,  KEY_PAGEDOWN,  KEY_RESERVED,  KEY_PRINT,     KEY_PAGEUP,   KEY_RESERVED,  KEY_RESERVED

};
#endif

#define K_ESC	0x1b
#define K_RSV	0x80
#define K_F1	0x81
#define K_F2	0x82
#define K_F3	0x83
#define K_F4	0x84
#define K_F5	0x85
#define K_F6	0x86
#define K_F7	0x87
#define K_F8	0x88
#define K_F9	0x89
#define K_F10	0x8a
#define K_F11	0xab
#define K_F12	0x8c
#define K_ALT	0x8d
#define K_SHFT	0x8e
#define K_CTRL	0x8f
#define K_CPSL	0x90
#define K_HOME	0x91
#define K_NMLCK	0x92
#define K_SCLCK 0x93
#define K_SYSRQ	0x94
#define K_PAUSE	0x95
#define K_END	0x96
#define K_INSRT	0x97
#define K_DEL	0x98
#define K_PRINT	0x99
#define K_LEFT	0xa0
#define K_RIGHT	0xa1
#define K_UP	0xa2
#define K_DOWN	0xa3
#define K_PGDN	0xa4
#define K_PGUP	0xa5

static unsigned char translate[256] = {
/* 00 */  K_RSV,  K_F9,   K_RSV,  K_F5,   K_F3,   K_F1,   K_F2,   K_F12,
/* 08 */  K_ESC,  K_F10,  K_F8,   K_F6,   K_F4,   '\t',   '`',    K_RSV,
/* 10 */  K_RSV,  K_ALT,  K_SHFT, K_RSV,  K_CTRL, 'q',    '1',    K_RSV,
/* 18 */  K_RSV,  K_RSV,  'z',    's',    'a',    'w',    '2',    K_RSV,
/* 20 */  K_RSV,  'c',    'x',    'd',    'e',    '4',    '3',    K_RSV,
/* 28 */  K_RSV,  ' ',    'v',    'f',    't',    'r',    '5',    K_RSV,
/* 30 */  K_RSV,  'n',    'b',    'h',    'g',    'y',    '6',    K_RSV,
/* 38 */  K_RSV,  K_ALT,  'm',    'j',    'u',    '7',    '8',    K_RSV,
/* 40 */  K_RSV,  ',',    'k',    'i',    'o',    '0',    '9',    K_RSV,
/* 48 */  K_RSV,  '.',    '/',    'l',    ';',    'p',    '-',    K_RSV,
/* 50 */  K_RSV,  K_RSV,  '\'',   K_RSV,  '[',    '=',    K_RSV,  K_RSV,
/* 58 */  K_CPSL, K_SHFT, '\r',   ']',    K_RSV,  '\\',   K_RSV,  K_RSV,
/* 60 */  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  '\b',   K_RSV,
/* 68 */  K_RSV,  '1',    K_RSV,  '4',    '7',    K_RSV,  K_HOME, K_RSV,
/* 70 */  '0',    '.',    '2',    '5',    '6',    '8',    K_ESC,  K_NMLCK,
/* 78 */  K_F11,  '+',    '3',    '-',    '*',    '9',    K_SCLCK,K_RSV,
/* 80 */  K_RSV,  K_RSV,  K_RSV,  K_F7,   K_SYSRQ,K_RSV,  K_RSV,  K_RSV,
/* 88 */  K_PAUSE,K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* 90 */  K_RSV,  K_ALT,  K_RSV,  K_RSV,  K_CTRL, K_RSV,  K_RSV,  K_RSV,
/* 98 */  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* a0 */  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* a8 */  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* b0 */  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* b8 */  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* c0 */  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* c8 */  K_RSV,  K_RSV,  '/',    K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* d0 */  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* d8 */  K_RSV,  K_RSV,  '\r',   K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* e0 */  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,  K_RSV,
/* e8 */  K_RSV,  K_END,  K_RSV,  K_LEFT, K_HOME, K_RSV,  K_RSV,  K_RSV,
/* f0 */  K_INSRT,K_DEL,  K_DOWN, K_RSV,  K_RIGHT,K_UP,   K_RSV,  K_RSV,
/* f8 */  K_RSV,  K_RSV,  K_PGDN, K_RSV,  K_PRINT,K_PGUP, K_RSV,  K_RSV
};


void translate_key(uint32_t key, int down) {

	uint32_t ascii;

	static uint32_t shift_state=0;
	static uint32_t alt_state=0;
	static uint32_t ctrl_state=0;

	(void)alt_state;

	ascii=translate[key];

	if (ascii==K_ALT) {
		if (down) alt_state=1;
		else alt_state=0;
		return;
	}

	if (ascii==K_CTRL) {
		if (down) ctrl_state=1;
		else ctrl_state=0;
		return;
	}

	if (ascii==K_SHFT) {
		if (down) shift_state=1;
		else shift_state=0;
		return;
	}

	/* For the time being, only report keycodes at release */
	if (down) return;

	if ((ascii>='a') && (ascii<='z')) {

		/* Convert to control chars */
		if (ctrl_state) {
			ascii-=0x60;
		}

		/* convert to capital letters */
		if (shift_state) {
			ascii-=0x20;
		}
	}

	console_insert_char(ascii);

}

/* Handle GPIO interrupt */

int ps2_interrupt_handler(void) {

	static unsigned key;

	int data_value;

	static int parity=0;
	static int clock_bits=0;
	static int message=0;

	static uint32_t old_ticks=0;

	/* Sanity check clock line is low? */
//	clk_value=gpio_get_value(gpio_clk);


	if (old_ticks==0) {
		old_ticks=tick_counter;
	}

	/* If it's been too long since an interrupt, clear out the char */
	/* This probably means we lost an interrupt somehow and got out */
	/* of sync.							*/
	/* We are only running at 2HZ here */
	if ((tick_counter-old_ticks) > 2) {
		clock_bits=0;
		message=0;
		parity=0;
	}
	old_ticks=tick_counter;

	clock_bits++;

	/* read the data line */
	data_value=gpio_get_value(gpio_data);

	/* Shift in backwards as protocol is LSB first */
	parity+=data_value;
	message|=(data_value<<11);
	message>>=1;

	/* We haven't received 11 bits, so we're done for now */
	if (clock_bits!=11) {
		goto exit_handler;
	}

	/* We received our 11 bits */
	clock_bits=0;

	/* Validate our 11-bit packet */
	/* FIXME: should do something useful (request resend?) if invalid */
	if (message&0x1) {
		printk("Invalid start bit %x\n",message);
	}
	if (!(message&0x400)) {
		printk("Invaid stop bit %x\n",message);
	}
	if ( ( ((message&0x200>>8)&0x1) + (parity&0x1) ) &0x1) {
		printk("Parity error %x %x\n",message,parity);
	}

	key = (message>>1) & 0xff;
	message=0;
	parity=0;

	/* Key-up events start with 0xf0 */
	if (key == 0xf0) {
		keyup = 1;
		goto exit_handler;
	}

	/* Extended events start with 0xe0 */
	if (key == 0xe0) {
		escape = 1;
		goto exit_handler;
	}

	/* Crazy pause key starts with 0xe1, has no keyup */
	if (key == 0xe1) {
		pause = 2;
		goto exit_handler;
	}
	if (pause == 2) {
		pause = 1;
		goto exit_handler;
	}
	if (pause == 1) {
		key = 0x88;
		pause = 0;
	}

	/* Use high bit to indicate this is an extended escape keypress */
	if (escape == 1) {
		key |= 0x80;
		escape = 0;
	}

	/* Translate key and push to console */
	translate_key(key,!keyup);
	keyup=0;

	//printk("Key: %x\n",key);

exit_handler:

	gpio_clear_interrupt(gpio_clk);

	return 0;

}

/* Initialize the Module */
int ps2_keyboard_init(void) {

	uint32_t result;

	/* Allocate data/clock, use GPIO23 and GPIO24 by default */
	result=gpio_request(gpio_clk, "ps2_clock");
	if (result<0) goto init_error;
	result=gpio_request(gpio_data, "ps2_data");
	if (result<0) goto init_error;

	/* Set to be inputs */
	gpio_direction_input(gpio_clk);
	gpio_direction_input(gpio_data);

	/* Get interrupt number for clock input */
	irq_num=gpio_to_irq(gpio_clk);
	if (irq_num<0) goto init_error;

	/* FIXME */
	/* should probe to make sure keyboard actually exists */


	/* Setup IRQ */
	gpio_set_falling(gpio_clk);
	irq_enable(irq_num);

	printk("ps2-keyboard using GPIO%d/%d, irq %d\n",
		gpio_clk,gpio_data,irq_num);

	return 0;

init_error:

	printk("ps2-keyboard installation failed\n");

	return -1;

}



/* Remove module */
void ps2_keyboard_cleanup(void) {

//	free_irq(irq_num,(void *)irq_handler);

	gpio_free(gpio_data);
	gpio_free(gpio_clk);

	return;
}



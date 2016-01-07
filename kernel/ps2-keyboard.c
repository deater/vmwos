/*
 * ps2-keyboard.c -- vmwOS driver for ps2pi PS/2 keyboard/GPIO device
 *	by Vince Weaver <vincent.weaver _at_ maine.edu>
 */

#include <stdint.h>
#include "gpio.h"
#include "printk.h"
#include "ps2-keyboard.h"
#include "interrupts.h"

static int irq_num;

/* Default for the VMW ps2pi board */
int gpio_clk = 23;
int gpio_data = 24;


#if 0
static unsigned keyup = 0;
static unsigned escape = 0;
static unsigned pause = 0;

static struct input_dev *ps2;

static unsigned char translate[256] = {

/* Raw SET 2 scancode table */

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

module_param(gpio_clk,int,0);
module_param(gpio_data,int,0);

/* Handle GPIO interrupt, get keycode, send to event subsystem */

/* Pretty horrible code, not re-entrant although maybe that doesn't */
/* matter as currently you can only have one device at a time       */

irq_handler_t irq_handler(int irq, void *dev_id, struct pt_regs *regs) {

	static unsigned key;

	int clk_value;
	int data_value;

	static int parity=0;
	static int clock_bits=0;
	static int message=0;

	static unsigned long old_jiffies=0;

	/* Sanity check clock line is low? */
//	clk_value=gpio_get_value(gpio_clk);


	if (old_jiffies==0) {
		old_jiffies=jiffies;
	}

	/* If it's been too long since an interrupt, clear out the char */
	/* This probably means we lost an interrupt somehow and got out */
	/* of sync.							*/
	/* We use HZ/100 (10ms) as the threedhold.			*/
	if ((jiffies-old_jiffies) > HZ/100) {
		clock_bits=0;
		message=0;
	}
	old_jiffies=jiffies;

	clock_bits++;

	/* read the data line */
	data_value=gpio_get_value(gpio_data);

	/* Shift in backwards as protocol is LSB first */
	parity+=data_value;
	message|=(data_value<<11);
	message>>=1;

	/* We haven't received 11 bits, so we're done for now */
	if (clock_bits!=11) {
		return 0;
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

	/* Key-up events start with 0xf0 */
	if (key == 0xf0) {
		keyup = 1;
		return 0;
	}

	/* Extended events start with 0xe0 */
	if (key == 0xe0) {
		escape = 1;
		return 0;
	}

	/* Crazy pause key starts with 0xe1, has no keyup */
	if (key == 0xe1) {
		pause = 2;
		return 0;
	}
	if (pause == 2) {
		pause = 1;
		return 0;
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

	/* Translate using RAW set2 keymap */
	key = translate[key];

	/* Report the keypress to the input layer */
	if (keyup == 1) {
		input_report_key(ps2,key,0);
		keyup = 0;
	} else {
		input_report_key(ps2,key,1);
	}

	/* Sync things up */
	input_sync(ps2);

	return 0;

}
#endif

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



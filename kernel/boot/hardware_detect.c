#include <stdint.h>
#include <stddef.h>

#include "boot/device_tree.h"
#include "boot/atags.h"
#include "boot/hardware_detect.h"

#include "lib/printk.h"

static int32_t device_tree_found=0;
static int32_t atags_found=0;

/* Location of BCM2835-style peripherals */
uint32_t io_base;

/* default, this is over-ridden later */
//uint32_t hardware_type=RPI_MODEL_B;
uint32_t hardware_type=RPI_MODEL_3B;
static uint32_t hardware_revision;


/* Convert revision to our internal type */
static uint32_t hardware_convert_type(uint32_t revision) {

	uint32_t type;

	/* http://elinux.org/RPi_HardwareHistory */
	switch(revision) {
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0xd:
		case 0xe:
		case 0xf:	type=RPI_MODEL_B;
				break;
		case 0x7:
		case 0x8:
		case 0x9:	type=RPI_MODEL_A;
				break;
		case 0x10:
		case 0x13:
		case 0x900032:
				type=RPI_MODEL_BPLUS;
				break;
		case 0x11:
		case 0x14:
				type=RPI_COMPUTE_MODULE1;
				break;
		case 0xa020a0:
				type=RPI_COMPUTE_MODULE3;
				break;
		case 0x12:
		case 0x15:
		case 0x900021:	/* 512 MB */
				type=RPI_MODEL_APLUS;
				break;
		case 0x900092:
		case 0x900093:
		case 0x920093:
				type=RPI_MODEL_ZERO;
				break;
		case 0x9000c1:
				type=RPI_MODEL_ZERO_W;
				break;
		case 0x902120:
				type=RPI_MODEL_ZERO_2W;
				break;
		case 0xa01040:
		case 0xa01041:
		case 0xa21041:
				type=RPI_MODEL_2B;
				break;
		case 0xa22042:
				type=RPI_MODEL_2B_V1_2;
				break;
		case 0xa02082:
		case 0xa22082:
		case 0xa32082:
				type=RPI_MODEL_3B;
				break;
		case 0xa020d3:
				type=RPI_MODEL_3BPLUS;
				break;
		case 0x9020e0:
				type=RPI_MODEL_3APLUS;
				break;
		case 0xa03111:
		case 0xb03111:
		case 0xb03112:
		case 0xb03114:
		case 0xb03115:
		case 0xc03111:
		case 0xc03112:
		case 0xc03114:
		case 0xc03115:
		case 0xd03114:
		case 0xd03115:
				type=RPI_MODEL_4B;
				break;

		default:	type=RPI_MODEL_UNKNOWN;
				break;
	}

	return type;

}

uint32_t hardware_detect(void *info_ptr) {

	uint32_t result;

	result=devicetree_setup((uint32_t *)info_ptr);

	if (result==0) {
		device_tree_found=1;
		devicetree_find_int("system","linux,revision",&hardware_revision);
		hardware_type=hardware_convert_type(hardware_revision);
	} else {
		/* Atags is being deprecated on new Pis */
		atags_detect((uint32_t *)info_ptr);
		atags_found=1;
		hardware_type=hardware_convert_type(atags_get_revision());
	}

	return 0;
}

void hardware_print_model(uint32_t version) {

	char string[DT_STRING_MAXSIZE];

	/* Print hardware version */
	printk("\tHardware version: %x ",version);
	if (version==0xc42) printk("(Raspberry Pi)");
	else printk("(Unknown Hardware)");
	printk("\n");

	if (device_tree_found) {
		devicetree_find_string(NULL,"model",string,DT_STRING_MAXSIZE);
		printk("\tDevice Tree reports: %s\n",string);
	}

	printk("\tWe detect as: Model ");
	switch(hardware_type) {
		case RPI_MODEL_A:	printk("A"); break;
		case RPI_MODEL_APLUS:	printk("A+"); break;
		case RPI_MODEL_B:	printk("B"); break;
		case RPI_MODEL_BPLUS:	printk("B+"); break;
		case RPI_MODEL_2B:	printk("2B"); break;
		case RPI_MODEL_2B_V1_2:	printk("2B v1.2"); break;
		case RPI_MODEL_3B:	printk("3B"); break;
		case RPI_MODEL_3BPLUS:	printk("3B+"); break;
		case RPI_MODEL_3APLUS:	printk("3A+"); break;
		case RPI_MODEL_ZERO:	printk("Zero"); break;
		case RPI_MODEL_ZERO_W:	printk("Zero W"); break;
		case RPI_MODEL_ZERO_2W:	printk("Zero 2W"); break;
		case RPI_COMPUTE_MODULE1:
					printk("Compute Module 1"); break;
		case RPI_COMPUTE_MODULE3:
					printk("Compute Module 3"); break;
		case RPI_MODEL_4B:	printk("4B"); break;
		default:		printk("Unknown %x",hardware_revision);
					break;
	}
	printk("\n");

}

#define CMDLINESIZE	1024
static char cmdline[CMDLINESIZE];

void hardware_print_commandline(void) {

	printk("Command line from bootloader:\n");
	if (device_tree_found) {
		/* Print bootargs */
		devicetree_find_string("chosen","bootargs",cmdline,CMDLINESIZE);
		printk("%s\n",cmdline);
	}
	else if (atags_found) {
		printk("FIXME: print command line\n");
	}

}

uint32_t hardware_get_type(void) {
	return hardware_type;
}

/* FIXME: more advanced hardware returns multiple ranges */
void hardware_get_memory(uint32_t *start, uint32_t *length) {

	if (device_tree_found) {
		*start=0;
		*length=devicetree_get_memory();
	}
	else if (atags_found) {
		*start=0;
		*length=atags_get_memory();
	}
	else {
		printk("ERROR: did not detect memory, assuming 256MB\n");
		*start=0;
		*length=256*1024*1024;
	}
}


extern uint32_t io_base;
uint32_t act_led_gpio=0;

void hardware_setup_vars(void) {

	switch(hardware_type) {
		case RPI_MODEL_A:
			io_base=0x20000000;
			act_led_gpio=16;
			break;
		case RPI_MODEL_APLUS:
			io_base=0x20000000;
			act_led_gpio=16;
			break;
		case RPI_MODEL_B:
			io_base=0x20000000;
			break;
		case RPI_MODEL_BPLUS:
			io_base=0x20000000;
			break;
		case RPI_MODEL_2B:
			io_base=0x3f000000;
			break;
		case RPI_MODEL_2B_V1_2:
			io_base=0x3f000000;
			break;
		case RPI_MODEL_3B:
			io_base=0x3f000000;
			break;
		case RPI_MODEL_3BPLUS:
			io_base=0x3f000000;
			act_led_gpio=29;
			break;
		case RPI_MODEL_3APLUS:
			io_base=0x3f000000;
			act_led_gpio=29;
			break;
		case RPI_MODEL_ZERO:
			io_base=0x20000000;
			break;
		case RPI_MODEL_ZERO_W:
			io_base=0x20000000;
			break;
		case RPI_MODEL_ZERO_2W:
			io_base=0x3f000000;
			break;
		case RPI_COMPUTE_MODULE1:
			io_base=0x20000000;
			break;
		case RPI_COMPUTE_MODULE3:
			io_base=0x3f000000;
			break;
		case RPI_MODEL_4B:
			act_led_gpio=42;
			io_base=0xfe000000;
			break;
		default:
			io_base=0xfe000000;
			break;
	}
}


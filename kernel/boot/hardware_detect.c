#include <stdint.h>
#include <stddef.h>

#include "boot/device_tree.h"
#include "boot/atags.h"
#include "boot/hardware_detect.h"

#include "lib/printk.h"

static int32_t device_tree_found=0;
static int32_t atags_found=0;

/* default, this is over-ridden later */
//uint32_t hardware_type=RPI_MODEL_B;
static uint32_t hardware_type=RPI_MODEL_3B;
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
		case 0x13:	type=RPI_MODEL_BPLUS;
				break;
		case 0x11:
		case 0x14:	type=RPI_COMPUTE_NODE;
				break;
		case 0x12:
		case 0x15:	type=RPI_MODEL_APLUS;
				break;
		case 0x90092:
		case 0x90093:
				type=RPI_MODEL_ZERO;
				break;
		case 0xa01040:
		case 0xa01041:
		case 0xa21041:
		case 0xa22042:
				type=RPI_MODEL_2B;
				break;
		case 0xa02082:
		case 0xa22082:
				type=RPI_MODEL_3B;
				break;
		case 0xa020d3:
				type=RPI_MODEL_3BPLUS;
				break;
		default:	type=RPI_UNKNOWN;
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

	printk("\tVMWos detects as: Model ");
	switch(hardware_type) {
		case RPI_MODEL_A:	printk("A"); break;
		case RPI_MODEL_APLUS:	printk("A+"); break;
		case RPI_MODEL_B:	printk("B"); break;
		case RPI_MODEL_BPLUS:	printk("B+"); break;
		case RPI_MODEL_2B:	printk("2B"); break;
		case RPI_MODEL_3B:	printk("3B"); break;
		case RPI_MODEL_3BPLUS:	printk("3B+"); break;
		case RPI_MODEL_ZERO:	printk("Zero"); break;
		case RPI_COMPUTE_NODE:	printk("Compute Node"); break;
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

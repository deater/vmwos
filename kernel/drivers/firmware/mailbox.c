/* based on info at https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/screen01.html */
/* https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes */
/* See also arch/arm/mach-bcm2708/vcio.c */
/* https://github.com/raspberrypi/firmware/wiki/Mailboxes */

#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/firmware/mailbox.h"

#include "memory/mmu-common.h"

/* See include/drivers/firmware/mailbox.h for a long explanation */
uint32_t firmware_phys_to_bus_address(uint32_t addr) {

#ifdef ARMV7
	return (addr|0xC0000000);
#else
	return (addr|0x40000000);
#endif
}

static void dump_mailbox_buffer(uint32_t value) {

	uint32_t *values,len,i;

	values=(uint32_t *)value;
	len=values[0]/4;
	printk("MAILBOX: printing data of len %d at 0x%x\n",len,value);

	if (len>16) {
		printk("  truncating length\n");
		len=16;
	}

	for(i=0;i<len;i++) {
		printk("   mailbox_data[%d] = 0x%x\n",
			i,values[i]);
	}
}


/* should always write mailbox1 */
int mailbox_write(unsigned int value, unsigned int channel) {

	printk("MAILBOX_WRITE: writing value=%x channel %x\n",
		value,channel);

	dump_mailbox_buffer(value);

#ifdef ARMV7
	printk("Flushing %x\n",value);
//	asm volatile ("MCR p15, 0, %0, c7, c14, 1" : : "r" (value) : "memory");
#endif

	/* Bottom 4 bits of value must be 0 */
	if (value&0xf) {
		printk("mailbox_write: bottom bits not zero %x\n",
			value);
		return -1;
	}

	/* Channel must fit in 4 bits */
	if (channel>15) {
		printk("mailbox_write: channel too high %x\n",
			channel);
		return -1;
	}

	/* Wait until mailbox is ready */

	while( (bcm2835_read(MAILBOX1_STATUS) & MAIL_FULL) ) {
		printk("Write mailbox full!\n");
	}

#ifdef ARMV7
	asm volatile("DMB ISHST");
	asm volatile ("MCR p15, 0, %0, c7, c14, 1" : : "r" (value) : "memory");
#endif

        /* GPU wrote to physical memory, but it is probably cached */
//      printk("Flushing from %x to %x (%x to %x)\n",
//                      (uint32_t)&msg,
//                      (uint32_t)&msg+sizeof(msg),
//                      ((uint32_t)&msg)&0xfffffff0,
//                      ((uint32_t)&msg+sizeof(msg))&0xfffffff0);

        /* Flush dcache so we read in the value from memory */
//      flush_dcache((uint32_t)&msg, (uint32_t)&msg+sizeof(msg));

	flush_dcache(value,value+4096);

	/* write the command, channel bottom 4 bits */
	bcm2835_write(MAILBOX_WRITE,
		firmware_phys_to_bus_address(value)|channel);

	return 0;
}

/* should always read from mailbox 0 */
int mailbox_read(unsigned int channel) {

	unsigned int mail;
	unsigned int channel_read;

	printk("MAILBOX_READ: reading channel %x\n",
		channel);

	/* Channel must be 4-bits */
	if (channel>15) {
		printk("mailbox_read: channel too high\n",
			channel);
		return -1;
	}

	/* Wait until mailbox has something there */

	while((bcm2835_read(MAILBOX0_STATUS) & MAIL_EMPTY) ) {
		printk("mailbox_read: mail_empty\n");
	}

#ifdef ARMV7
	asm volatile("DMB ISHST");
#endif

	mail=bcm2835_read(MAILBOX_READ);

	/* get the channel we read from */
	channel_read=mail&0xf;

	/* return to virtual address from bus address */
	mail=mail&0x3ffffff0;

	/* flush dcache so CPU can see updated value */
	flush_dcache(mail,mail+4096);

#ifdef ARMV7
	asm volatile ("MCR p15, 0, %0, c7, c14, 1" : : "r" (mail) : "memory");
#endif

	/* Got mail from the wrong channel!*/
	/* FIXME: Should we try again? */

	if (channel_read!=channel) {
		printk("mailbox_read: read from wrong channel %x (%x expected)\n",
			channel_read,channel);
		return -1;
	}

	dump_mailbox_buffer(mail);

	/* Return top 28 bits */
	return mail;
}

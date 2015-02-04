#include <stdint.h>
#include "atags.h"
#include "printk.h"

/* Original references */
/* http://www.raspberrypi.org/forums/viewtopic.php?t=10889&p=123721 */
/* http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html */


void dump_atags(uint32_t *atags) {

	/* each tag has at least 32-bit size and then 32-bit value 	*/
	/* some tags have multiple values				*/

	int size;
	int tag_value;
	uint32_t *tags=atags;
	char *cmdline;

	printk("Dumping ATAGs provied by bootloader:\r\n");

	while(1) {
		size=tags[0];

		switch (tags[1]) {

		/* Start List */
		case ATAG_CORE:
			printk("  CORE: flags: %x, pagesize: %x, "
				"rootdev: %x\r\n", tags[2], tags[3], tags[4]);
			tags += size;
			break;

		/* Physical Memory */
		case ATAG_MEM:
			printk("  MEMORY: size: %x (%dMB), start: %x\r\n",
				tags[2], tags[2]/1024/1024, tags[3]);
			tags += size;
			break;

		/* VGA Text Display */
		case ATAG_VIDEOTEXT:
			printk("  VIDEOTEXT x,y,video: %x, "
				"mode,cols, ega: %x, lines, vga, points: %x\r\n",
				tags[2], tags[3], tags[4]);
			tags += size;
			break;

		/* RAMDISK Use */
		case ATAG_RAMDISK:
			printk("  RAMDISK flags: %x, size: %x, start: %x\r\n",
				tags[2], tags[3], tags[4]);
			tags += size;
			break;

		/* INITRD Ramdisk */
		case ATAG_INITRD2:
			printk("  INITRD size: %x, start: %x\r\n",
				tags[3], tags[2]);
			tags += size;
			break;

		/* 64-bit serial number */
		case ATAG_SERIAL:
			printk(" SERIAL NUMBER: low: %x, high: %x\r\n",
				tags[2], tags[3]);
			tags += size;
			break;

		/* Board Revision */
		case ATAG_REVISION:
			printk(" Board revision: %x\r\n", tags[2]);
			tags += size;
			break;

		/* VESA Framebuffer Info */
		case ATAG_VIDEOLFB:
			printk("  VESA framebuffer\r\n");
			tags += size;
			break;

		case ATAG_CMDLINE:
			printk("  Commandline: ");
			cmdline = (char *)(&tags[2]);
			printk(cmdline);
			tags += size;
			break;

		/* Empty tag to end list */
		case ATAG_NONE:
			printk("\r\n");
			return;
			break;

		default:
			printk("ERROR! Unknown atag\r\n");
			break;
		}

	}
}

#include <stdint.h>
#include "atags.h"
#include "printk.h"
#include "hardware.h"

/* Original references */
/* http://www.raspberrypi.org/forums/viewtopic.php?t=10889&p=123721 */
/* http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html */


void atags_dump(uint32_t *atags) {

	/* each tag has at least 32-bit size and then 32-bit value 	*/
	/* some tags have multiple values				*/

	uint32_t size,i;
//	int tag_value;
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
			for(i=0;i<size;i++) {
				printk("%c",cmdline[i]);
			}
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



static int parse_cmdline_int(char *cmdline, char *key) {

	int i=0,digit=0,rev=-1;

	while(1) {
		if (cmdline[i]==0) break;

		if ((cmdline[i]=='r') &&
			(cmdline[i+1]=='e') &&
			(cmdline[i+2]=='v')) {

			digit=cmdline[i+6];
			if (digit<='9') rev=digit-'0';
			else rev=(digit-'a')+10;

			if (cmdline[i+7]!=' ') {
				rev*=16;
				digit=cmdline[i+7];
				if (digit<='9') rev+=digit-'0';
				else rev+=(digit-'a')+10;

			}

			break;
		}

		i++;

	}
	return rev;
}

void atags_detect(uint32_t *atags, struct atag_info_t *info) {

	/* each tag has at least 32-bit size and then 32-bit value 	*/
	/* some tags have multiple values				*/

	uint32_t size,total_ram,rev;
//	int tag_value;
	uint32_t *tags=atags;
	char *cmdline;

	/* clear out the info array */
	memset(info,0,sizeof(struct atag_info_t));

	while(1) {
		size=tags[0];

		switch (tags[1]) {

		/* Start List */
		case ATAG_CORE:
			tags += size;
			break;

		/* Physical Memory */
		case ATAG_MEM:
			total_ram=tags[2];
			if (tags[3]!=0) {
				printk("Warning!  We do not handle memory not starting at zero!\r\n");
			}
			info->ramsize=total_ram;
			tags += size;
			break;

		/* VGA Text Display */
		case ATAG_VIDEOTEXT:
			tags += size;
			break;

		/* RAMDISK Use */
		case ATAG_RAMDISK:
			tags += size;
			break;

		/* INITRD Ramdisk */
		case ATAG_INITRD2:
			tags += size;
			break;

		/* 64-bit serial number */
		case ATAG_SERIAL:
			tags += size;
			break;

		/* Board Revision */
		case ATAG_REVISION:
			tags += size;
			break;

		/* VESA Framebuffer Info */
		case ATAG_VIDEOLFB:
			tags += size;
			break;

		case ATAG_CMDLINE:
			cmdline = (char *)(&tags[2]);

			rev=parse_cmdline_int(cmdline,"rev");

			/* http://elinux.org/RPi_HardwareHistory */
			switch(rev) {
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x5:
				case 0x6:
				case 0xd:
				case 0xe:
				case 0xf:	info->hardware_type=RPI_MODEL_B;
						break;
				case 0x7:
				case 0x8:
				case 0x9:	info->hardware_type=RPI_MODEL_A;
						break;
				case 0x10:	info->hardware_type=RPI_MODEL_BPLUS;
						break;
				case 0x11:	info->hardware_type=RPI_COMPUTE_NODE;
						break;
				case 0x12:	info->hardware_type=RPI_MODEL_APLUS;
						break;

				default:	info->hardware_type=RPI_UNKNOWN;
						break;
			}

			tags += size;
			break;

		/* Empty tag to end list */
		case ATAG_NONE:
			return;
			break;

		default:
			printk("ERROR! Unknown atag\r\n");
			break;
		}

	}
}



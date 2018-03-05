#include <stdint.h>

#include "boot/atags.h"
#include "lib/printk.h"
#include "boot/hardware_detect.h"
#include "lib/string.h"
#include "lib/memset.h"

/* Original references */
/* http://www.raspberrypi.org/forums/viewtopic.php?t=10889&p=123721 */
/* http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html */

static struct atag_info_t atag_info;

void atags_dump(uint32_t *atags) {

	/* each tag has at least 32-bit size and then 32-bit value 	*/
	/* some tags have multiple values				*/

	uint32_t size,count;
	int32_t length;
//	int tag_value;
	uint32_t *tags=atags;
	char *cmdline;

	printk("Dumping ATAGs provied by bootloader:\n");

	while(1) {
		size=tags[0];

		switch (tags[1]) {

		/* Start List */
		case ATAG_CORE:
			printk("  CORE: flags: %x, pagesize: %x, "
				"rootdev: %x\n", tags[2], tags[3], tags[4]);
			tags += size;
			break;

		/* Physical Memory */
		case ATAG_MEM:
			printk("  MEMORY: size: %x (%dMB), start: %x\n",
				tags[2], tags[2]/1024/1024, tags[3]);
			tags += size;
			break;

		/* VGA Text Display */
		case ATAG_VIDEOTEXT:
			printk("  VIDEOTEXT x,y,video: %x, "
				"mode,cols, ega: %x, lines, vga, points: %x\n",
				tags[2], tags[3], tags[4]);
			tags += size;
			break;

		/* RAMDISK Use */
		case ATAG_RAMDISK:
			printk("  RAMDISK flags: %x, size: %x, start: %x\n",
				tags[2], tags[3], tags[4]);
			tags += size;
			break;

		/* INITRD Ramdisk */
		case ATAG_INITRD2:
			printk("  INITRD size: %x, start: %x\n",
				tags[3], tags[2]);
			tags += size;
			break;

		/* 64-bit serial number */
		case ATAG_SERIAL:
			printk(" SERIAL NUMBER: low: %x, high: %x\n",
				tags[2], tags[3]);
			tags += size;
			break;

		/* Board Revision */
		case ATAG_REVISION:
			printk(" Board revision: %x\n", tags[2]);
			tags += size;
			break;

		/* VESA Framebuffer Info */
		case ATAG_VIDEOLFB:
			printk("  VESA framebuffer\n");
			tags += size;
			break;

		case ATAG_CMDLINE:
			cmdline = (char *)(&tags[2]);
			length=strlen(cmdline);
			printk("  Commandline (%d): ",length);

			/* Get around 256 char printk limit */
			count=0;
			while(length>0) {
				printk("%s",
					cmdline+(256*count));
				length-=256;
				count++;
			}
			printk("\n");
			tags += size;
			break;

		/* Empty tag to end list */
		case ATAG_NONE:
			printk("\n");
			return;
			break;

		default:
			printk("ERROR! Unknown atag\n");
			break;
		}

	}
}



static int parse_cmdline_int(char *cmdline, char *key) {

	uint32_t i=0,digit=0,base=10;
	int32_t result=0;
	uint32_t cmdline_len,key_len;

	cmdline_len=strlen(cmdline);
	key_len=strlen(key);

	while(1) {

		if (cmdline[i]==0) break;
		if (i+key_len>cmdline_len) break;

		if (!strncmp(key,cmdline+i,key_len)) {

			/* +1 to skip equals sign */
			i+=key_len;
			i++;

			if ((cmdline[i]=='0') && (cmdline[i+1]=='x')) {
				base=16;
				i+=2;
			}
			else {
				base=10;
			}

			while(1) {

				if (i+key_len>cmdline_len) break;

				digit=cmdline[i];

				if ((digit==' ') || (digit==0)) break;

				result*=base;

				if (digit<='9') result+=(digit-'0');
				else result+=((digit-'a')+10);

				i++;
			}

			break;
		}

		i++;

	}
	return result;
}

void atags_detect(uint32_t *atags) {

	/* each tag has at least 32-bit size and then 32-bit value 	*/
	/* some tags have multiple values				*/

	uint32_t size,total_ram;
//	int tag_value;
	uint32_t *tags=atags;
	char *cmdline;

	/* clear out the info array */
	memset(&atag_info,0,sizeof(struct atag_info_t));

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
				printk("Warning!  We do not handle memory not starting at zero!\n");
			}
			atag_info.ramsize=total_ram;
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

			atag_info.framebuffer_x=parse_cmdline_int(cmdline,"fbwidth");
			atag_info.framebuffer_y=parse_cmdline_int(cmdline,"fbheight");

			atag_info.revision=parse_cmdline_int(cmdline,"rev");

			tags += size;
			break;

		/* Empty tag to end list */
		case ATAG_NONE:
			return;
			break;

		default:
			printk("ERROR! Unknown atag\n");
			break;
		}

	}
}


uint32_t atags_get_memory(void) {

	return atag_info.ramsize;

}
uint32_t atags_get_revision(void) {

	return atag_info.revision;

}


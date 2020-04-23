#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <string.h>
#endif

#include "svmwgraph.h"

#define PCX_UNKNOWN	0
#define PCX_8BIT	1
#define PCX_24BIT	2

static int debug=1;

int vmwLoadPCX(unsigned char *image, int x_offset, int y_offset,
	unsigned char *buffer, int buffer_xsize)  {

	int x,i,numacross,xsize,ysize,xmin,ymin,total;
	int xmax,ymax;
	unsigned char temp_byte;
	int version,bpp,type;
	unsigned char *image_pointer;
	int output_pointer=0;
	int bytes_per_line;

	xmin=(image[5]<<8)+image[4];
	ymin=(image[7]<<8)+image[6];

	xmax=(image[9]<<8)+image[8];
	ymax=(image[11]<<8)+image[10];

	version=image[1];
	bpp=image[3];

	if ((version==5) && (bpp==8) && (image[65]==3)) type=PCX_24BIT;
	else if (version==5) type=PCX_8BIT;
	else type=PCX_UNKNOWN;

	if (debug) {
		printf("Manufacturer: ");
		if (image[0]==10) printf("Zsoft\n");
		else printf("Unknown %i\n",image[0]);

		printf("Version: ");
		switch(version) {
			case 0: printf("2.5\n"); break;
			case 2: printf("2.8 w palette\n"); break;
			case 3: printf("2.8 w/o palette\n"); break;
			case 4: printf("Paintbrush for Windows\n"); break;
			case 5: printf("3.0+\n"); break;
			default: printf("Unknown %i\n",version);
		}
		printf("Encoding: ");
		if (image[2]==1) printf("RLE\n");
		else printf("Unknown %i\n",image[2]);

		printf("Type: ");
		if (type==PCX_UNKNOWN) printf("Unknown!\n");
		if (type==PCX_8BIT) printf("8-bit\n");
		if (type==PCX_24BIT) printf("24-bit\n");

		printf("BitsPerPixelPerPlane: %i\n",bpp);
		printf("File goes from %i,%i to %i,%i\n",xmin,ymin,xmax,ymax);

		printf("Horizontal DPI: %i\n",(image[13]<<8)+image[12]);
		printf("Vertical   DPI: %i\n",(image[15]<<8)+image[14]);

		printf("Number of colored planes: %i\n",image[65]);
		printf("Bytes per line: %i\n",(image[67]<<8)+image[66]);
		bytes_per_line=(image[67]<<8)+image[66];
		printf("Palette Type: %i\n",(image[69]<<8)+image[68]);
		printf("Hscreen Size: %i\n",(image[71]<<8)+image[70]);
		printf("Vscreen Size: %i\n",(image[73]<<8)+image[72]);

	}

	xsize=(xmax-xmin+1);
	ysize=(ymax-ymin+1);


	total=0;
	x=0;

	output_pointer=(y_offset*buffer_xsize)+x_offset;

	image_pointer=image+128;

	while (total<bytes_per_line*ysize) {

		/* read a byte */
		temp_byte=*image_pointer;
		image_pointer++;

		/* if > 0xc0, then it's a RLE byte */
		if (0xc0 == (temp_byte&0xc0)) {
			numacross=temp_byte&0x3f;
			temp_byte=*image_pointer;
			image_pointer++;

//			printf("x=%d numacross=%d color=%d\n",
//				x,numacross,temp_byte);
			for(i=0;i<numacross;i++) {
				if (x<xsize) {
					buffer[output_pointer]=temp_byte;
				}
				output_pointer++;
				total++;
				x++;
				if (x>bytes_per_line-1) {
					//printf("[%d+%d] ",
					//	output_pointer,XSIZE-xsize);
					output_pointer+=(buffer_xsize-bytes_per_line);
					//printf("x=%d numacross=%d i=%d color=%d\n",
					//	x,numacross,i,temp_byte);

					x=0;

					/* skip padding */
					break;

				}
				//printf("%d ",x);
			}
		}
		else {
//			printf("x=%d numacross=1 color=%d\n",
//				x,temp_byte);
			if (x<xsize) buffer[output_pointer]=temp_byte;
			output_pointer++;
			total++;
			x++;
			if (x>bytes_per_line-1) {
				x=0;
				output_pointer+=(buffer_xsize-bytes_per_line);
				//printf("[%d] ",output_pointer);
			}
			//printf("%d ",x);
		}
	}


	return 0;
}

int vmwPCXLoadPalette(unsigned char *image, int offset, struct palette *pal) {

	unsigned char *pal_pointer,temp_byte;
	unsigned char r,g,b;
	int i;

	/*Load Palette*/

	pal_pointer=image+offset;

//	result=lseek(pcx_fd,-769,SEEK_END);

	temp_byte=*pal_pointer;
	pal_pointer++;

	if (temp_byte!=12) {
		printf("Error!  No palette found! 0%x 0%x\n",
			temp_byte,offset);
		return -1;
	}

	for(i=0;i<256;i++) {
		r=*pal_pointer;
		pal_pointer++;
		g=*pal_pointer;
		pal_pointer++;
		b=*pal_pointer;
		pal_pointer++;

		pal->red[i]=r;
		pal->green[i]=g;
		pal->blue[i]=b;

	}
	return 0;
}

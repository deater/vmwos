//#include <stdio.h>	/* For FILE I/O */
//#include <string.h>	/* For strncmp */
//#include <fcntl.h>	/* for open()  */
//#include <unistd.h>	/* for lseek() */
//#include <sys/stat.h>	/* for file modes */
//#include <stdlib.h>	/* exit() */

#include <stddef.h>
#include <stdint.h>
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"



int vmwLoadPCX(char *filename, char *raw_image) {

	int pcx_fd,debug=1,bpp;
	int x,y;
	int i,numacross;
	int xsize,ysize,plane;
//	unsigned int r,g,b;
//	int bpp,planes,bpl,xmin,ymin,xmax,ymax,version,type;
//	int bpp,planes,bpl;
	int xmin,ymin,xmax,ymax,version;
	unsigned char pcx_header[128];
	unsigned char temp_byte;

	/* Open the file */
	pcx_fd=open(filename,O_RDONLY,0);

	if (pcx_fd<0) {
		printf("ERROR!  File \"%s\" not found!\n",filename);
		return -1;
	}

/*************** DECODE THE HEADER *************************/

	//lseek(pcx_fd,0,SEEK_SET);

	read(pcx_fd,&pcx_header,128);

	xmin=(pcx_header[5]<<8)+pcx_header[4];
	ymin=(pcx_header[7]<<8)+pcx_header[6];

	xmax=(pcx_header[9]<<8)+pcx_header[8];
	ymax=(pcx_header[11]<<8)+pcx_header[10];

	version=pcx_header[1];
	bpp=pcx_header[3];

	if (debug) {
		printf("Manufacturer: ");
		if (pcx_header[0]==10) printf("Zsoft\n");
		else printf("Unknown %d\n",pcx_header[0]);

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
		if (pcx_header[2]==1) printf("RLE\n");
		else printf("Unknown %d\n",pcx_header[2]);

		printf("BitsPerPixelPerPlane: %d\n",bpp);
		printf("File goes from %i,%i to %i,%i\n",xmin,ymin,xmax,ymax);

		printf("Horizontal DPI: %i\n",(pcx_header[13]<<8)+pcx_header[12]);
		printf("Vertical   DPI: %i\n",(pcx_header[15]<<8)+pcx_header[14]);

		printf("Number of colored planes: %i\n",pcx_header[65]);
		printf("Bytes per line: %i\n",(pcx_header[67]<<8)+pcx_header[66]);
		printf("Palette Type: %i\n",(pcx_header[69]<<8)+pcx_header[68]);
		printf("Hscreen Size: %i\n",(pcx_header[71]<<8)+pcx_header[70]);
		printf("Vscreen Size: %i\n",(pcx_header[73]<<8)+pcx_header[72]);
	}

//	if ((version==5) && (bpp==8) && (pcx_header[65]==3)) type=24;
//	else if (version==5) type=8;
//	else type=0;


	xsize=((xmax-xmin)+1);
	ysize=((ymax-ymin)+1);

	x=0; y=0;

	int rgb[800][4];

	while(y<ysize) {
		for(plane=0;plane<4;plane++) {
			x=0;
			while (x<xsize) {
				read(pcx_fd,&temp_byte,1);
				if (0xc0 == (temp_byte&0xc0)) {
					numacross=temp_byte&0x3f;
					read(pcx_fd,&temp_byte,1);
					//printf("%i pixels of %i\n",numacross,temp_byte);
					for(i=0;i<numacross;i++) {
						rgb[x][plane]=temp_byte;
						x++;
					}
				}
				else {
//					printf("%i, %i Color=%i\n",x,y,temp_byte);
					rgb[x][plane]=temp_byte;
					x++;
				}
			}
		}

		for(i=0;i<xsize;i++) {
			raw_image[(y*xsize*3)+(i*3)+0]=rgb[i][0];
			raw_image[(y*xsize*3)+(i*3)+1]=rgb[i][1];
			raw_image[(y*xsize*3)+(i*3)+2]=rgb[i][2];
//			 (*output)[offset]=(rgb[i][0]<<24 | rgb[i][1]<<16 | rgb[i][2]<<8);
//			offset++;
		}
		y++;
	}

	close(pcx_fd);

	return 0;
}

/* Force it into the data segment */
/* FIXME when bss properly implemented */
//char pixels[800*600*3]={1};

int main(int argc, char **argv) {

	int result;
	int slide=0;
	int ch;
	char filename[32];

	char *pixels;

//	if (argc<2) {
//		filename=default_filename;
//	}
//	else {
//		filename=argv[1];
//	}

	printf("Starting slide show!\n");

	/* allocating memory */
	pixels=vmwos_malloc(800*600*3);

	printf("Memsetting %p\n",pixels);
	memset(pixels,0xff,800*600*3);

	while(1) {

		sprintf(filename,"out-%d.pcx",slide);

		result=vmwLoadPCX(filename,pixels);
		if (result<0) {
			printf("Error opening %s\n",filename);
			exit(1);
		}


		vmwos_framebuffer_load(800, 600, 24, pixels);

		printf("About to get character\n");

		ch=getchar();

		printf("Read character %x\n",ch);

		if (ch==' ') slide++;
		if (ch=='q') break;
		if (ch=='b') {
			slide--;
			if (slide<0) slide=0;
		}
	}

	return 0;
}

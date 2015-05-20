#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>


#define HARD_LINK	0
#define DIRECTORY	1
#define REGULAR_FILE	2
#define SYMBOLIC_LINK	3
#define BLOCK_DEVICE	4
#define CHAR_DEVICE	5
#define SOCKET		6
#define	FIFO		7
#define EXECUTABLE	8

static unsigned int affs_checksum(FILE *fff,long begin,long end,int max) {

	unsigned int checksum=0;

	int temp_int,i;
	long size;
	long saved_location;

	saved_location=ftell(fff);

	size=end-begin;

	if (max!=0) if (size>max) size=max;

	size=size/4;

	fprintf(stderr,"Checksum size %ld going from %lx to %lx\n",size,
		begin,end);

	fseek(fff,begin,SEEK_SET);
	for(i=0;i<size;i++) {
		fread(&temp_int,4,1,fff);
//		fprintf(stderr,"%x ",temp_int);
		checksum+=ntohl(temp_int);
		fprintf(stderr,"(%x %x)",checksum,ntohl(temp_int));
	}
	fprintf(stderr,"\n");
	fseek(fff,saved_location,SEEK_SET);

	return (0xffffffff-checksum)+1;

}

int main(int argc, char **argv) {

	FILE *fff,*ggg;
	char filename[]="out.romfs";

	unsigned char buffer[16];
	int int_buffer;
	int i,j,padding;
	int temp_int;

	int size=0,checksum=0;
	long last_offset=0,next_offset=0,blargh_offset=0;
	int file_size,ch;

	fff=fopen(filename,"w+");
	if (fff==NULL) {
		fprintf(stderr,"Error opening %s\n",filename);
		return -1;
	}


	/**********/
	/* Header */
	/**********/
	fprintf(stderr,"Creating header...\n");

	/* Write magic number */
	/* Should also have size and checksum, we will write those later */
	memcpy(buffer,"-rom1fs-",8);
	fwrite(buffer,16,1,fff);
	size+=16;

	/* Write volume name, aligned at 16-bytes */
	memset(buffer,0,16);
	memcpy(buffer,"VMWos",5);
	fwrite(buffer,16,1,fff);
	size+=16;

	/***************/
	/* . and ..    */
	/***************/
	fprintf(stderr,"Adding . and ..\n");

	temp_int=htonl(0x40|DIRECTORY|EXECUTABLE);
	memcpy(buffer,&temp_int,4);
	temp_int=htonl(0x20);		/* point to ourself as first entry?? */
	memcpy(buffer+4,&temp_int,4);
	temp_int=htonl(0x0);		/* size */
	memcpy(buffer+8,&temp_int,4);
	temp_int=htonl(0x0);		/* empty checksum */
	memcpy(buffer+12,&temp_int,4);
	fwrite(buffer,16,1,fff);
	/* filename */
	memset(buffer,0,16);
	memcpy(buffer,".",1);
	fwrite(buffer,16,1,fff);
	size+=0x20;

	/* checksum */
	checksum=affs_checksum(fff,0x20,0x40,0);
	temp_int=htonl(checksum);
	memcpy(buffer,&temp_int,4);
	fseek(fff,-20,SEEK_CUR);
	fwrite(buffer,4,1,fff);

	fseek(fff,size,SEEK_SET);

	temp_int=htonl(0x60);		/* Hard link, next is 0x60 */
	memcpy(buffer,&temp_int,4);
	temp_int=htonl(0x20);		/* hard link to . */
	memcpy(buffer+4,&temp_int,4);
	temp_int=htonl(0x0);		/* size is 0 */
	memcpy(buffer+8,&temp_int,4);
	temp_int=htonl(0x0);
	memcpy(buffer+12,&temp_int,4);	/* empty checksum */
	fwrite(buffer,16,1,fff);
	/* filename */
	memset(buffer,0,16);
	memcpy(buffer,"..",2);
	fwrite(buffer,16,1,fff);

	size+=0x20;

	/* checksum */
	checksum=affs_checksum(fff,0x40,0x60,0);
	temp_int=htonl(checksum);
	memcpy(buffer,&temp_int,4);
	fseek(fff,-20,SEEK_CUR);
	fwrite(buffer,4,1,fff);

	fseek(fff,size,SEEK_SET);


	/************/
	/* FILES    */
	/************/
	fprintf(stderr,"Adding files...\n");

	for(i=1;i<argc;i++) {
		printf("\tAdding %s\n",argv[i]);

		/* save offset */
		last_offset=ftell(fff);

		/* Skip header for now */
		memset(buffer,0,16);
		fwrite(buffer,16,1,fff);
		size+=16;

		for(j=0;j<strlen(argv[i]);j++) {
			fwrite(&argv[i][j],1,1,fff);
		}

		memset(buffer,0,16);
		padding=16-(strlen(argv[i])%16);
		fwrite(buffer,padding,1,fff);

		blargh_offset=ftell(fff);


		/* update total filesize */
		size+=strlen(argv[i])+padding;

		ggg=fopen(argv[i],"r");
		if (ggg==NULL) {
			fprintf(stderr,"Error! Could not open %s\n",argv[i]);
			return -1;
		}

		file_size=0;

		/* append the file contents */
		while(1) {
			ch=fgetc(ggg);
			if (ch==EOF) break;
			fputc(ch,fff);
			file_size++;
		}

		fclose(ggg);

		/* pad to 16-byte boundary */
		memset(buffer,0,16);
		padding=16-(file_size%16);
		fwrite(buffer,padding,1,fff);

		/* update total filesize */
		size+=file_size+padding;

		fprintf(stderr,"\tfile_size=%d padding=%d ",
			file_size,padding);

		next_offset=ftell(fff);

		/* Rewind and print header */
		fseek(fff,last_offset,SEEK_SET);

		/* write file info */

		/* type */
		temp_int=htonl(REGULAR_FILE);
		memcpy(buffer,&temp_int,4);
		fwrite(buffer,4,1,fff);

		/* extra */
		temp_int=htonl(0);
		memcpy(buffer,&temp_int,4);
		fwrite(buffer,4,1,fff);

		/* size */
		temp_int=htonl(file_size);
		memcpy(buffer,&temp_int,4);
		fwrite(buffer,4,1,fff);

		/* checksum */
		/* ugh, only the metadta, not whole file */
		checksum=affs_checksum(fff,last_offset,blargh_offset,0);
		temp_int=htonl(checksum);
		memcpy(buffer,&temp_int,4);
		fwrite(buffer,4,1,fff);

		fprintf(stderr,"checksum %x\n",checksum);

		/* Fast-forward back to end */
		fseek(fff,next_offset,SEEK_SET);

	}

	/********/
	/* size */
	/********/
	fprintf(stderr,"Updating size (%d)\n",size);

	fseek(fff,8,SEEK_SET);
	int_buffer=htonl(size);
	memcpy(buffer,&int_buffer,4);
	fwrite(buffer,4,1,fff);

	/************/
	/* checksum */
	/************/

	checksum=affs_checksum(fff,0,512,size);

	fprintf(stderr,"Updating overall checksum (%x)...\n",checksum);
	fseek(fff,12,SEEK_SET);
	int_buffer=htonl(checksum);
	memcpy(buffer,&int_buffer,4);
	fwrite(buffer,4,1,fff);

	/*************/
	/* Pad to 1k */
	/*************/

	/* Linux driver wants this */
	buffer[0]=0;
	if (size<1024) {
		fseek(fff,1023,SEEK_SET);
		fwrite(buffer,1,1,fff);
	}

	fprintf(stderr,"Finished\n");

	fclose(fff);

	return 0;
}

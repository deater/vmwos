#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

struct romfs_header_t {
	char magic[8];
	int size;
	int checksum;
	char volume_name[17];
	int first_offset;
} header;

struct romfs_file_header_t {
	int addr;
	int next;
	int type;
	int special;
	int size;
	int checksum;
	int filename_start;
	int data_start;
} file_header;

int main(int argc, char **argv) {

	FILE *fff;
	int temp_int;
	int debug=1;
	int ch;

	char buffer[16];

	if (argc<2) {
		fprintf(stderr,"Error! need filename argument\n\n");
		return -1;
	}

	fff=fopen(argv[1],"r");
	if (fff==NULL) {
		fprintf(stderr,"Error opening %s\n\n",argv[1]);
		return -1;
	}

	/* Read header */
	fread(header.magic,8,1,fff);
	if (memcmp(header.magic,"-rom1fs-",8)) {
		fprintf(stderr,"Wrong magic number!\n\n");
		return -1;
	}

	if (debug) printf("Found romfs filesystem!\n");

	/* Read size */
	fread(&temp_int,4,1,fff);
	header.size=ntohl(temp_int);

	if (debug) printf("Size: %d bytes\n",header.size);

	/* Read checksum */
	fread(&temp_int,4,1,fff);
	header.checksum=ntohl(temp_int);

	if (debug) printf("Checksum: %x\n",header.size);
	/* FIXME: validate checksum */

	/* Read volume name */
	/* FIXME, various overflow possibilities */
	/* We only record last 16 bytes in header */
	/* We really don't care about volume name */
	while(1) {
		fread(buffer,16,1,fff);
		memcpy(header.volume_name,buffer,16);
		if (buffer[15]==0) break;	/* NUL terminated */
	}
	if (debug) printf("Volume: %s\n",header.volume_name);

	while(1) {
		file_header.addr=ftell(fff);

		/* Next */
		fread(&temp_int,4,1,fff);
		file_header.next=ntohl(temp_int)&~0xf;
		file_header.type=ntohl(temp_int)&0xf;

		/* Special */
		fread(&temp_int,4,1,fff);
		file_header.special=ntohl(temp_int);
		/* Size */
		fread(&temp_int,4,1,fff);
		file_header.size=ntohl(temp_int);
		/* Checksum */
		fread(&temp_int,4,1,fff);
		file_header.checksum=ntohl(temp_int);

		file_header.filename_start=ftell(fff);
		while(1) {
			fread(buffer,16,1,fff);
			if (buffer[15]==0) break;	/* NUL terminated */
		}

		file_header.data_start=ftell(fff);


		fseek(fff,file_header.filename_start,SEEK_SET);
		while( (ch=fgetc(fff))>0) {
			putchar(ch);
		}
		printf("\n");
		printf("\tAddr: 0x%x\n",file_header.addr);
		printf("\tNext: 0x%x\n",file_header.next);
		printf("\tType: 0x%x\n",file_header.type);
		printf("\tSize: %d\n",file_header.size);
		printf("\tChecksum: %x\n",file_header.checksum);

		fseek(fff,file_header.next,SEEK_SET);

		if (file_header.next==0) break;
	}


	fclose(fff);

	return 0;
}

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include <arpa/inet.h>

#define BFLT_MAGIC_OFFSET       0x00
#define BFLT_VERSION_OFFSET     0x04
#define BFLT_ENTRY              0x08
#define BFLT_DATA_START         0x0C
#define BFLT_BSS_START          0x10
#define BFLT_BSS_END            0x14
#define BFLT_STACK_SIZE         0x18
#define BFLT_RELOC_START        0x1C
#define BFLT_RELOC_COUNT        0x20
#define BFLT_FLAGS              0x24

int main(int argc, char **argv) {

	int fd;
	uint32_t buffer[16];
	char magic[5];

	if (argc<2) {
		printf("Usage: %s file\n",argv[0]);
	}

	fd=open(argv[1],O_RDONLY);
	if (fd<0) {
		fprintf(stderr,"Error opening %s, %s\n",
			argv[1],strerror(errno));
	}

	if (read(fd,buffer,64)<64) {
		fprintf(stderr,"Error reading!\n");
	}

	memcpy(magic,(unsigned char *)&buffer[0],4);
	magic[4]=0;
	printf("Found magic: %s\n",magic);
	if (strncmp(magic,"bFLT",4)) {
		fprintf(stderr,"Bad magic...\n");
		exit(1);
	}

	printf("\tVersion: 0x%x\n",ntohl(buffer[BFLT_VERSION_OFFSET/4]));
	printf("\tEntry: 0x%x\n",ntohl(buffer[BFLT_ENTRY/4]));
	printf("\tData Start: 0x%x\n",ntohl(buffer[BFLT_DATA_START/4]));
	printf("\tBSS Start: 0x%x\n",ntohl(buffer[BFLT_BSS_START/4]));
	printf("\tBSS End: 0x%x\n",ntohl(buffer[BFLT_BSS_END/4]));

	close(fd);

	return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>

static void print_usage(char *name) {

	printf("%s elf_filename bflt_filename\n\n",name);

}

int main(int argc, char **argv) {

	int fd,out,i;
	struct stat sb;
	char *addr;
	uint32_t temp;
	uint16_t temp16;

	if (argc<3) {
		print_usage(argv[0]);
		return 5;
	}

	fd=open(argv[1],O_RDONLY);
	if (fd<0) {
		printf("Error opening file %s\n",argv[1]);
		return 6;
	}

	out=open(argv[2],O_CREAT|O_WRONLY,0666);
	if (out<0) {
		printf("Error opening file %s\n",argv[2]);
		return 7;
	}

	if (fstat(fd, &sb) < 0) {
		printf("Error with fstat\n");
		return 7;
	}

	addr=mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		printf("Error mmaping!\n");
		return 8;
	}

	if ((addr[0]==0x7f) &&
		(addr[1]=='E') && (addr[2]=='L') && (addr[3]=='F')) {
		printf("Found elf\n");
	}
	else {
		printf("NOT AN ELF FILE!\n");
		return 9;
	}
	printf("Size: %d bit\n",addr[4]==1?32:64);
	printf("Endian: %s\n",addr[5]==1?"little":"big");
	printf("Version %d\n",addr[6]);
	printf("ABI %d\n",addr[7]);
	printf("Type %d:%d\n",addr[0x10],addr[0x11]);
	printf("ISA %d:%d (40==ARM)\n",addr[0x12],addr[0x13]);
	memcpy(&temp,&addr[0x14],4);
	printf("Version again %d\n",temp);
	memcpy(&temp,&addr[0x18],4);
	printf("Entry %x\n",temp);
	memcpy(&temp,&addr[0x1c],4);
	printf("Phoff %x\n",temp);
	memcpy(&temp,&addr[0x20],4);
	printf("Shoff %x\n",temp);
	memcpy(&temp,&addr[0x24],4);
	printf("Flags %x\n",temp);
	memcpy(&temp16,&addr[0x28],2);
	printf("ehsize %x\n",temp16);
	memcpy(&temp16,&addr[0x2a],2);
	printf("phentsize %x\n",temp16);
	memcpy(&temp16,&addr[0x2c],2);
	printf("phnum %x\n",temp16);
	memcpy(&temp16,&addr[0x2e],2);
	printf("shentsize %x\n",temp16);
	memcpy(&temp16,&addr[0x30],2);
	printf("shnum %x\n",temp16);
	memcpy(&temp16,&addr[0x32],2);
	printf("shstrndx %x\n",temp16);



	/***************************/
	/* Write out the bflt file */
	/***************************/

	/* magic */
	write(out,"bFLT",4);

	/* version */
	temp=htonl(4);
	write(out,&temp,4);

	/* entry */
	temp=htonl(0x40);
	write(out,&temp,4);

	/* data-start */
	temp=htonl(0x80);
	write(out,&temp,4);

	/* bss-start */
	temp=htonl(0x90);
	write(out,&temp,4);

	/* bss-end */
	temp=htonl(0xa0);
	write(out,&temp,4);

	/* stack size */
	temp=htonl(4096);
	write(out,&temp,4);

	/* reloc count */
	temp=0;
	write(out,&temp,4);

	/* flags */
	temp=0;
	write(out,&temp,4);

	/* padding */
	for(i=0;i<6;i++) {
		write(out,&temp,4);
	}

	munmap(addr, sb.st_size);

	close(fd);
	close(out);
	return 0;
}

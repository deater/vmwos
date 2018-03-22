#include <stdio.h>
#include <stdint.h>
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

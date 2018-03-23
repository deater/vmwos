#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#define SHT_NULL		0x0 	// Section header entry unused
#define SHT_PROGBITS		0x1	// Program data
#define SHT_SYMTAB		0x2 	// Symbol table
#define SHT_STRTAB		0x3	// String table
#define SHT_RELA 		0x4	// Relocation entries with addends
#define SHT_HASH 		0x5	// Symbol hash table
#define SHT_DYNAMIC 		0x6	// Dynamic linking information
#define SHT_NOTE 		0x7	// Notes
#define SHT_NOBITS 		0x8	// Program space with no data (bss)
#define SHT_REL 		0x9	// Relocation entries, no addends
#define	SHT_SHLIB 		0xa	// Reserved
#define	SHT_DYNSYM 		0xb	// Dynamic linker symbol table
#define	SHT_INIT_ARRAY 		0xe	// Array of constructors
#define	SHT_FINI_ARRAY 		0xf	// Array of destructors
#define	SHT_PREINIT_ARRAY	0x10	// Array of pre-constructors
#define SHT_GROUP 		0x11	// Section group
#define SHT_SYMTAB_SHNDX	0x12	// Extended section indeces
#define	SHT_NUM			0x13	// Number of defined types.
#define	SHT_LOOS		0x60000000 // Start OS-specific.
#define SHT_ARM_ATTRIBUTES	0x70000003
static void print_type_name(uint32_t type) {

	switch(type) {
		case SHT_NULL:		printf("NULL"); break;
		case SHT_PROGBITS:	printf("PROGBITS"); break;
		case SHT_SYMTAB:	printf("SYMTAB"); break;
		case SHT_STRTAB:	printf("STRTAB"); break;
		case SHT_RELA:		printf("RELA"); break;
		case SHT_HASH:		printf("HASH"); break;
		case SHT_DYNAMIC:	printf("DYNAMIC"); break;
		case SHT_NOTE:		printf("NOTE"); break;
		case SHT_NOBITS:	printf("NOBITS"); break;
		case SHT_REL:		printf("REL"); break;
		case SHT_SHLIB:		printf("SHLIB"); break;
		case SHT_DYNSYM:	printf("DYNSYM"); break;
		case SHT_INIT_ARRAY:	printf("INIT_ARRAY"); break;
		case SHT_FINI_ARRAY:	printf("FINI_ARRAY"); break;
		case SHT_PREINIT_ARRAY:	printf("PREINIT_ARRAY"); break;
		case SHT_GROUP:		printf("GROUP"); break;
		case SHT_SYMTAB_SHNDX:	printf("SYMTAB_SHNDX"); break;
		case SHT_NUM:		printf("NUM"); break;
		case SHT_ARM_ATTRIBUTES:printf("ARM_ATTRIBUTES"); break;
		default:
			printf("Unknown type %x\n",type);
	}
}


static void print_usage(char *name) {

	printf("%s elf_filename bflt_filename\n\n",name);

}

int main(int argc, char **argv) {

	int fd,out,i;
	struct stat sb;
	char *addr,*shptr,*string_pointer;
	uint32_t temp;
	uint16_t temp16;

	uint32_t phoff,phnum;
	uint32_t shnum,shoff,shsize,strindex;


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
	phoff=temp;
	printf("Phoff %x\n",phoff);

	memcpy(&temp,&addr[0x20],4);
	shoff=temp;

	printf("Shoff %x\n",temp);
	memcpy(&temp,&addr[0x24],4);
	printf("Flags %x\n",temp);
	memcpy(&temp16,&addr[0x28],2);
	printf("ehsize %x\n",temp16);
	memcpy(&temp16,&addr[0x2a],2);
	printf("phentsize %x\n",temp16);
	memcpy(&temp16,&addr[0x2c],2);
	phnum=temp16;
	printf("phnum %x\n",temp16);
	memcpy(&temp16,&addr[0x2e],2);
	printf("shentsize %x\n",temp16);
	shsize=temp16;

	memcpy(&temp16,&addr[0x30],2);
	shnum=temp16;
	printf("shnum %x\n",temp16);
	memcpy(&temp16,&addr[0x32],2);
	strindex=temp16;
	printf("shstrndx %x\n",temp16);

	printf("%d ph entries starting at %x\n",phnum,phoff);

	printf("%d sh entries starting at %x, size %d\n",
		shnum,shoff,shsize);

	shptr=&addr[shoff];

	/* get string pointer */
	string_pointer=&shptr[strindex*shsize];
	memcpy(&temp,&string_pointer[0x10],4);

	string_pointer=&addr[temp];


	for(i=0;i<shnum;i++) {
		printf("Section header %d\n",i);

		memcpy(&temp,&shptr[0x0],4);
		printf("\tname: %s\n",string_pointer+temp);

		memcpy(&temp,&shptr[0x4],4);
		printf("\ttype: "); print_type_name(temp); printf("\n");
		memcpy(&temp,&shptr[0x10],4);
		printf("\toffset: %x\n",temp);
		memcpy(&temp,&shptr[0x14],4);
		printf("\tsize: %x\n",temp);


		shptr+=shsize;
	}

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

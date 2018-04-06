#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>

static int debug=0;

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
	char *addr,*shptr,*string_pointer,*name;
	uint32_t temp;
	uint16_t temp16;

	uint32_t phoff,phnum;
	uint32_t shnum,shoff,shsize,strindex;

	uint32_t bss_size=0,data_size=0,text_size=0;
	uint32_t text_start=0,data_start=0,bss_start=0,bss_end=0;
//	uint32_t data_offset=0,text_offset=0,bss_offset=0;
	uint32_t entry=0,size,offset;

	if (argc<3) {
		print_usage(argv[0]);
		return 5;
	}

	fd=open(argv[1],O_RDONLY);
	if (fd<0) {
		printf("Error opening file %s\n",argv[1]);
		return 6;
	}

	out=open(argv[2],O_CREAT|O_TRUNC|O_WRONLY,0777);
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
		if (debug) printf("Found elf\n");
	}
	else {
		printf("NOT AN ELF FILE!\n");
		return 9;
	}

	if (debug) {
		printf("Size: %d bit\n",addr[4]==1?32:64);
		printf("Endian: %s\n",addr[5]==1?"little":"big");
		printf("Version %d\n",addr[6]);
		printf("ABI %d\n",addr[7]);
		printf("Type %d:%d\n",addr[0x10],addr[0x11]);
		printf("ISA %d:%d (40==ARM)\n",addr[0x12],addr[0x13]);
		memcpy(&temp,&addr[0x14],4);
		printf("Version again %d\n",temp);
	}

	memcpy(&temp,&addr[0x18],4);
	entry=temp;
	if (debug) printf("Entry %x\n",temp);

	memcpy(&temp,&addr[0x1c],4);
	phoff=temp;
	if (debug) printf("Phoff %x\n",phoff);

	memcpy(&temp,&addr[0x20],4);
	shoff=temp;
	if (debug) printf("Shoff %x\n",temp);

	if (debug) {
		memcpy(&temp,&addr[0x24],4);
		printf("Flags %x\n",temp);
		memcpy(&temp16,&addr[0x28],2);
		printf("ehsize %x\n",temp16);
		memcpy(&temp16,&addr[0x2a],2);
		printf("phentsize %x\n",temp16);
	}

	memcpy(&temp16,&addr[0x2c],2);
	phnum=temp16;
	if (debug) printf("phnum %x\n",temp16);

	memcpy(&temp16,&addr[0x2e],2);
	shsize=temp16;
	if (debug) printf("shentsize %x\n",temp16);

	memcpy(&temp16,&addr[0x30],2);
	shnum=temp16;
	if (debug) printf("shnum %x\n",temp16);

	memcpy(&temp16,&addr[0x32],2);
	strindex=temp16;
	if (debug) printf("shstrndx %x\n",temp16);

	if (debug) {
		printf("%d ph entries starting at %x\n",phnum,phoff);

		printf("%d sh entries starting at %x, size %d\n",
			shnum,shoff,shsize);
	}

	shptr=&addr[shoff];

	/* get string pointer */
	string_pointer=&shptr[strindex*shsize];
	memcpy(&temp,&string_pointer[0x10],4);

	string_pointer=&addr[temp];

	/* get sizes */
	for(i=0;i<shnum;i++) {
		memcpy(&temp,&shptr[0x4],4);
		if (debug) {
			printf("\ttype: ");
			print_type_name(temp);
			printf("\n");
		}

		if (temp==SHT_PROGBITS) {
			if (debug) printf("Section header %d\n",i);

			memcpy(&temp,&shptr[0x0],4);
			name=string_pointer+temp;

			/* Don't write out comment */
			if (!strcmp(name,".comment")) {
			}
			else if (!strncmp(name,".text",6)) {
				memcpy(&temp,&shptr[0x10],4);
				offset=temp;

				memcpy(&temp,&shptr[0x14],4);
				text_size=temp;

				if (debug) {
					printf("\ttext_start: %x\n",text_start);
					printf("\ttext_size: %x\n",text_size);
				}
			}
			else if (!strncmp(name,".rodata",6)) {
				memcpy(&temp,&shptr[0x10],4);
				offset=temp;

				memcpy(&temp,&shptr[0x14],4);
				text_size=(offset-entry)+temp;

				if (debug) {
					printf("\trodata_start: %x\n",offset);
					printf("\ttext_size: %x\n",text_size);
				}
			}
			else if (!strncmp(name,".data",5)) {

				memcpy(&temp,&shptr[0x10],4);
				offset=temp;

				memcpy(&temp,&shptr[0x14],4);
				data_size=temp;

				data_start=offset-entry;
				if (debug) {
					printf("\tdata_start: %x, %x %d\n",
						data_start,offset,data_size);
				}

			}
			else {
				//printf("What to do with %s\n",name);
			}

		}

		/* Handle bss */
		if (temp==SHT_NOBITS) {
			if (debug) printf("Section header %d\n",i);

			memcpy(&temp,&shptr[0x0],4);
			name=string_pointer+temp;

			/* Don't write out comment */
			if (!strncmp(name,".bss",4)) {
				memcpy(&temp,&shptr[0x0c],4);
				offset=temp;
				bss_start=offset-entry;

				memcpy(&temp,&shptr[0x14],4);
				bss_size=temp;
				if (debug) printf("\tbss_start,size: %x %d\n",
					bss_start,bss_size);

			}
			else {
				printf("bss: what to do with %s\n",name);
			}

		}
		shptr+=shsize;

	}

	/***************************/
	/* Write out the bflt file */
	/***************************/

	text_start=0x40;
	if (data_start==0) {
		data_start=text_start+text_size;
	} else {
		data_start+=text_start;
	}

	if (bss_start==0) {
		bss_start=data_start+data_size;
		bss_end=data_start+data_size;
	}
	else {
		bss_start+=text_start;
		bss_end=bss_start+bss_size;
	}

	/* magic */
	write(out,"bFLT",4);

	/* version.  We're version 4 for now */
	temp=htonl(4);
	write(out,&temp,4);

	/* entry.  Entry after end of header */
	if (debug) printf("BFLT: text_start %x\n",text_start);
	temp=htonl(text_start);
	write(out,&temp,4);

	/* data_start */
	if (debug) printf("BFLT: data_start %x\n",data_start);
	temp=htonl(data_start);
	write(out,&temp,4);

	/* data_end */
	if (debug) printf("BFLT: data_end %x\n",bss_start);
	temp=htonl(bss_start);
	write(out,&temp,4);

	/* bss_end */
	if (debug) printf("BFLT: bss_end %x\n",bss_end);
	temp=htonl(bss_end);
	write(out,&temp,4);

	/* stack size */
	temp=htonl(8192);
	write(out,&temp,4);

	/* reloc start */
	temp=0;
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

	/* write out data */
	shptr=&addr[shoff];
	if (debug) printf("Writing data:\n");
	for(i=0;i<shnum;i++) {
		memcpy(&temp,&shptr[0x4],4);
		if (temp==SHT_PROGBITS) {
			if (debug) printf("Section header %d\n",i);

			memcpy(&temp,&shptr[0x0],4);
			name=string_pointer+temp;

			/* Don't write out comment */
			if (!strcmp(name,".comment")) {
			}
			else if ((!strncmp(name,".text",5)) ||
				 (!strncmp(name,".rodata",6))) {
				if (debug) printf("Writing text: %s!\n",name);

				memcpy(&temp,&shptr[0x10],4);
				offset=temp;
				if (debug) printf("\toffset: %x\n",offset);

				memcpy(&temp,&shptr[0x14],4);
				size=temp;
				if (debug) printf("\tsize: %x\n",size);

				if (debug) printf("Seeking to %x\n",
					text_start+offset-entry);
				lseek(out,text_start+offset-entry,SEEK_SET);
				write(out,&addr[offset],size);
			}
			else if (!strncmp(name,".data",5)) {
				if (debug) printf("Writing data: %s\n",name);

				memcpy(&temp,&shptr[0x10],4);
				offset=temp;
				if (debug) printf("\toffset: %x\n",offset);

				memcpy(&temp,&shptr[0x14],4);
				size=temp;
				if (debug) printf("\tsize: %x\n",size);

				if (debug) printf("Seeking to %x\n",
					text_start+offset-entry);
				lseek(out,text_start+offset-entry,SEEK_SET);
				write(out,&addr[offset],size);


			}
			else {
				printf("What to do with %s\n",name);
				return -1;
			}

		}

		shptr+=shsize;
	}



	munmap(addr, sb.st_size);

	close(fd);
	close(out);
	return 0;
}

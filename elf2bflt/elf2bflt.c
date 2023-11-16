#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
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

#define BFLT_FLAG_RAM		0x0001 /* load program entirely into RAM */
#define BFLT_FLAG_GOTPIC	0x0002 /* program is PIC with GOT */
#define BFLT_FLAG_GZIP		0x0004 /* all but the header is compressed */
#define BFLT_FLAG_GZDATA	0x0008 /* only data/relocs compressed (XIP) */
#define BFLT_FLAG_KTRACE	0x0010 /* ktrace debugging (not implemented) */
#define BFLT_FLAG_L1STK		0x0020 /* 4k stack in L1 (not imp)  */


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

static int is_relocated(uint32_t addr,uint32_t *relocations, int reloc_count) {

	int i;

	for(i=0;i<reloc_count;i++) {
		if (addr==relocations[i]) return 1;
	}
	return 0;
}

static uint32_t get_uint32(char *ptr) {
	uint32_t temp;

	memcpy(&temp,ptr,4);
	return temp;
}

static uint16_t get_uint16(char *ptr) {
	uint16_t temp;

	memcpy(&temp,ptr,2);
	return temp;
}


int main(int argc, char **argv) {

	int fd,out,i,j;
	struct stat sb;
	char *addr,*shptr,*string_pointer,*name;
	uint32_t temp;
	uint16_t temp16;

	uint32_t phoff,phnum;
	uint32_t shnum,shoff,shsize,strindex;

	uint32_t bss_size=0;
	uint32_t text_start=0,data_start=0,bss_start=0,bss_end=0,data_end=0;
	uint32_t reloc_start=0,reloc_count=0,reloc_index=0;
	uint32_t *relocations=NULL;
	uint32_t temp_addr,temp_type;
	uint32_t entry=0,size,offset,output_addr;
	uint32_t uses_got=0;

	uint32_t text_address=0,text_offset=0,text_size=0;
	uint32_t text_startup_address=0,text_startup_offset=0,text_startup_size=0;
	uint32_t rodata_address=0,rodata_offset=0,rodata_size=0;
	uint32_t data_address=0,data_offset,data_size=0;

	/* check command line arguments */
	if (argc<3) {
		print_usage(argv[0]);
		return 5;
	}

	/* open input file */
	fd=open(argv[1],O_RDONLY);
	if (fd<0) {
		printf("Error opening file %s\n",argv[1]);
		return 6;
	}

	/* open output file */
	out=open(argv[2],O_CREAT|O_TRUNC|O_WRONLY,0777);
	if (out<0) {
		printf("Error opening file %s\n",argv[2]);
		return 7;
	}

	/* run fstat to get file size */
	if (fstat(fd, &sb) < 0) {
		printf("Error with fstat\n");
		return 7;
	}

	/* mmap the file */
	addr=mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		printf("Error mmaping!\n");
		return 8;
	}

	/* check that it's an elf file */
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


	uint32_t flags,type;
	uint16_t ehsize,phentsize;

	entry=get_uint32(&addr[0x18]);	if (debug) printf("Entry %x\n",entry);
	phoff=get_uint32(&addr[0x1c]);	if (debug) printf("Phoff %x\n",phoff);
	shoff=get_uint32(&addr[0x20]);	if (debug) printf("Shoff %x\n",shoff);
	flags=get_uint32(&addr[0x24]);	if (debug) printf("Flags %x\n",flags);
	ehsize=get_uint16(&addr[0x28]);	if (debug) printf("ehsize %x\n",ehsize);
	phentsize=get_uint16(&addr[0x2a]); if (debug) printf("phentsize %x\n",phentsize);
	phnum=get_uint16(&addr[0x2c]);	if (debug) printf("phnum %x\n",phnum);
	shsize=get_uint16(&addr[0x2e]);	if (debug) printf("shentsize %x\n",shsize);
	shnum=get_uint16(&addr[0x30]);	if (debug) printf("shnum %x\n",shnum);


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

	/***************************/
	/* Get Sizes               */
	/***************************/
	/* FIXME: this is probably not doing the best job */

	for(i=0;i<shnum;i++) {
		if (debug) printf("Section header %d\n",i);

		type=get_uint32(&shptr[0x4]);
		if (debug) {
			printf("\ttype: ");
			print_type_name(type);
			printf("\n");
		}

		if (type==SHT_PROGBITS) {

			memcpy(&temp,&shptr[0x0],4);
			name=string_pointer+temp;

			/* Don't write out comment */
			if (!strcmp(name,".comment")) {
			}
			/* Don't write out debug info */
			else if (!strncmp(name,".debug",6)) {
			}
			else if (!strncmp(name,".text",6)) {
				text_address=get_uint32(&shptr[0x0c]);
				text_offset=get_uint32(&shptr[0x10]);
				text_size=get_uint32(&shptr[0x14]);
				if (debug) {
					printf("\t.text (0x%x) at 0x%x size 0x%x (%d)\n",
						text_address,text_offset,text_size,text_size);
				}
			}
			else if (!strncmp(name,".text.startup",13)) {
				text_startup_address=get_uint32(&shptr[0xc]);
				text_startup_offset=get_uint32(&shptr[0x10]);
				text_startup_size=get_uint32(&shptr[0x14]);
				if (debug) {
					printf("\t.text.startup (0x%x) at 0x%x size 0x%x (%d)\n",
						text_startup_address,
						text_startup_offset,
						text_startup_size,
						text_startup_size);
				}
			}
			else if (!strncmp(name,".rodata",6)) {
				rodata_address=get_uint32(&shptr[0x0c]);
				rodata_offset=get_uint32(&shptr[0x10]);
				rodata_size=get_uint32(&shptr[0x14]);

				if (debug) {
					printf("\t.rodata at 0x%x size %d\n",
						rodata_address,rodata_size);
				}
			}
			else if (!strncmp(name,".interp",7)) {
				memcpy(&temp,&shptr[0x10],4);
				offset=temp;

				memcpy(&temp,&shptr[0x14],4);
				text_size+=temp;

				if (debug) {
					printf("\t.interp at 0x%x size %d\n",
						offset,temp);
				}

			}
			else if (!strncmp(name,".data.rel.local",15)) {

				memcpy(&temp,&shptr[0x10],4);
				offset=temp;

				memcpy(&temp,&shptr[0x14],4);
				data_size+=temp;

				if (debug) {
					printf(".data.rel.local at 0x%x size %d (total %d)\n",
						offset,temp,data_size);
				}
			}
			else if (!strncmp(name,".data.rel.ro.local",18)) {

				memcpy(&temp,&shptr[0x10],4);
				offset=temp;

				memcpy(&temp,&shptr[0x14],4);
				data_size+=temp;

				if (debug) {
					printf("\t.data.rel.ro.local at 0x%x size %d (total %d)\n",
						offset,temp,data_size);
				}
			}
			else if (!strncmp(name,".got.plt",8)) {

				memcpy(&temp,&shptr[0x10],4);
				offset=temp;

				memcpy(&temp,&shptr[0x14],4);
				data_size+=temp;

				if (debug) {
					printf("\t.got.plt at 0x%x size %d (total %d)\n",
						offset,temp,data_size);
				}
			}
			else if (!strncmp(name,".got",4)) {
				memcpy(&temp,&shptr[0x10],4);
				offset=temp;

				memcpy(&temp,&shptr[0x14],4);
				data_size+=temp;

				uses_got=1;

				if (debug) {
					printf("\t.got at 0x%x size %d (total %d)\n",
						offset,temp,data_size);
				}

			}

/*
	typedef struct {
               uint32_t   sh_name;    0
               uint32_t   sh_type;    4
               uint32_t   sh_flags;   8
               Elf32_Addr sh_addr;    12
               Elf32_Off  sh_offset;  16 0x10
               uint32_t   sh_size;       0x14
               uint32_t   sh_link;
               uint32_t   sh_info;
               uint32_t   sh_addralign;
               uint32_t   sh_entsize;
           } Elf32_Shdr;
*/
			else if (!strncmp(name,".data",6)) {
				data_address=get_uint32(&shptr[0x0c]);
				data_offset=get_uint32(&shptr[0x10]);
				data_size=get_uint32(&shptr[0x14]);
				if (debug) {
					printf("\t.data [0x%x] at 0x%x size %x (%d)\n",
						data_address,data_offset,
						data_size,data_size);
				}

			}
			else {
				fprintf(stderr,"ERROR: what to do with %s\n",name);
				return 10;
			}

		}

		/* Handle bss */
		if (type==SHT_NOBITS) {
			if (debug) printf("Section header %d\n",i);

			memcpy(&temp,&shptr[0x0],4);
			name=string_pointer+temp;

			/* Don't write out comment */
			if (!strncmp(name,".bss",4)) {
				memcpy(&temp,&shptr[0x0c],4);
				offset=temp;
				bss_start=offset-entry;
				if (offset<entry) {
					fprintf(stderr,"Error! bss offset less than entry!\n");
					exit(1);
				}

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
	/* Create the Reloc Table  */
	/***************************/

	if (debug) printf("** CREATING RELOC TABLE **\n");

	shptr=&addr[shoff];

	for(i=0;i<shnum;i++) {

		memcpy(&temp,&shptr[0x4],4);

		if (temp==SHT_REL) {

			if (debug) printf("Section header %d\n",i);

			memcpy(&temp,&shptr[0x0],4);
			name=string_pointer+temp;

			if (!strncmp(name,".rel.dyn",8)) {
				memcpy(&temp,&shptr[0x10],4);
				offset=temp;

				memcpy(&temp,&shptr[0x14],4);
				size=temp;

				relocations=calloc(size/8,sizeof(uint32_t));
				if (relocations==NULL) {
					fprintf(stderr,"ERROR ALLOCATING\n");
					exit(6);
				}
				reloc_count=size/8;

				if (debug) {
					printf("\t.rel.dyn at 0x%x size %d\n",
						offset,size);
				}
				if (debug) printf("Trying up to %d\n",size/8);
				for(j=0;j<size;j+=8) {
					temp_addr=*(uint32_t *)(&addr[offset+j]);
					temp_type=*(uint32_t *)(&addr[offset+j]+4);
					if (temp_type!=0x17) {
						fprintf(stderr,"Unknown reloc type 0x%x\n",temp_type);
						exit(5);
					}
					relocations[reloc_index]=temp_addr;
					reloc_index++;

					if (debug) {
						printf("%d: %x %x\n",j,
							temp_addr,temp_type);
					}
				}

			}
		}
		shptr+=shsize;
	}

	/***************************/
	/* Write out the bflt file */
	/***************************/

	if (debug) printf("** WRITING BFLT FILE **\n");

	/* we skip the bflt header */
	text_start=0x40;

	uint32_t text_segment_end=0;

	if (text_start+text_size>text_segment_end) {
		text_segment_end=(text_start+text_size);
	}
	if ((text_startup_offset-text_offset)+text_startup_size>text_segment_end) {
		text_segment_end=(text_startup_offset-text_offset)+text_startup_size;
	}
	if ((rodata_offset-text_offset)+rodata_size>text_segment_end) {
		text_segment_end=(rodata_offset-text_offset)+rodata_size;
	}


	if (data_address==0) {
		data_start=text_start+text_segment_end;
		if (debug) printf("Calculating data start=text_start+text_segment_end=%x+%x\n",
			text_start,text_segment_end);
	} else {
		/* should this be offset instead? */
		data_start=text_start+(data_address-text_address);
		if (debug) printf("Calculating data start=text_start+(data_addr-text_addr)=%x-%x\n",
			data_address,text_address);
	}

	data_end=data_start+data_size;

	if (bss_start==0) {
		bss_start=data_start+data_size;
		bss_end=data_start+data_size;
	}
	else {
		bss_start+=text_start;
		bss_end=bss_start+bss_size;
	}

	if (reloc_count) {
		reloc_start=bss_start;
	}

	/*****************/
	/* Set up header */
	/*****************/

	/* magic */
	write(out,"bFLT",4);

	/* version.  We're version 4 for now */
	temp=htonl(4);	write(out,&temp,4);

	/* entry.  Entry after end of header */
	if (debug) printf("BFLT: text_start %x\n",text_start);
	temp=htonl(text_start);	write(out,&temp,4);

	/* data_start */
	if (debug) printf("BFLT: data_start %x\n",data_start);
	temp=htonl(data_start);	write(out,&temp,4);

	/* data_end */
	if (debug) printf("BFLT: data_end %x (%x+%d)\n",data_end,
		data_start,data_size);

	/* bss_start */
	if (debug) printf("BFLT: bss_start %x\n",bss_start);
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
	if (debug) printf("BFLT: reloc_start %x\n",reloc_start);
	temp=htonl(reloc_start);
	write(out,&temp,4);

	/* reloc count */
	if (debug) printf("BFLT: reloc_count %d\n",reloc_count);
	temp=htonl(reloc_count);
	write(out,&temp,4);

	/* flags */
	temp=0;
	if (uses_got) temp|=BFLT_FLAG_GOTPIC;
	write(out,&temp,4);

	/* padding */
	for(i=0;i<6;i++) {
		write(out,&temp,4);
	}

	/* write out data */
	shptr=&addr[shoff];
	if (debug) printf("Writing data:\n");
	for(i=0;i<shnum;i++) {
		if (debug) printf("SHNUM %d/%d\n",i,shnum);
		memcpy(&temp,&shptr[0x4],4);
		if (temp==SHT_PROGBITS) {
			if (debug) printf("Section header %d\n",i);

			memcpy(&temp,&shptr[0x0],4);
			name=string_pointer+temp;

			/* Don't write out comment */
			if (!strcmp(name,".comment")) {
				if (debug) printf("Skipping %s\n",name);
			}
			/* Don't write out debug info */
			else if (!strncmp(name,".debug",6)) {
				if (debug) printf("Skipping %s\n",name);
			}
			else if ((!strncmp(name,".text",5)) ||
				 (!strncmp(name,".rodata",6))) {
				if (debug) printf("Writing text: %s!\n",name);

				offset=get_uint32(&shptr[0x10]);
				if (debug) printf("\toffset: %x\n",offset);

				size=get_uint32(&shptr[0x14]);
				if (debug) printf("\tsize: %x\n",size);

				if (debug) printf("\tSeeking to %x\n",
					text_start+(offset-text_offset));
				lseek(out,text_start+(offset-text_offset),
					SEEK_SET);
				if (debug) printf("\tWriting %x bytes\n",
					size);
				write(out,&addr[offset],size);
			}
			else if (!strncmp(name,".interp",7)) {
				/* interpreter, probaby something like */
				/* /usr/lib/ld.so.1 */
				if (debug) printf("Writing useless data: %s\n",name);

				offset=get_uint32(&shptr[0x10]);
				if (debug) printf("\toffset: %x\n",offset);

				size=get_uint32(&shptr[0x14]);
				if (debug) printf("\tsize: %x\n",size);

				if (debug) printf("Seeking to %x\n",
					text_start+(offset-text_offset));
				lseek(out,text_start+(offset-text_offset),
					SEEK_SET);
				if (debug) printf("Writing %x\n",
					size);
				write(out,&addr[offset],size);
			}
			else if ((!strncmp(name,".data.rel.local",15)) ||
				(!strncmp(name,".data.rel.ro.local",18)) ||
				(!strncmp(name,".got.plt",8)) ||
				(!strncmp(name,".got",4)) ||
				(!strncmp(name,".data",5))) {

				if (debug) printf("Writing data: %s\n",name);

				offset=get_uint32(&shptr[0x10]);
				if (debug) printf("\toffset: %x\n",offset);

				size=get_uint32(&shptr[0x14]);
				if (debug) printf("\tsize: %x\n",size);

				output_addr=text_start+(offset-text_offset);

				if (debug) printf("\tSeeking to %x\n",
					output_addr);
				lseek(out,output_addr,SEEK_SET);

				if (debug) printf("\tWriting %x bytes\n",size);

				output_addr=offset;
				for(j=0;j<size;j+=4) {
					temp=*(uint32_t *)&addr[offset+j];
					if (debug) printf("TESTING RELOC %x\n",output_addr+j);
					if (is_relocated(output_addr+j,relocations,reloc_count)) {
						if (debug) printf("Relocating %x\n",output_addr+j);
						temp=temp-text_offset;
					}
					write(out,&temp,sizeof(uint32_t));
				}
			}
			else {
				fprintf(stderr,"What to do with %s\n",name);
				return -1;
			}

		}

		shptr+=shsize;
	}

	munmap(addr, sb.st_size);

	/* Write out relocations */

	/* FIXME: relocate GOT too */
	/* but... the got entries seem to appear in rel.dyn??? */
	if (reloc_count) {
		if (debug) printf("Writing %d relocations at %x\n",reloc_count,reloc_start);
		lseek(out,reloc_start,SEEK_SET);
		for(j=0;j<reloc_count;j++) {
			if (debug) printf("reloc %d: old %x new %x\n",
				j,relocations[j],relocations[j]-text_offset);
			temp_addr=htonl(relocations[j]-text_offset);
			write(out,&temp_addr,sizeof(uint32_t));
		}
	}

	if (relocations) free(relocations);

	close(fd);
	close(out);
	return 0;
}

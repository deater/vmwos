#include <stdio.h>
#include <string.h>

int debug=1;

#define SYMBOL_LENGTH	20
#define MAX_SYMBOLS	100

struct symbols_t {
	char name[SYMBOL_LENGTH];
	int offset;
} symbols[MAX_SYMBOLS];


static FILE *fff;
static int line=1;

void skip_to_end_of_line(void) {

	int ch;

	while(1) {
		ch=fgetc(fff);
		if (ch==EOF) break;
		if (ch=='\n') {
			line++;
			break;
		}
	}
}

int main(int argc, char **argv) {


	int ch;
	char buffer[BUFSIZ];
	int x;

	int next_symbol=0;
	int offset=0;


	if (argc<2) {
		fprintf(stderr,"Error, need argument\n");
		return -1;
	}

	fff=fopen(argv[1],"r");
	if (fff==NULL) {
		fprintf(stderr,"Error, could not open %s\n",argv[1]);
		return -1;
	}


	/* First Pass */
	while(1) {

		ch=fgetc(fff);
		if (ch==EOF) break;

		if ((ch==' ') || (ch=='\t') || (ch=='\r')) continue;
		if (ch=='\n') {
			line++;
			continue;
		}

		if ((ch=='#') || (ch=='@')) {
			while(1) {
				ch=fgetc(fff);
				if (ch==EOF) break;
				if (ch=='\n') {
					line++;
					break;
				}
			}
			continue;
		}
		if (ch=='.') {
			printf("Directive: ");
			while(1) {
				ch=fgetc(fff);
				if (ch==EOF) break;
				if (ch=='\n') {
					line++;
					break;
				}
				printf("%c",ch);
			}
			printf("\n");
			continue;
		}

		x=0;
		while(1) {
			buffer[x]=ch;
			x++;
			if (ch==':') {
				buffer[x]=0;
				//printf("Label: %s\n",buffer);
				if (strlen(buffer)>SYMBOL_LENGTH) {
					fprintf(stderr,"ERROR! Symbol %s too long at line %d\n",buffer,line);
				}
				strncpy(symbols[next_symbol].name,buffer,
					SYMBOL_LENGTH);
				symbols[next_symbol].offset=offset;
				next_symbol++;
				if (next_symbol>=MAX_SYMBOLS) {
					fprintf(stderr,"ERROR!  Too many symbols\n");
				}
				break;
			}
			if ((ch==' ')||(ch=='\t')) {
				buffer[x]=0;
				printf("%x:\t%s\n",offset,buffer);
				offset+=4;
				break;
			}
			ch=fgetc(fff);
			if (ch==EOF) break;

		}

//		printf("%c",ch);

	}

	rewind(fff);
	offset=0;
	line=1;

	/* Second Pass */
	while(1) {

		ch=fgetc(fff);
		if (ch==EOF) break;

		if ((ch==' ') || (ch=='\t') || (ch=='\r')) continue;
		if (ch=='\n') {
			line++;
			continue;
		}

		if ((ch=='#') || (ch=='@')) {
			skip_to_end_of_line();
			continue;
		}

		if (ch=='/') {
			ch=fgetc(fff);
			if (ch=='/') {
				skip_to_end_of_line();
			}
			continue;
		}

		if (ch=='.') {
			printf("Directive: ");
			while(1) {
				ch=fgetc(fff);
				if (ch==EOF) break;
				if (ch=='\n') {
					line++;
					break;
				}
				printf("%c",ch);
			}
			printf("\n");
			continue;
		}

		x=0;
		while(1) {

			buffer[x]=ch;
			x++;
			if (ch==':') {
				buffer[x-1]=0;
				printf("%08x: <%s>:\n",offset,buffer);
				if (strlen(buffer)>SYMBOL_LENGTH) {
					fprintf(stderr,"ERROR! Symbol %s too long at line %d\n",buffer,line);
				}
				strncpy(symbols[next_symbol].name,buffer,
					SYMBOL_LENGTH);
				symbols[next_symbol].offset=offset;
				next_symbol++;
				if (next_symbol>=MAX_SYMBOLS) {
					fprintf(stderr,"ERROR!  Too many symbols\n");
				}
				break;
			}

			if (ch=='/') {
				if (buffer[x-1]=='/') {
					buffer[x-1]=0;
					printf("%4x:\t%08x\t%s\n",offset,0,buffer);
					offset+=4;
					skip_to_end_of_line();
					break;
				}
			}

			if (ch=='@') {
				buffer[x-1]=0;
				printf("%4x:\t%08x\t%s\n",offset,0,buffer);
				offset+=4;
				skip_to_end_of_line();
				break;
			}

			if (ch=='\n') {
				buffer[x-1]=0;
				printf("%4x:\t%08x\t%s\n",offset,0,buffer);
				offset+=4;
				line++;
//				skip_to_end_of_line();
				break;
			}

			ch=fgetc(fff);
			if (ch==EOF) break;

		}

//		printf("%c",ch);

	}

	return 0;
}

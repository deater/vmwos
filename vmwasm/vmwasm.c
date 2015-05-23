#include <stdio.h>

int debug=1;

int main(int argc, char **argv) {

	FILE *fff;
	int ch;
	char buffer[BUFSIZ];
	int x;

	if (argc<2) {
		fprintf(stderr,"Error, need argument\n");
		return -1;
	}

	fff=fopen(argv[1],"r");
	if (fff==NULL) {
		fprintf(stderr,"Error, could not open %s\n",argv[1]);
		return -1;
	}

	while(1) {

		ch=fgetc(fff);
		if (ch==EOF) break;

		if ((ch==' ') || (ch=='\t') || (ch=='\n') || (ch=='\r')) continue;

		if ((ch=='#') || (ch=='@')) {
			while(1) {
				ch=fgetc(fff);
				if (ch==EOF) break;
				if (ch=='\n') break;
			}
			continue;
		}
		if (ch=='.') {
			printf("Directive: ");
			while(1) {
				ch=fgetc(fff);
				if (ch==EOF) break;
				if (ch=='\n') break;
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
				printf("Label: %s\n",buffer);
				break;
			}
			if ((ch==' ')||(ch=='\t')) {
				buffer[x]=0;
				printf("Instruction: %s\n",buffer);
				break;
			}
			ch=fgetc(fff);
			if (ch==EOF) break;

		}

//		printf("%c",ch);

	}

	return 0;
}

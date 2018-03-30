#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/vfs.h>
#endif

static int debug=0;

static int prettyprint(uint64_t size) {

	if (size<1024) {
		printf("%dB",(uint32_t)size);
	}
	else if (size<1024*1024) {
		printf("%dk",(uint32_t)(size/1024));
	}
	else if (size<1024*1024*1024) {
		printf("%dM",(uint32_t)(size/(1024*1024)));
	}
	else if (size<1024ULL*1024ULL*1024ULL*1024ULL) {
		printf("%dG",(uint32_t)(size/(1024*1024*1024)));
	}
	else if (size<1024ULL*1024ULL*1024ULL*1024ULL*1024ULL) {
		printf("%dT",(uint32_t)(size/(1024ULL*1024ULL*1024ULL*1024ULL)));
	}
	else printf("HUGE!\n");

	return 0;
}

static int df(char *device,char *mountpoint) {

	struct statfs buf;
	uint32_t blocksize,totalblocks,freeblocks;
	uint64_t totalsize,totalfree,totalused;

	if (debug) printf("df: running with %s %s\n",device,mountpoint);

	statfs(mountpoint, &buf);

	blocksize=buf.f_bsize;
	totalblocks=buf.f_blocks;
	freeblocks=buf.f_bfree;

	totalsize=(uint64_t)blocksize*(uint64_t)totalblocks;
	totalfree=(uint64_t)blocksize*(uint64_t)freeblocks;
	totalused=totalsize-totalfree;


	if (debug) {
		printf("Totalsize = %d*%d = %ld\n",
			blocksize,totalblocks,totalsize);
	}

	printf("%s\t",device);
	prettyprint(totalsize);
	printf("\t");
	prettyprint(totalused);
	printf("\t");
	prettyprint(totalfree);

	if (totalblocks==0) {
		printf("\t100%%");
	}
	else {
		printf("\t%3d%%",100-(freeblocks/totalblocks));
	}

	printf("\t%s\n",mountpoint);

	return 0;

}

int main(int argc, char **argv) {

	FILE *fff;
	char *result,*start,*device;
	char buffer[256];
	uint32_t i,j;

//Filesystem      Size  Used Avail Use% Mounted on
///dev/sda4       114G  112G  1.4G  99% /

	printf("Filesystem\tSize\tUsed\tAvail\tUse%%\tMounted on\n");


	fff=fopen("/etc/fstab","r");
	if (fff==NULL) {
		if (debug) printf("df: Couldn't open fstab!\n");
		df("/","/");
		return 0;
	}

	while(1) {
		result=fgets(buffer,256,fff);

		if (debug) {
			printf("fgets result: %x\n",result);
			if (result!=NULL) printf("fgets result: %s\n",result);
		}

		if (result==NULL) break;
		if (result[0]=='#') continue;

		// sscanf(result,"%*s %s",blah);
		i=0;

		/* skip leading spaces */
		while((result[i]!=0)&&(result[i]!='\n')&&
			((result[i]==' ')||(result[i]=='\t'))) {
			i++;
		}
		if ((result[i]==0) || (result[i]=='\n')) continue;

		device=&result[i];

		/* skip until spaces */
		while((result[i]!=0)&&(result[i]!='\n')&&
			(result[i]!=' ')&&(result[i]!='\t')) {
			i++;
		}
		if (result[i]==0) continue;

		j=i;

		/* skip more spaces */
		while((result[i]!=0)&&(result[i]!='\n')&&
			((result[i]==' ')||(result[i]=='\t'))) {
			i++;
		}
		if ((result[i]==0) || (result[i]=='\n')) continue;


		result[j]=0;
		start=&result[i];

		/* skip until spaces */
		while((result[i]!=0)&&(result[i]!='\n')&&
			(result[i]!=' ')&&(result[i]!='\t')) {
			i++;
		}
		if (result[i]==0) continue;

		result[i]=0;

		df(device,start);

	}
	fclose(fff);

	return 0;
}

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

//#define DEBUG_STANDALONE 1

#ifdef DEBUG_STANDALONE
#include <unistd.h>
struct file_struct {
        int fd;
        int eof;
};
typedef struct file_struct FILE;
#else
#include "syscalls.h"
#include "vmwos.h"
#include "vlibc.h"
#endif

#define MAX_PRINT_SIZE 256


static int pad(char *buffer, int *buffer_pointer, int max_print_size,
		int pad_len, int printed_size, char pad_value) {

	int i;

	if (pad_len>printed_size) {
		for(i=0;i<pad_len-printed_size;i++) {
			buffer[*buffer_pointer]=pad_value;
			(*buffer_pointer)++;
			if (*buffer_pointer==max_print_size) {
				return -1;
			}
		}
	}

	return 0;
}

static int append_buffer(char *input, int len, int max_print_size,
		char *buffer, int *buffer_pointer) {

	int i;

	for(i=0;i<len;i++) {
		buffer[*buffer_pointer]=input[i];
		(*buffer_pointer)++;
		if (*buffer_pointer==max_print_size) {
			return -1;
		}
	}
	return 0;
}

static int32_t convert_decimal32(uint32_t x, int32_t is_signed,
				char *int_buffer,char **out_pointer) {

	int int_pointer;
	int printed_size=0;
	int negative=0;

	/* See if negative */
	if ((is_signed) && (x&0x80000000)) {
		negative=1;
		/* twos complement */
		x=~x;
		x++;
	}

	int_pointer=32;
	int_buffer[int_pointer]='\0';
	int_pointer--;

	do {
		int_buffer[int_pointer]=(x%10)+'0';
		int_pointer--;
		printed_size++;
		x/=10;
	} while(x!=0);

	if (negative) {
		int_buffer[int_pointer]='-';
		int_pointer--;
		printed_size++;
	}

	*out_pointer=&int_buffer[int_pointer+1];

	return printed_size;
}

static int32_t convert_hex32(uint32_t x, char *int_buffer,char **out_pointer) {

	int int_pointer;
	int printed_size=0;

	int_pointer=31;
	int_buffer[int_pointer]='\0';
	int_pointer--;

	do {
		if ((x%16)<10) {
			int_buffer[int_pointer]=(x%16)+'0';
		}
		else {
			int_buffer[int_pointer]=(x%16)-10+'a';
		}
		int_pointer--;
		printed_size++;
		x/=16;
	} while(x!=0);

	*out_pointer=&int_buffer[int_pointer+1];

	return printed_size;
}

static int32_t convert_decimal64(uint64_t x, int32_t is_signed,
				char *int_buffer,char **out_pointer) {

	int int_pointer;
	int printed_size=0;
	int negative=0;

	/* See if negative */
	if ((is_signed) && (x&0x8000000000000000ULL)) {
		negative=1;
		/* twos complement */
		x=~x;
		x++;
	}

	int_pointer=31;
	int_buffer[int_pointer]='\0';
	int_pointer--;

	do {
		int_buffer[int_pointer]=(x%10)+'0';
		int_pointer--;
		printed_size++;
		x/=10;
	} while(x!=0);

	if (negative) {
		int_buffer[int_pointer]='-';
		int_pointer--;
		printed_size++;
	}

	*out_pointer=&int_buffer[int_pointer+1];

	return printed_size;
}

static int32_t convert_hex64(uint64_t x, char *int_buffer,char **out_pointer) {

	int int_pointer=0;
	int printed_size=0;

	int_pointer=18;
	int_buffer[int_pointer]='\0';
	int_pointer--;

	do {
		if ((x%16)<10) {
			int_buffer[int_pointer]=(x%16)+'0';
		}
		else {
			int_buffer[int_pointer]=(x%16)-10+'a';
		}
		int_pointer--;
		printed_size++;
		x/=16;
	} while(x!=0);

	*out_pointer=&int_buffer[int_pointer+1];

	return printed_size;
}


static int vsnprintf_internal(char *buffer, uint32_t size,
				const char *string, va_list ap) {

	char int_buffer[32];
	char *out_pointer;
	int precision=0;
	int result;

	int buffer_pointer=0;
	uint32_t x;
	uint64_t lx;
	char pad_value,pad_len,printed_size;
	int max_print_size=size;

	while(1) {
		if (*string==0) break;

		if (*string=='%') {
			string++;

			pad_len=0;
			pad_value=' ';
			printed_size=0;

			/* Precision: FIXME */
			if (*string=='.') {
				string++;
				precision=1;
				(void)precision;
			}

			/* Padding */
			if ((*string>='0') && (*string<='9')) {
				if (*string=='0') pad_value='0';
				else pad_value=' ';
				pad_len=*string-'0';
				string++;
				while((*string>='0') && (*string<='9')) {
					pad_len*=10;
					pad_len+=*string-'0';
					string++;
				}
			}

			/* Signed integer */
			if ((*string=='d') || (*string=='i')) {
				string++;
				/* get 32-bit value */
				x=va_arg(ap, uint32_t);
				printed_size=convert_decimal32(x,1,int_buffer,
							&out_pointer);
				result=pad(buffer,&buffer_pointer,
						max_print_size,
						pad_len, printed_size,
						pad_value);
				if (result<0) break;
				result=append_buffer(out_pointer,printed_size,
						max_print_size,
						buffer,&buffer_pointer);
				if (result<0) break;
			}
			/* Unigned integer */
			else if (*string=='u') {
				string++;
				/* get 32-bit value */
				x=va_arg(ap, uint32_t);
				printed_size=convert_decimal32(x,0,int_buffer,
							&out_pointer);
				result=pad(buffer,&buffer_pointer,
						max_print_size,
						pad_len, printed_size,
						pad_value);
				if (result<0) break;
				result=append_buffer(out_pointer,printed_size,
						max_print_size,
						buffer,&buffer_pointer);
				if (result<0) break;
			}

			/* long long x (FIXME!) */
			else if (*string=='l') {
				string++;
				if (*string!='l'); // FIXME: indicate error
				string++;
				if (*string=='x') {
					string++;
					lx=va_arg(ap, uint64_t);
					printed_size=convert_hex64(lx,int_buffer,
								&out_pointer);
					result=pad(buffer,&buffer_pointer,
							max_print_size,
							pad_len, printed_size,
							pad_value);
					if (result<0) break;
					result=append_buffer(out_pointer,printed_size,
							max_print_size,
							buffer,&buffer_pointer);
					if (result<0) break;
				}
				else if (*string=='d') {
					string++;
					lx=va_arg(ap, uint64_t);
					printed_size=convert_decimal64(lx,1,int_buffer,
								&out_pointer);
					result=pad(buffer,&buffer_pointer,
							max_print_size,
							pad_len, printed_size,
							pad_value);
					if (result<0) break;
					result=append_buffer(out_pointer,printed_size,
							max_print_size,
							buffer,&buffer_pointer);
					if (result<0) break;
				}
				else if (*string=='u') {
					string++;
					lx=va_arg(ap, uint64_t);
					printed_size=convert_decimal64(lx,0,int_buffer,
								&out_pointer);
					result=pad(buffer,&buffer_pointer,
							max_print_size,
							pad_len, printed_size,
							pad_value);
					if (result<0) break;
					result=append_buffer(out_pointer,printed_size,
							max_print_size,
							buffer,&buffer_pointer);
					if (result<0) break;
				}
			}
			/* Hex */
			else if ((*string=='x') || (*string=='p')) {
				string++;
				/* get 32-bit value */
				x=va_arg(ap, uint32_t);
				printed_size=convert_hex32(x,int_buffer,
							&out_pointer);
				result=pad(buffer,&buffer_pointer,
						max_print_size,
						pad_len, printed_size,
						pad_value);
				if (result<0) break;
				result=append_buffer(out_pointer,printed_size,
						max_print_size,
						buffer,&buffer_pointer);
				if (result<0) break;

			}
			/* char */
			else if (*string=='c') {
				string++;
				x=va_arg(ap, unsigned long);
				buffer[buffer_pointer]=x;
				buffer_pointer++;
				if (buffer_pointer==max_print_size) break;
			}
			/* %% */
			else if (*string=='%') {
				string++;
				buffer[buffer_pointer]='%';
				buffer_pointer++;
				if (buffer_pointer==max_print_size) break;
			}
			/* string */
			else if (*string=='s') {
				char *s;
				string++;
				s=(char *)va_arg(ap, long);
				while(*s) {
					buffer[buffer_pointer]=*s;
					s++;
					buffer_pointer++;
					if (buffer_pointer==
						max_print_size) break;
				}
			}
		}
		else {
			buffer[buffer_pointer]=*string;
			buffer_pointer++;
			if (buffer_pointer==max_print_size) break;
			string++;
		}
		if (buffer_pointer==max_print_size-1) break;
	}

	if (buffer_pointer>=max_print_size) {
		buffer_pointer=max_print_size-1;
	}
	buffer[buffer_pointer]='\0';

	return buffer_pointer;
}

int vsprintf(char *buffer, const char *string, ...) {

	int result;

	va_list argp;
	va_start(argp, string);

	result=vsnprintf_internal(buffer,MAX_PRINT_SIZE,string,argp);

	va_end(argp);

        return result;

}

int vsnprintf(char *buffer, uint32_t size, const char *string, va_list argp) {

	int result;

	result=vsnprintf_internal(buffer,size,string,argp);

        return result;

}

int printf(const char *string,...) {

	char buffer[MAX_PRINT_SIZE];
	int result;

	va_list argp;
	va_start(argp, string);

	result=vsnprintf_internal(buffer,MAX_PRINT_SIZE,string,argp);

	va_end(argp);

	write(1,buffer,result);

        return result;

}

int sprintf(char *string, char *fmt, ...) {

	int result;

	va_list argp;
	va_start(argp, fmt);

	result=vsnprintf_internal(string,MAX_PRINT_SIZE,fmt,argp);

	va_end(argp);

	/* NUL terminate */
	string[result]='\0';

	return result;

}

int snprintf(char *string, uint32_t size, const char *fmt, ...) {

	int result;

	va_list argp;
	va_start(argp, fmt);

	result=vsnprintf_internal(string,size,fmt,argp);

	va_end(argp);

	/* NUL terminate */
	string[result]='\0';

	return result;

}


int fprintf(FILE *stream, const char *string, ...) {

	char buffer[MAX_PRINT_SIZE];
	int result;

	va_list argp;
	va_start(argp, string);

	result=vsnprintf_internal(buffer,MAX_PRINT_SIZE,string,argp);

	va_end(argp);

	write(stream->fd,buffer,result);

	return result;

}

#if DEBUG_STANDALONE

int main(int argc, char **argv) {

	char *p = (char *)0xdeadbeef;

	printf("Hello World!\n");
	printf("Trying 12345 with d:\t%d\n",12345);
	printf("Trying 12345 with i:\t%i\n",12345);
	printf("Trying 12345 54321 with d i:\t%d %i\n",12345,54321);
	printf("Trying -12345 with d:\t%d\n",-12345);
	printf("Trying -12345 with i:\t%i\n",-12345);
	printf("Trying 12345 12345 with d u:\t%d %u\n",12345,12345);
	printf("Trying -1 -1 with d u:\t%d %u\n",-1,-1);

	printf("Trying 0xdeadbeef with x p:\t%x %p\n",0xdeadbeef,p);
	printf("Trying 0x13feb78 with x:\t%x\n",0x13feb78);
	printf("Trying c 65 (A):\t%c\n",65);
	printf("Trying s HELLO WORLD:\t%s\n","HELLO WORLD");

	printf("Trying %%5x padding, should be *  b0b*:\t*%5x*\n",0xb0b);

	printf("Trying %%5d padding, should be *  123*:\t*%5d*\n",123);

	printf("Trying %%05x padding, should be *00b0b*:\t*%05x*\n",0xb0b);

	printf("Trying %%05d padding, should be *00123*:\t*%05d*\n",123);

	printf("Trying %%llx, should be 0xdeadbeefcafebabe:\t%llx\n",
		0xdeadbeefcafebabeULL);
	printf("Trying %%lld, should be -1 123456789012345:\t%lld %lld\n",
		-1LL,123456789012345LL);
	printf("Trying %%llu, should be 18446744073709551615 123456789012345:\t%llu %llu\n",
		-1LL,123456789012345LL);

	return 0;

}
#endif

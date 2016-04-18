include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	lib.a

lib.a:	div.o memcpy.o memory_benchmark.o memset.o printk.o string.o
	$(CROSS)$(AR) crs lib.a div.o memcpy.o memory_benchmark.o memset.o \
		printk.o string.o

div.o:	div.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o div.o -c div.c

memcpy.o:	memcpy.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o memcpy.o -c memcpy.c

memory_benchmark.o:	memory_benchmark.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o memory_benchmark.o -c memory_benchmark.c

memset.o:	memset.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o memset.o -c memset.c

printk.o:	printk.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o printk.o -c printk.c

string.o:	string.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o string.o -c string.c

clean:
	rm -f *~ *.o *.a


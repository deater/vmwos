CC = gcc
CFLAGS = -Wall -O2 -g -I../kernel/include -nostartfiles -ffreestanding -nostdlib
LFLAGS =

CFLAGS_TEST = -Wall -O2 -g
LFLAGS_TEST =

all:	fs.a test_dev test_df

######

test_df:	test_df.o fs.a test_glue.o
	$(CC) -o test_df test_df.o $(LFLAGS_TEST) fs.a test_glue.o

test_df.o:	test_df.c
	$(CC) $(CFLAGS_TEST) -c test_df.c


######

test_glue.o:	test_glue.c
	$(CC) $(CFLAGS_TEST) -c test_glue.c

######

test_dev:	test_dev.o fs.a
	$(CC) -o test_dev test_dev.o $(LFLAGS_TEST) fs.a

test_dev.o:	test_dev.c
	$(CC) $(CFLAGS_TEST) -c test_dev.c

######

fs.a:	files.o \
	romfs/romfs.a
	$(AR) crsT fs.a \
		files.o \
		romfs/romfs.a

files.o:	files.c
	$(CC) $(CFLAGS) -o files.o -c files.c

romfs/romfs.a:	force_look
	cd romfs && make

####

force_look:
	true


clean:
	rm -f *~ *.o *.a
	cd romfs && make clean

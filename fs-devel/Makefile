CC = gcc

CFLAGS = -Wall -O2 -g
LFLAGS =

FSA_LOC = ../kernel/fs/fs-armv7.a

all:	test_df test_ls test_cat

######

test_cat:	test_cat.o test_glue.o
	$(CC) -o test_cat test_cat.o $(LFLAGS) test_glue.o $(FSA_LOC)

test_cat.o:	test_cat.c
	$(CC) $(CFLAGS) -c test_cat.c

######

test_df:	test_df.o test_glue.o
	$(CC) -o test_df test_df.o $(LFLAGS) $(FSA_LOC) test_glue.o

test_df.o:	test_df.c
	$(CC) $(CFLAGS) -c test_df.c

######

test_ls:	test_ls.o test_glue.o
	$(CC) -o test_ls test_ls.o $(LFLAGS) test_glue.o $(FSA_LOC)

test_ls.o:	test_ls.c
	$(CC) $(CFLAGS) -c test_ls.c



######

test_glue.o:	test_glue.c
	$(CC) $(CFLAGS) -c test_glue.c

######

test_dev:	test_dev.o fs.a
	$(CC) -o test_dev test_dev.o $(LFLAGS_TEST) fs.a

test_dev.o:	test_dev.c
	$(CC) $(CFLAGS_TEST) -c test_dev.c

clean:
	rm -f *~ *.o *.a test_cat test_df test_ls


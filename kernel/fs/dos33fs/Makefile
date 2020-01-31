include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	dos33fs-arm1176.a dos33fs-armv7.a

### ARM1176

dos33fs-arm1176.a:	dos33fs.arm1176.o
	$(CROSS)$(AR) crs dos33fs-arm1176.a dos33fs.arm1176.o

dos33fs.arm1176.o:	dos33fs.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o dos33fs.arm1176.o -c dos33fs.c

### ARMV7

dos33fs-armv7.a:	dos33fs.armv7.o
	$(CROSS)$(AR) crs dos33fs-armv7.a dos33fs.armv7.o

dos33fs.armv7.o:	dos33fs.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o dos33fs.armv7.o -c dos33fs.c

clean:
	rm -f *~ *.o *.a


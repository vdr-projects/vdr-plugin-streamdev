INCS   = -I.
CFLAGS =  -g -Wall -O2 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -fPIC
MFLAG  = -M
OBJS = ctools.o ringbuffy.o remux.o transform.o
SRC  = $(wildcard *.c)

DESTDIR = /usr/local

.PHONY: clean

clean:
	- rm -f *.o  *~ *.a .depend

libdvbmpegtools.a: $(OBJS)
	ar -rcs libdvbmpegtools.a $(OBJS) 

%.o:    %.c 
	$(CC) -c $(CFLAGS) $(INCS) $(DEFINES) $<

.depend: 
	$(CXX) $(DEFINES) $(MFLAG) $(SRC) $(INCS)> .depend



-include .depend

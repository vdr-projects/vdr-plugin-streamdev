INCS   = -I.
CFLAGS =  -g -Wall -O2 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -fPIC
MFLAG  = -M
OBJS = ctools.o ringbuffy.o remux.o transform.o cpptools.o
SRC  = $(wildcard *.c)
CPPSRC = $(wildcard *.cpp)
CSRC = $(wildcard *.cc) 

DESTDIR = /usr/local

.PHONY: depend clean install uninstall

clean:
	- rm -f *.o  *~ *.a .depend

libdvbmpegtools.a: $(OBJS)
	ar -rcs libdvbmpegtools.a $(OBJS) 

%.o:    %.cc 
	$(CXX) -c $(CFLAGS) $(INCS) $(DEFINES) $<

%.o:    %.cpp
	$(CXX) -c $(CFLAGS) $(INCS) $(DEFINES) $<

%.o:    %.c 
	$(CC) -c $(CFLAGS) $(INCS) $(DEFINES) $<

.depend: 
	$(CXX) $(DEFINES) $(MFLAG) $(SRC) $(CSRC) $(CPPSRC) $(INCS)> .depend



-include .depend

#
# Makefile for a Video Disk Recorder plugin
#
# $Id: Makefile,v 1.1.2.1 2010/06/14 10:40:18 schmirl Exp $

### The object files (add further files here):

OBJS = tsremux.o ts2es.o ts2pes.o ts2ps.o extern.o

### The main target:

.PHONY: clean
remux.a: $(OBJS)
	ar -rcs remux.a $^

### Implicit rules:

%.o: %.c 
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies

$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

### Targets:

clean:
	@-rm -f $(OBJS) $(DEPFILE) *.a core* *~

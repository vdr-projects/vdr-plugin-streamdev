#
# Makefile for a Video Disk Recorder plugin
#

### The object files (add further files here):

OBJS = tsremux.o ts2es.o ts2pes.o ts2ps.o extern.o

### The main target:

.PHONY: clean
remux.a: $(OBJS)
	@echo AR remux/ $@
	$(Q)$(AR) -rcs remux.a $^

### Implicit rules:

%.o: %.c 
	@echo CC remux/ $@
	$(Q)$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies

$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

### Targets:

clean:
	@-rm -f $(OBJS) $(DEPFILE) *.a core* *~

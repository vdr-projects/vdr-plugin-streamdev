#
# Makefile for a Video Disk Recorder plugin
#
# $Id: Makefile,v 1.2 2010/07/19 13:49:44 schmirl Exp $

### The object files (add further files here):

OBJS = select.o socket.o source.o tools.o

### The main target:

.PHONY: clean
sockettools.a: $(OBJS)
	@echo AR tools/ $@
	$(Q)$(AR) -rcs sockettools.a $(OBJS) 

### Implicit rules:

%.o: %.c 
	@echo CC tools/ $@
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

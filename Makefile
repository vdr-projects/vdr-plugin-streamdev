#
# Makefile for a Video Disk Recorder plugin
#
# $Id: Makefile,v 1.5 2005/04/24 16:19:14 lordjaxom Exp $

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
#
PLUGIN = streamdev

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'const char \*VERSION *=' common.c | awk '{ print $$5 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX      ?= g++
CXXFLAGS ?= -W -Woverloaded-virtual

### The directory environment:

DVBDIR = ../../../../DVB
VDRDIR = ../../..
LIBDIR = ../../lib
TMPDIR = /tmp

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

### The version number of VDR (taken from VDR's "config.h"):

VDRVERSION = $(shell grep 'define VDRVERSION ' $(VDRDIR)/config.h | awk '{ print $$3 }' | sed -e 's/"//g')

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include -I$(DVBDIR)/include -I.

DEFINES += -D_GNU_SOURCE

### The object files (add further files here):

COMMONOBJS = common.o i18n.o \
	\
	tools/source.o tools/select.o tools/socket.o tools/tools.o

CLIENTOBJS = $(PLUGIN)-client.o \
	\
	client/socket.o client/device.o client/setup.o \
	client/remote.o client/assembler.o client/filter.o


SERVEROBJS = $(PLUGIN)-server.o \
	\
	server/server.o server/connectionVTP.o server/connectionHTTP.o \
	server/componentHTTP.o server/componentVTP.o server/connection.o \
	server/component.o server/suspend.o server/setup.o server/streamer.o \
	server/livestreamer.o server/livefilter.o \
	\
	remux/tsremux.o remux/ts2ps.o remux/ts2es.o remux/extern.o
	
ifdef DEBUG
	DEFINES += -DDEBUG
	CXXFLAGS += -g
else
	CXXFLAGS += -O2
endif

ifeq ($(shell test -f $(VDRDIR)/fontsym.h ; echo $$?),0)
  DEFINES += -DHAVE_BEAUTYPATCH
endif

ifeq ($(shell test -f $(VDRDIR)/fontsym.c ; echo $$?),0)
  DEFINES += -DHAVE_BEAUTYPATCH
endif

# HAVE_AUTOPID only applies if VDRVERSNUM < 10300
ifeq ($(shell test -f $(VDRDIR)/sections.c ; echo $$?),0)
  DEFINES += -DHAVE_AUTOPID
endif

libdvbmpeg/libdvbmpegtools.a: libdvbmpeg/*.c libdvbmpeg/*.cc libdvbmpeg/*.h libdvbmpeg/*.hh
	make -C ./libdvbmpeg libdvbmpegtools.a

### Implicit rules:

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

# Dependencies:

MAKEDEP = g++ -MM -MG
DEPFILE = .dependencies
ifdef GCC3
$(DEPFILE): Makefile
	@rm -f $@
	@for i in $(CLIENTOBJS:%.o=%.c) $(SERVEROBJS:%.o=%.c) $(COMMONOBJS:%.o=%.c) ; do \
		$(MAKEDEP) $(DEFINES) $(INCLUDES) -MT "`dirname $$i`/`basename $$i .c`.o" $$i >>$@ ; \
	done
else
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(CLIENTOBJS:%.o=%.c) $(SERVEROBJS:%.o=%.c) \
			$(COMMONOBJS:%.o=%.c) > $@
endif

-include $(DEPFILE)

### Targets:

all: libvdr-$(PLUGIN)-client.so libvdr-$(PLUGIN)-server.so

libvdr-$(PLUGIN)-client.so: $(CLIENTOBJS) $(COMMONOBJS) libdvbmpeg/libdvbmpegtools.a
libvdr-$(PLUGIN)-server.so: $(SERVEROBJS) $(COMMONOBJS) libdvbmpeg/libdvbmpegtools.a

%.so: 
	$(CXX) $(CXXFLAGS) -shared $^ -o $@
	@cp $@ $(LIBDIR)/$@.$(VDRVERSION)

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz --exclude SCCS -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	@-rm -f $(COMMONOBJS) $(CLIENTOBJS) $(SERVEROBJS) $(DEPFILE) *.so *.tgz core* *~
	make -C ./libdvbmpeg clean

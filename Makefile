#
# Makefile for a Video Disk Recorder plugin
#
# $Id: $

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.

PLUGIN = streamdev

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'const char \*VERSION *=' common.c | awk '{ print $$5 }' | sed -e 's/[";]//g')

### The directory environment:

# Use package data if installed...otherwise assume we're under the VDR source directory:
PKGCFG = $(if $(VDRDIR),$(shell pkg-config --variable=$(1) $(VDRDIR)/vdr.pc),$(shell pkg-config --variable=$(1) vdr || pkg-config --variable=$(1) ../../../vdr.pc))
LIBDIR = $(call PKGCFG,libdir)
LOCDIR = $(call PKGCFG,locdir)
PLGCFG = $(call PKGCFG,plgcfg)
#
TMPDIR ?= /tmp

### The compiler options:

export CFLAGS   = $(call PKGCFG,cflags)
export CXXFLAGS = $(call PKGCFG,cxxflags)

### The version number of VDR's plugin API:

APIVERSION = $(call PKGCFG,apiversion)

### Allow user defined options to overwrite defaults:

-include $(PLGCFG)

### export all vars for sub-makes, using absolute paths
LIBDIR := $(shell cd $(LIBDIR) >/dev/null 2>&1 && pwd)
LOCDIR := $(shell cd $(LOCDIR) >/dev/null 2>&1 && pwd)
export
unexport PLUGIN

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include -I..
export INCLUDES

DEFINES += -D_GNU_SOURCE

ifdef DEBUG
DEFINES += -DDEBUG
endif
ifdef STREAMDEV_DEBUG
DEFINES += -DDEBUG
endif

### The main target:

.PHONY: all client server install install-client install-server dist clean
all: client server

### Targets:

client:
	$(Q)$(MAKE) --no-print-directory -C ./tools
	$(Q)$(MAKE) --no-print-directory -C ./client

server:
	$(Q)$(MAKE) --no-print-directory -C ./tools
	$(Q)$(MAKE) --no-print-directory -C ./libdvbmpeg
	$(Q)$(MAKE) --no-print-directory -C ./remux
	$(Q)$(MAKE) --no-print-directory -C ./server

install-client: client
	$(Q)$(MAKE) --no-print-directory -C ./client install
	$(Q)# installs to $(LIBDIR)/libvdr-streamdev-client.so.$(APIVERSION)

install-server: server
	$(Q)$(MAKE) --no-print-directory -C ./server install
	$(Q)# installs to $(LIBDIR)/libvdr-streamdev-server.so.$(APIVERSION)

install: install-client install-server

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	$(Q)$(MAKE) --no-print-directory -C ./tools clean
	$(Q)$(MAKE) --no-print-directory -C ./libdvbmpeg clean
	$(Q)$(MAKE) --no-print-directory -C ./remux clean
	$(Q)$(MAKE) --no-print-directory -C ./client clean
	$(Q)$(MAKE) --no-print-directory -C ./server clean

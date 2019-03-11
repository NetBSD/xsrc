SUBDIRS = man
EXTRA_DIST = autogen.sh

bin_PROGRAMS = transset

AM_CFLAGS = $(CWARNFLAGS) $(TRANSSET_CFLAGS)
transset_LDADD = $(TRANSSET_LIBS)

transset_SOURCES = transSet.c dsimple.c dsimple.h

MAINTAINERCLEANFILES = ChangeLog INSTALL

.PHONY: ChangeLog INSTALL

INSTALL:
	$(INSTALL_CMD)

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog INSTALL

#	$NetBSD: Makefile,v 1.1 1997/12/01 13:11:50 mrg Exp $
#
# this is a lame hack.  such is life

all: all-xc all-contrib

all-xc:
	(cd xc; make)

all-contrib:
	(cd contrib; make)

build: build-xc build-contrib
	make install

build-xc:
	(cd xc; make World)

build-contrib:
	(cd contrib; make World)

install: install-xc install-contrib

install-xc:
	(cd xc; make install && make install.man)

install-contrib:
	(cd contrib; make install && make install.man)

#	$NetBSD: Makefile,v 1.2 1997/12/04 21:12:00 mrg Exp $
#
# this is a lame hack.  such is life

all: all-xc all-contrib

all-xc:
	(cd xc; ${MAKE})

all-contrib:
	(cd contrib; ${MAKE})

build: build-xc build-contrib
	${MAKE} install

build-xc:
	(cd xc; ${MAKE} World)

build-contrib:
	(cd contrib; xmkmf -a; ${MAKE} clean; ${MAKE})
	${MAKE} install-contrib 

install: install-xc install-contrib

install-xc:
	(cd xc; ${MAKE} install && ${MAKE} install.man)

install-contrib:
	(cd contrib; ${MAKE} install && ${MAKE} install.man)

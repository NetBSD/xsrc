#	$NetBSD: Makefile,v 1.4 1999/05/16 16:07:12 tv Exp $
#
# build and install xsrc

build:
.if exists(xc/xmakefile)
	@echo ""
	@echo "Warning:  This does not rebuild from a clean tree."
	@echo "Use 'make clean' first if you want to start from scratch."
	@echo ""
.endif
	@${MAKE} all install

all: all-xc all-contrib

all-xc:
.if exists(xc/xmakefile)
	@cd xc && ${MAKE} -f xmakefile World
.else
	@cd xc && ${MAKE} World
.endif

all-contrib:
.if !exists(contrib/Makefile)
	@cd contrib && PATH=../xc/config/imake:$$PATH \
	    sh ../xc/config/util/xmkmf -a ../xc ../contrib
.endif
	@cd contrib && ${MAKE}

install: install-xc install-contrib

install-xc:
	@cd xc && ${MAKE} install && ${MAKE} install.man

install-contrib:
	@cd contrib && ${MAKE} install && ${MAKE} install.man

clean cleandir distclean:
	@cd xc && ${MAKE} clean
	@-cd contrib && ${MAKE} clean

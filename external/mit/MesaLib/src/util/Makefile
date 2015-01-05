#	$NetBSD: Makefile,v 1.1.2.2 2015/01/05 20:59:27 martin Exp $

# generate format_srgb.c

.include "../Makefile.inc"

SCRIPT=	${X11SRCDIR.MesaLib}/src/util/format_srgb.py
format_srgb.c: ${SCRIPT}
	${PYTHON} ${SCRIPT} > $@.tmp && mv $@.tmp $@

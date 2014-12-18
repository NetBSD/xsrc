#	$NetBSD: Makefile,v 1.1 2014/12/18 06:37:30 mrg Exp $

# generate format_srgb.c

.include "../Makefile.inc"

SCRIPT=	${X11SRCDIR.MesaLib}/src/util/format_srgb.py
format_srgb.c: ${SCRIPT}
	${PYTHON} ${SCRIPT} > $@.tmp && mv $@.tmp $@

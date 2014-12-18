#	$NetBSD: Makefile,v 1.1 2014/12/18 06:37:30 mrg Exp $

# generate program_parse.tab.c and .h

all: program_parse.tab.c

.include "../../Makefile.inc"

PROGRAM_PARSE=${X11SRCDIR.MesaLib}/src/mesa/program/program_parse.y

BISON?=	bison

program_parse.tab.c: ${_SCRIPT} ${XML}
	${BISON} ${PROGRAM_PARSE} -p "_mesa_program_" -v -d -o $@

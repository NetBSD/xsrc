#	$NetBSD: Makefile,v 1.1.1.1 2019/03/08 10:19:24 mrg Exp $

# generate program_parse.tab.c and .h

all: program_parse.tab.c

.include "../../Makefile.inc"

PROGRAM_PARSE=${X11SRCDIR.MesaLib}/src/mesa/program/program_parse.y

BISON?=	bison

program_parse.tab.c: ${_SCRIPT} ${XML}
	${BISON} ${PROGRAM_PARSE} -p "_mesa_program_" -v -d -o $@

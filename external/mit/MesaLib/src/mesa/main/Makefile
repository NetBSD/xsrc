#	$NetBSD: Makefile,v 1.1 2014/12/18 06:37:30 mrg Exp $

# generate stuff

all: enums.c api_exec.c dispatch.h remap_helper.h

.include "../../Makefile.inc"

XML=		${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/gl_and_es_API.xml

MAINPAIRS=	gl_enums.py		enums.c \
		gl_genexec.py		api_exec.c \
		remap_helper.py		remap_helper.h

.for _py _out in ${MAINPAIRS}
${_out}: ${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/${_py} ${XML}
	${PYTHON} ${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/${_py} -f ${XML} > $@.tmp && mv $@.tmp $@
.endfor

dispatch.h: ${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/gl_table.py ${XML}
	${PYTHON} ${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/gl_table.py -f ${XML} -m remap_table > $@.tmp && mv $@.tmp $@

#	$NetBSD: Makefile,v 1.1 2014/12/18 06:37:30 mrg Exp $

# generate stuff

all: glapitemp.h glapitable.h glapi_gentable.c glapi_x86-64.S glapi_glapi_mapi_tmp.h glprocs.h indirect.c indirect.h indirect_init.c indirect_size.h indirect_size.c shared-glapi_glapi_mapi_tmp.h

.include "../../Makefile.inc"

ABI_SCRIPT=	${X11SRCDIR.MesaLib}/src/mapi/mapi_abi.py
XML=		${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/gl_and_es_API.xml
GLXML=		${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/gl_API.xml

shared-glapi_glapi_mapi_tmp.h: ${ABI_SCRIPT} ${XML}
	${PYTHON} ${ABI_SCRIPT} --mode lib --printer shared-glapi ${XML} > $@.tmp && mv $@.tmp $@

GLPAIRS=	gl_apitemp.py		glapitemp.h \
		gl_table.py		glapitable.h \
		gl_gentable.py		glapi_gentable.c \
		gl_x86-64_asm.py	glapi_x86-64.S

.for _py _out in ${GLPAIRS}
${_out}: ${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/${_py} ${XML}
	${PYTHON} ${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/${_py} -f ${XML} > $@.tmp && mv $@.tmp $@
.endfor

glapi_glapi_mapi_tmp.h: ${ABI_SCRIPT} ${XML}
	${PYTHON} ${ABI_SCRIPT} --printer glapi --mode lib ${XML} > $@.tmp && mv $@.tmp $@

glprocs.h: ${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/gl_procs.py ${XML}
	${PYTHON} ${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/gl_procs.py -c -f ${XML} > $@.tmp && mv $@.tmp $@

INDENT=			indent -i4 -nut -br -brs -npcs -ce -TGLubyte -TGLbyte -TBool
GLXPROTO_SEND_SCRIPT=	${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/glX_proto_send.py
GLXPROTO_SIZE_SCRIPT=	${X11SRCDIR.MesaLib}/src/mapi/glapi/gen/glX_proto_size.py

indirect.c: ${GLXPROTO_SEND_SCRIPT} ${GLXML}
	${PYTHON} ${GLXPROTO_SEND_SCRIPT} -f ${GLXML} -m proto | \
		${INDENT} > $@.tmp && mv $@.tmp $@

indirect.h: ${GLXPROTO_SEND_SCRIPT} ${GLXML}
	${PYTHON} ${GLXPROTO_SEND_SCRIPT} -f ${GLXML} -m init_h \
		> $@.tmp && mv $@.tmp $@

indirect_init.c: ${GLXPROTO_SEND_SCRIPT} ${GLXML}
	${PYTHON} ${GLXPROTO_SEND_SCRIPT} -f ${GLXML} -m init_c \
		> $@.tmp && mv $@.tmp $@

indirect_size.h: ${GLXPROTO_SIZE_SCRIPT} ${GLXML}
	${PYTHON} ${GLXPROTO_SIZE_SCRIPT} -f ${GLXML} -m size_h --only-set -h _INDIRECT_SIZE_H_ | \
		${INDENT} > $@.tmp && mv $@.tmp $@

indirect_size.c: ${GLXPROTO_SIZE_SCRIPT} ${GLXML}
	${PYTHON} ${GLXPROTO_SIZE_SCRIPT} -f ${GLXML} -m size_c --only-set | \
		${INDENT} > $@.tmp && mv $@.tmp $@

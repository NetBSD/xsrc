#	$NetBSD: Makefile,v 1.1.1.1 2019/03/08 10:19:24 mrg Exp $

# generate options.h

all: options.h

.include "../../../../../Makefile.inc"

POOL=	${X11SRCDIR.MesaLib}/src/mesa/drivers/dri/common/xmlpool

_SCRIPT=${POOL}/gen_xmlpool.py

T_OPTIONS_H=	${POOL}/t_options.h

# XXX ca/de/es/etc are not found?
options.h: ${_SCRIPT} ${T_OPTIONS_H}
	(cd ${POOL}; ${PYTHON} ${_SCRIPT} ${T_OPTIONS_H} . ca de es nl fr sv ) > $@.tmp && mv $@.tmp $@

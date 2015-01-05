#	$NetBSD: Makefile,v 1.1.2.2 2015/01/05 20:59:25 martin Exp $

# generate u_indices_gen.c u_unfilled_gen.c and u_format_table.c

all: u_indices_gen.c u_unfilled_gen.c u_format_table.c

.include "../../Makefile.inc"

u_indices_gen.c: ${X11SRCDIR.MesaLib}/src/gallium/auxiliary/indices/u_indices_gen.py
	${PYTHON} ${X11SRCDIR.MesaLib}/src/gallium/auxiliary/indices/u_indices_gen.py > $@.tmp && mv $@.tmp $@

u_unfilled_gen.c: ${X11SRCDIR.MesaLib}/src/gallium/auxiliary/indices/u_unfilled_gen.py
	${PYTHON} ${X11SRCDIR.MesaLib}/src/gallium/auxiliary/indices/u_unfilled_gen.py > $@.tmp && mv $@.tmp $@
	
_CSV=${X11SRCDIR.MesaLib}/src/gallium/auxiliary/util/u_format.csv
u_format_table.c: ${X11SRCDIR.MesaLib}/src/gallium/auxiliary/util/u_format_table.py ${_CSV}
	${PYTHON} ${X11SRCDIR.MesaLib}/src/gallium/auxiliary/util/u_format_table.py ${_CSV} > $@.tmp && mv $@.tmp $@

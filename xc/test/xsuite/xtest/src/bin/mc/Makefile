# 
# Copyright (c) 1990, 1991  X Consortium
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 
# Except as contained in this notice, the name of the X Consortium shall not be
# used in advertising or otherwise to promote the sale, use or other dealings
# in this Software without prior written authorization from the X Consortium.
#
# Copyright 1990, 1991 UniSoft Group Limited.
#
# Permission to use, copy, modify, distribute, and sell this software and
# its documentation for any purpose is hereby granted without fee,
# provided that the above copyright notice appear in all copies and that
# both that copyright notice and this permission notice appear in
# supporting documentation, and that the names of MIT and UniSoft not be
# used in advertising or publicity pertaining to distribution of the
# software without specific, written prior permission.  MIT and UniSoft
# make no representations about the suitability of this software for any
# purpose.  It is provided "as is" without express or implied warranty.
# 
# $XConsortium: Makefile,v 1.6 94/04/17 21:00:18 rws Exp $
#

CFILES=main.c sections.c files.c shortname.c error.c gccomps.c set.c \
	expand.c code.c make.c ma.c cond.c mas.c
OFILES=main.o sections.o files.o shortname.o error.o gccomps.o set.o \
	expand.o code.o make.o ma.o cond.o mas.o
RM_FILES = *.o mc core

MV=mv

all: mc

mc:$P $(OFILES)
	$(CC) $(LDFLAGS) -o $@ $(OFILES) $(PORTLIB) $(SYSLIBS)

name10lc: shortname.c
	-$(MV) shortname.o tmp.o
	$(CC) -c $(CFLAGS) -Dtest=name10lc shortname.c
	$(CC) $(LDFLAGS) -o $@ shortname.o $(LIBS)
	$(MV) tmp.o shortname.o

name12: shortname.c
	-$(MV) shortname.o tmp.o
	$(CC) -c $(CFLAGS) -Dtest=name12 shortname.c
	$(CC) $(LDFLAGS) -o $@ shortname.o $(LIBS)
	$(MV) tmp.o shortname.o

clean:
	$(RM) $(RM_FILES)

install: mc
	$(RM) $(XTESTBIN)/mc
	$(CP) mc $(XTESTBIN)/mc
	$(RM) $(XTESTBIN)/ma $(XTESTBIN)/mmkf $(XTESTBIN)/mexpand
	$(LN) $(XTESTBIN)/mc $(XTESTBIN)/ma
	$(LN) $(XTESTBIN)/mc $(XTESTBIN)/mmkf
	$(LN) $(XTESTBIN)/mc $(XTESTBIN)/mexpand

clobber: clean

lint:
	$(LINT) $(INCLUDES) $(CFILES)

mcproto.h:
	mkxtl *.c > mcproto.h

frc:
	rm mcproto.h

main.o error.o files.o gccomps.o make.o: mc.h mcproto.h
code.o sections.o set.o shortname.o: mc.h mcproto.h

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
# $XConsortium: Makefile,v 1.13 94/04/17 21:01:16 rws Exp $
#
#
#	Makefile for server tester library
#
#

RM_FILES = *.ln *.o core *.a ConnOption.c

HDRS=\
	../include/Xstlib.h\
	../include/Xstos.h\
	../include/XstDisplay.h\
	XstlibInt.h\
	XstosInt.h\
	DataMove.h

CFILES=\
	ShowReq.c\
	ShowExtReq.c\
	SendReq.c\
	SendXIReq.c\
	SendEvt.c\
	ShowRep.c\
	ShowExtRep.c\
	MakeReq.c\
	MakeXIReq.c\
	RcvRep.c\
	RcvExtRep.c\
	RcvEvt.c\
	RcvExtEvt.c\
	RcvErr.c\
	RcvExtErr.c\
	ShowEvt.c\
	ShowExtEvt.c\
	ShowErr.c\
	ShowExtErr.c\
	DfltVals.c\
	ResMng.c\
	Log.c\
	ClientMng.c\
	Expect.c\
	ConnOption.c\
	ValListMng.c\
	Timer.c\
	XstIO.c\
	DataMove.c\
	TestMng.c\
	Utils.c\
	SendSup.c\
	ShowSup.c\
	TestSup.c\
	Validate.c\
	tpstartup.c\
	startup.c\
	linkstart.c\
	delete.c


OFILES =\
	ShowReq.o\
	ShowExtReq.o\
	SendReq.o\
	SendXIReq.o\
	SendEvt.o\
	ShowRep.o\
	ShowExtRep.o\
	MakeReq.o\
	MakeXIReq.o\
	RcvRep.o\
	RcvExtRep.o\
	RcvEvt.o\
	RcvExtEvt.o\
	RcvErr.o\
	RcvExtErr.o\
	ShowEvt.o\
	ShowExtEvt.o\
	ShowExtErr.o\
	ShowErr.o\
	DfltVals.o\
	ResMng.o\
	Log.o\
	ClientMng.o\
	Expect.o\
	ConnOption.o\
	ValListMng.o\
	Timer.o\
	XstIO.o\
	DataMove.o\
	TestMng.o\
	Utils.o\
	SendSup.o\
	ShowSup.o\
	TestSup.o\
	Validate.o\
	tpstartup.o\
	startup.o\
	linkstart.o\
	delete.o

# CFLAGS - Flags for C compiler specific to the X Protocols tests.
CFLAGS=$(XP_CFLAGS)

# LINTFLAGS - Flags for lint specific to the X Protocol tests.
LINTFLAGS=$(XP_LINTFLAGS)
LINTLIBS=$(XP_LINTLIBS)

all:	libXst.a

ConnOption.c: $(XP_OPEN_DIS)
	rm -f $@
	$(CP) $(XP_OPEN_DIS) $@

libXst.a:$(P) $(OFILES)
	rm -f $@
	$(AR) $@ `$(LORDER) $(OFILES) | $(TSORT)`
	: $(RANLIB) $@

install: all
	$(CP) libXst.a $(XSTLIB)
	$(RANLIB) $(XSTLIB)

FORCE:	  CLOBBER all

LINT:
		$(LINT) $(LINTFLAGS) $(CFILES) $(LINTTCM) $(LINTLIBS)

LINTLIB:	$(LINTXST)

$(LINTXST):   llib-lXst.ln
	       cp $? $@
		@echo " $? made and installed"

llib-lXst.ln:
		$(LINT) -CXst $(LINTFLAGS) $(CFILES)

clean:
		$(RM) $(RM_FILES)

clobber:	clean

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
# $XConsortium: Makefile,v 1.34 94/04/17 21:00:30 rws Exp $
#

##########################################################################
#
#	SCCS:		
#	NAME:		xtestlib
#	PRODUCT:	xtest library
#	AUTHOR:		Steve Ratcliffe, UniSoft Ltd.
#	DATE CREATED:	30 Aug 1990
#	TARGETS:	libxtest.a
#	MODIFICATIONS:
#
##########################################################################


CFILES =	startup.c ex_startup.c \
	lookupname.c err.c getevent.c stackorder.c report.c \
	iponlywin.c badwin.c startcall.c makewin.c checkevent.c \
	nextvclass.c badgc.c badpixm.c badfont.c badvis.c linkstart.c \
	issuppvis.c checkarea.c getsize.c pattern.c crechild.c checktile.c \
	makepixm.c nextvinf.c makegc.c dumpimage.c verimage.c dset.c \
	delete.c rpt.c \
	savimage.c checkpixel.c notmember.c setline.c checkgc.c gcflush.c \
	tpstartup.c nondpth1pix.c gcinclude.c regid.c openfonts.c checkfont.c \
	badcmap.c makecolmap.c makecur.c makeimg.c checkimg.c opendisp.c \
	bitcount.c config.c makewin2.c block.c winh.c pointer.c events.c \
	gettime.c maxsize.c buildtree.c exposechk.c \
	settimeout.c makeregion.c xthost.c pfcount.c XTestExt.c devcntl.c \
	extenavail.c environ.c cursor.c XInput.c
OFILES =	startup.o ex_startup.o \
	lookupname.o err.o getevent.o stackorder.o report.o \
	iponlywin.o badwin.o startcall.o makewin.o checkevent.o \
	nextvclass.o badgc.o badpixm.o badfont.o badvis.o linkstart.o \
	issuppvis.o checkarea.o getsize.o pattern.o crechild.o checktile.o \
	makepixm.o nextvinf.o makegc.o dumpimage.o verimage.o dset.o \
	delete.o rpt.o \
	savimage.o checkpixel.o notmember.o setline.o checkgc.o gcflush.o \
	tpstartup.o nondpth1pix.o gcinclude.o regid.o openfonts.o checkfont.o \
	badcmap.o makecolmap.o makecur.o makeimg.o checkimg.o opendisp.o \
	bitcount.o config.o makewin2.o block.o winh.o pointer.o events.o \
	gettime.o maxsize.o buildtree.o exposechk.o \
	settimeout.o makeregion.o xthost.o pfcount.o XTestExt.o devcntl.o \
	extenavail.o environ.o cursor.o XInput.o

RM_FILES = *.ln *.o core *.a

##############################################################################

all: $(XTESTINCDIR)/xtestlib.h libxtest.a 

libxtest.a: $(OFILES)
		$(AR) $@ `$(LORDER) $(OFILES) | $(TSORT)`
		: $(RANLIB) $@


.c.o:
	$(CC) -c $(CFLAGS) $<

install: all
	$(CP) libxtest.a $(XTESTLIB)
	$(RANLIB) $(XTESTLIB)

FORCE:		CLOBBER all

LINT:
		$(LINT) $(LINTFLAGS) $(CFILES) $(LINTTCM) $(LINTLIBS)

LINTLIB:	$(LINTXTEST)

$(LINTXTEST):	llib-lxtest.ln
		cp $? $@
		@echo "	$? made and installed"

llib-lxtest.ln: $(CFILES)
		echo $(CFILES) | sed -e 's/linkstart.c //' >/tmp/Lint$$$$.tmp; \
		$(LINT) -Cxtest $(LINTFLAGS) `cat /tmp/Lint$$$$.tmp` ; \
		rm -f Lint$$$$.tmp

clean:
		$(RM) $(RM_FILES)

clobber:	clean

doc:
	ctodoc $(CFILES) > ../../doc/xtestraw.mm

xtlibproto.h: $(CFILES)
	mkxtl $(CFILES) > xtlibproto.h
	$(RM) $(XTESTINCDIR)/xtlibproto.h 
	$(CP) xtlibproto.h $(XTESTINCDIR)

tags:	$(CFILES)
	ctags $(CFILES) > tags 2> /dev/null

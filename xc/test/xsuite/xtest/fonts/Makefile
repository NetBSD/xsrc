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
# $XConsortium: Makefile,v 1.10 94/04/17 21:00:11 rws Exp $
#
#
#
#
#
# $XFree86: test/xsuite/xtest/fonts/Makefile,v 1.2 1997/01/16 10:54:58 dawes Exp $
#

##########################################################################
#
#	SCCS:		
#	NAME:		font
#	PRODUCT:	xtest library
#	AUTHOR:		Steve Ratcliffe, UniSoft Ltd.
#	DATE CREATED:	30 Aug 1990
#	TARGETS:	libfont.a
#	MODIFICATIONS:
#
##########################################################################


CFILES = xtfont0.c xtfont1.c xtfont2.c xtfont3.c xtfont4.c xtfont5.c \
	xtfont6.c
OFILES = xtfont0.o xtfont1.o xtfont2.o xtfont3.o xtfont4.o xtfont5.o \
	xtfont6.o
RM_FILES = *.ln *.o core *.a

##############################################################################

all: libfont.a

libfont.a: $P	$(OFILES)
		$(AR) $@ `$(LORDER) $(OFILES) | $(TSORT)`
		: $(RANLIB) $@

FORCE:		CLOBBER all

LINT:
		$(LINT) $(LINTFLAGS) $(CFILES) $(LINTTCM) $(LINTLIBS)

LINTLIB:	$(LINTFONTS)

$(LINTFONTS):	llib-lfonts.ln
		$(CP) $? $@
		@echo "	$? made and installed"

llib-lfonts.ln:
		$(LINT) -Cfonts $(LINTFLAGS) $(CFILES)

clean:
		$(RM) $(RM_FILES)

clobber:	clean

install: all install_pcf
	$(CP) libfont.a $(XTESTLIBDIR)
	$(RANLIB) $(XTESTLIBDIR)/libfont.a

# -----------------
#
# NOTE: The way that the fonts are installed is system dependent.
# This part of the Makefile is provided solely as an example.
#
# See the User Guide for more details.
#

FONTSRCS=xtfont0 xtfont1 xtfont2 xtfont3 xtfont4 xtfont5 xtfont6

comp_pcf:
	@ echo "These commands are system dependent - See User Guide"
	@ for f in $(FONTSRCS) ;\
	do \
		(set -x; bdftopcf -t $$f.bdf -o $$f.pcf); \
	done
	mkfontdir

comp_snf:
	@ echo "These commands are system dependent - See User Guide"
	@ for f in $(FONTSRCS) ;\
	do \
		(set -x; bdftosnf $$f.bdf > $$f.snf); \
	done
	mkfontdir

comp_dxpcf:
	@ echo "These commands are system dependent - See User Guide"
	@ for f in $(FONTSRCS) ;\
	do \
		(set -x; dxfc -t $$f.bdf > $$f.pcf); \
	done
	dxmkfontdir

comp_fb:
	@ echo "These commands are system dependent - See User Guide"
	@ for f in $(FONTSRCS) ;\
	do \
		(set -x; convertfont -d . -o $$f $$f.bdf); \
	done
	bldfamily -d .

install_pcf install_dxpcf: comp_pcf
	@ echo "These commands are system dependent - See User Guide"
	@ if [ -d $(XTESTFONTDIR) ]; then set +x; \
		else (set -x; mkdir $(XTESTFONTDIR)); fi
	@ for f in $(FONTSRCS) ;\
	do \
		(set -x; cp $$f.pcf $(XTESTFONTDIR)); \
	done
	cp fonts.dir $(XTESTFONTDIR)

install_snf:
	@ echo "These commands are system dependent - See User Guide"
	@ if [ -d $(XTESTFONTDIR) ]; then set +x; \
		else (set -x; mkdir $(XTESTFONTDIR)); fi
	@ for f in $(FONTSRCS) ;\
	do \
		(set -x; cp $$f.snf $(XTESTFONTDIR)); \
	done
	cp fonts.dir $(XTESTFONTDIR)

install_fb:
	@ echo "These commands are system dependent - See User Guide"
	@ if [ -d $(XTESTFONTDIR) ]; then set +x; \
		else (set -x; mkdir $(XTESTFONTDIR)); fi
	@ for f in $(FONTSRCS) ;\
	do \
		(set -x; cp $$f.fb $$f.ff $(XTESTFONTDIR)); \
	done

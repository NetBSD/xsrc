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
# $XConsortium: Makefile,v 1.8 94/04/17 21:01:52 rws Exp $
#

#
# This Makefile can be used to compile all the tests in this 
# section of the X test suite in such a way that they are all links to 
# a single executable file. This normally allows a considerable 
# reduction in the disc space requirements for the X test suite 
# when fully built.
#
# There are two ways this can be done:
# 1) Using the TET. Execute the command:
#     tcc -b -s link_scen xtest linkbuild
# in the directory $TET_ROOT/xtest.
# This will execute the TET build tool (which is normally pmake) 
# in the top level directory of each section of the test suite 
# (including this directory).
#
# 2) Directly without using the TET. Execute the command:
#     pmake 
# in this directory.
#
# For more details, refer to the User Guide
#

CAT=cat

ALLTESTS=Tests

TESTOFILES=\
allplns.o \
blckpxl.o \
blckpxlofs.o \
btmpbtordr.o \
btmppd.o \
btmpunt.o \
cllsofscrn.o \
clsdsply.o \
cnnctnnmbr.o \
dfltclrmp.o \
dfltclrmpo.o \
dfltdpth.o \
dfltdpthof.o \
dfltgc.o \
dfltgcofsc.o \
dfltrtwdw.o \
dfltscrn.o \
dfltscrnof.o \
dfltvsl.o \
dfltvslofs.o \
dsbckngstr.o \
dsplyclls.o \
dsplyhght.o \
dsplyhghtm.o \
dsplyofscr.o \
dsplyplns.o \
dsplystr.o \
dsplywdth.o \
dsplywdthm.o \
dssvundrs.o \
evntmskofs.o \
fr.o \
hghtmmofsc.o \
hghtofscrn.o \
imgbytordr.o \
lstdpths.o \
lstknwnrqs.o \
lstpxmpfrm.o \
mncmpsofsc.o \
mxcmpsofsc.o \
nop.o \
nxtrqst.o \
opndsply.o \
plnsofscrn.o \
prtclrvsn.o \
prtclvrsn.o \
qlngth.o \
rtwdw.o \
rtwdwofscr.o \
scrncnt.o \
scrnnmbrof.o \
scrnofdspl.o \
srvrvndr.o \
vndrrls.o \
wdthmmofsc.o \
wdthofscrn.o \
whtpxl.o \
whtpxlofsc.o

LINKOFILE=linktbl.o
OFILES = $(TESTOFILES) $(LINKOFILE)

DIRLIST=\
allplns \
blckpxl \
blckpxlofs \
btmpbtordr \
btmppd \
btmpunt \
cllsofscrn \
clsdsply \
cnnctnnmbr \
dfltclrmp \
dfltclrmpo \
dfltdpth \
dfltdpthof \
dfltgc \
dfltgcofsc \
dfltrtwdw \
dfltscrn \
dfltscrnof \
dfltvsl \
dfltvslofs \
dsbckngstr \
dsplyclls \
dsplyhght \
dsplyhghtm \
dsplyofscr \
dsplyplns \
dsplystr \
dsplywdth \
dsplywdthm \
dssvundrs \
evntmskofs \
fr \
hghtmmofsc \
hghtofscrn \
imgbytordr \
lstdpths \
lstknwnrqs \
lstpxmpfrm \
mncmpsofsc \
mxcmpsofsc \
nop \
nxtrqst \
opndsply \
plnsofscrn \
prtclrvsn \
prtclvrsn \
qlngth \
rtwdw \
rtwdwofscr \
scrncnt \
scrnnmbrof \
scrnofdspl \
srvrvndr \
vndrrls \
wdthmmofsc \
wdthofscrn \
whtpxl \
whtpxlofsc

all: subdirs test

test:$P $(OFILES) $(LIBS) $(TCM)
	$(CC) $(LDFLAGS) -o $@ $(OFILES) $(TCM) $(LIBS) $(SYSLIBS)
	$(CAT) test > $(ALLTESTS)
	$(RM) test

subdirs:
	if [ ! -f $(ALLTESTS) ]; then $(CAT) /dev/null > $(ALLTESTS); \
	chmod a+x $(ALLTESTS); else : ; fi
	for i in $(DIRLIST); do \
		if [ -d $$i ]; then \
			(cd $$i; echo Compiling in $$i; $(TET_BUILD_TOOL) linkexec); \
		fi; \
	done

# The xtestlib is made if it doesn't exist
$(XTESTLIB):
	cd $(XTESTROOT)/src/lib; $(TET_BUILD_TOOL) install

# The fontlib is made if it doesn't exist
$(XTESTFONTLIB):
	cd $(XTESTROOT)/fonts; $(TET_BUILD_TOOL) install

clean:	cletests clesubdirs

cletests:
	$(RM) test $(OFILES) $(ALLTESTS) core

clesubdirs:
	for i in $(DIRLIST); do \
		if [ -d $$i ]; then (cd $$i; $(TET_CLEAN_TOOL) ); fi; done

clobber: clean

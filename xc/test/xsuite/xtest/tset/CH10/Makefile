#
# Copyright 1990, 1991 by the Massachusetts Institute of Technology and
# UniSoft Group Limited.
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
# $XConsortium: Makefile,v 1.6 92/06/29 19:14:50 rws Exp $
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
addpxl.o \
clpbx.o \
crtbtmpfrm.o \
crtimg.o \
crtpxmpfrm.o \
crtrgn.o \
dltcntxt.o \
dstryimg.o \
dstryrgn.o \
emptyrgn.o \
eqlrgn.o \
fndcntxt.o \
ftchbffr.o \
ftchbyts.o \
gtdflt.o \
gtpxl.o \
gtvslinf.o \
intrsctrgn.o \
iscrsrky.o \
isfnctnky.o \
iskypdky.o \
ismdfrky.o \
ismscfnctn.o \
ispfky.o \
kycdtkysym.o \
kysymtkycd.o \
kysymtstr.o \
lkpkysym.o \
lkpstr.o \
mtchvslinf.o \
offstrgn.o \
plygnrgn.o \
pntinrgn.o \
prmllc.o \
prsclr.o \
prsgmtry.o \
ptpxl.o \
rbndkysym.o \
rctinrgn.o \
rdbtmpfl.o \
rfrshkybrd.o \
rmdstrydtb.o \
rmgtfldtbs.o \
rmgtrsrc.o \
rmgtstrdtb.o \
rmintlz.o \
rmmrgdtbss.o \
rmprscmmnd.o \
rmptfldtbs.o \
rmptlnrsrc.o \
rmptrsrc.o \
rmptstrrsr.o \
rmqgtrsrc.o \
rmqgtsrchl.o \
rmqgtsrchr.o \
rmqptrsrc.o \
rmqptstrrs.o \
rmqrktstr.o \
rmstrtbndn.o \
rmstrtqrk.o \
rmstrtqrkl.o \
rmunqqrk.o \
rsrcmngrst.o \
rttbffrs.o \
sbimg.o \
sbtrctrgn.o \
shrnkrgn.o \
strbffr.o \
strbyts.o \
strgn.o \
strtkysym.o \
svcntxt.o \
unnrctwthr.o \
unnrgn.o \
unqcntxt.o \
wrtbtmpfl.o \
xrrgn.o

LINKOFILE=linktbl.o
OFILES = $(TESTOFILES) $(LINKOFILE)

DIRLIST=\
addpxl \
clpbx \
crtbtmpfrm \
crtimg \
crtpxmpfrm \
crtrgn \
dltcntxt \
dstryimg \
dstryrgn \
emptyrgn \
eqlrgn \
fndcntxt \
ftchbffr \
ftchbyts \
gtdflt \
gtpxl \
gtvslinf \
intrsctrgn \
iscrsrky \
isfnctnky \
iskypdky \
ismdfrky \
ismscfnctn \
ispfky \
kycdtkysym \
kysymtkycd \
kysymtstr \
lkpkysym \
lkpstr \
mtchvslinf \
offstrgn \
plygnrgn \
pntinrgn \
prmllc \
prsclr \
prsgmtry \
ptpxl \
rbndkysym \
rctinrgn \
rdbtmpfl \
rfrshkybrd \
rmdstrydtb \
rmgtfldtbs \
rmgtrsrc \
rmgtstrdtb \
rmintlz \
rmmrgdtbss \
rmprscmmnd \
rmptfldtbs \
rmptlnrsrc \
rmptrsrc \
rmptstrrsr \
rmqgtrsrc \
rmqgtsrchl \
rmqgtsrchr \
rmqptrsrc \
rmqptstrrs \
rmqrktstr \
rmstrtbndn \
rmstrtqrk \
rmstrtqrkl \
rmunqqrk \
rsrcmngrst \
rttbffrs \
sbimg \
sbtrctrgn \
shrnkrgn \
strbffr \
strbyts \
strgn \
strtkysym \
svcntxt \
unnrctwthr \
unnrgn \
unqcntxt \
wrtbtmpfl \
xrrgn

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

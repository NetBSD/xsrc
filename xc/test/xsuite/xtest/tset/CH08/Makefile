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
# $XConsortium: Makefile,v 1.9 92/06/29 19:11:58 rws Exp $
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
bttnprss.o \
bttnrls.o \
chckifevnt.o \
chckmskevn.o \
chcktypdev.o \
chcktypdwd.o \
chckwdwevn.o \
clntmssg.o \
clrmpntfy.o \
cnfgrntfy.o \
cnfgrrqst.o \
crcltntfy.o \
crcltrqst.o \
crtntfy.o \
dsplymtnbf.o \
dsplynm.o \
dstryntfy.o \
entrntfy.o \
evntsqd.o \
exps.o \
fcsin.o \
fcsot.o \
flsh.o \
grphcsexps.o \
grvtyntfy.o \
gterrrdtbs.o \
gterrrtxt.o \
gtmtnevnts.o \
ifevnt.o \
kympntfy.o \
kyprss.o \
kyrls.o \
lvntfy.o \
mpntfy.o \
mppngntfy.o \
mprqst.o \
mskevnt.o \
mtnntfy.o \
nexps.o \
nxtevnt.o \
pkevnt.o \
pkifevnt.o \
pndng.o \
prprtyntfy.o \
ptbckevnt.o \
rprntntfy.o \
rszrqst.o \
slctinpt.o \
slctnclr.o \
slctnntfy.o \
slctnrqst.o \
sndevnt.o \
staftrfnct.o \
sterrrhndl.o \
stioerrrhn.o \
sync.o \
synchrnz.o \
unmpntfy.o \
vsbltyntfy.o \
wdwevnt.o

LINKOFILE=linktbl.o
OFILES = $(TESTOFILES) $(LINKOFILE)

DIRLIST=\
bttnprss \
bttnrls \
chckifevnt \
chckmskevn \
chcktypdev \
chcktypdwd \
chckwdwevn \
clntmssg \
clrmpntfy \
cnfgrntfy \
cnfgrrqst \
crcltntfy \
crcltrqst \
crtntfy \
dsplymtnbf \
dsplynm \
dstryntfy \
entrntfy \
evntsqd \
exps \
fcsin \
fcsot \
flsh \
grphcsexps \
grvtyntfy \
gterrrdtbs \
gterrrtxt \
gtmtnevnts \
ifevnt \
kympntfy \
kyprss \
kyrls \
lvntfy \
mpntfy \
mppngntfy \
mprqst \
mskevnt \
mtnntfy \
nexps \
nxtevnt \
pkevnt \
pkifevnt \
pndng \
prprtyntfy \
ptbckevnt \
rprntntfy \
rszrqst \
slctinpt \
slctnclr \
slctnntfy \
slctnrqst \
sndevnt \
staftrfnct \
sterrrhndl \
stioerrrhn \
sync \
synchrnz \
unmpntfy \
vsbltyntfy \
wdwevnt

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

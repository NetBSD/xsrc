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
# $XConsortium: Makefile,v 1.10 92/06/29 19:16:07 rws Exp $
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

# CFLAGS - Compilation flags specific to the X Protocol tests.
#
CFLAGS=$(XP_CFLAGS)
SYSLIBS=$(XP_SYSLIBS)
LIBS=$(XP_LIBS)

# LINTFLAGS - Flags for lint specific to the X Protocol tests.
#
LINTFLAGS=$(XP_LINTFLAGS)
LINTLIBS=$(XP_LINTLIBS)

CAT=cat

ALLTESTS=Tests

TESTOFILES=\
allcclr.o \
allcclrcll.o \
allcclrpln.o \
allcnmdclr.o \
allwevnts.o \
bdrqst.o \
bll.o \
chngactvpn.o \
chnggc.o \
chnghsts.o \
chngkybrdc.o \
chngkybrdm.o \
chngpntrcn.o \
chngprprty.o \
chngsvst.o \
chngwdwatt.o \
clrar.o \
clsfnt.o \
cnfgrwdw.o \
cnvrtslctn.o \
cpyar.o \
cpyclrmpan.o \
cpygc.o \
cpypln.o \
crcltwdw.o \
crtclrmp.o \
crtcrsr.o \
crtgc.o \
crtglyphcr.o \
crtpxmp.o \
crtwdw.o \
dltprprty.o \
dstrysbws.o \
dstrywdw.o \
fllply.o \
frclrmp.o \
frclrs.o \
frcrsr.o \
frcscrnsvr.o \
frgc.o \
frpxmp.o \
grbbttn.o \
grbky.o \
grbkybrd.o \
grbpntr.o \
grbsrvr.o \
gtatmnm.o \
gtfntpth.o \
gtgmtry.o \
gtimg.o \
gtinptfcs.o \
gtkybrdcnt.o \
gtkybrdmpp.o \
gtmdfrmppn.o \
gtmtnevnts.o \
gtpntrcntr.o \
gtpntrmppn.o \
gtprprty.o \
gtscrnsvr.o \
gtslctnown.o \
gtwdwattrb.o \
imgtxt16.o \
imgtxt8.o \
instllclrm.o \
intrnatm.o \
kllclnt.o \
lkpclr.o \
lstextnsns.o \
lstfnts.o \
lstfntswth.o \
lsthsts.o \
lstinstlld.o \
lstprprts.o \
mpsbws.o \
mpwdw.o \
noprtn.o \
opndsply.o \
opnfnt.o \
plyarc.o \
plyfllarc.o \
plyfllrctn.o \
plyln.o \
plypnt.o \
plyrctngl.o \
plysgmnt.o \
plytxt16.o \
plytxt8.o \
ptimg.o \
qrybstsz.o \
qryclrs.o \
qryextnsn.o \
qryfnt.o \
qrykymp.o \
qrypntr.o \
qrytr.o \
qrytxtextn.o \
rclrcrsr.o \
rprntwdw.o \
rttprprts.o \
sndevnt.o \
staccsscnt.o \
stclprctng.o \
stclsdwnmd.o \
stdshs.o \
stfntpth.o \
stinptfcs.o \
stmdfrmppn.o \
stpntrmppn.o \
strclrs.o \
strnmdclr.o \
stscrnsvr.o \
stslctnown.o \
trnsltcrds.o \
ungrbbttn.o \
ungrbky.o \
ungrbkybrd.o \
ungrbpntr.o \
ungrbsrvr.o \
unmpsbws.o \
unmpwdw.o \
unnstllclr.o \
wrppntr.o

LINKOFILE=linktbl.o
OFILES = $(TESTOFILES) $(LINKOFILE)

DIRLIST=\
allcclr \
allcclrcll \
allcclrpln \
allcnmdclr \
allwevnts \
bdrqst \
bll \
chngactvpn \
chnggc \
chnghsts \
chngkybrdc \
chngkybrdm \
chngpntrcn \
chngprprty \
chngsvst \
chngwdwatt \
clrar \
clsfnt \
cnfgrwdw \
cnvrtslctn \
cpyar \
cpyclrmpan \
cpygc \
cpypln \
crcltwdw \
crtclrmp \
crtcrsr \
crtgc \
crtglyphcr \
crtpxmp \
crtwdw \
dltprprty \
dstrysbws \
dstrywdw \
fllply \
frclrmp \
frclrs \
frcrsr \
frcscrnsvr \
frgc \
frpxmp \
grbbttn \
grbky \
grbkybrd \
grbpntr \
grbsrvr \
gtatmnm \
gtfntpth \
gtgmtry \
gtimg \
gtinptfcs \
gtkybrdcnt \
gtkybrdmpp \
gtmdfrmppn \
gtmtnevnts \
gtpntrcntr \
gtpntrmppn \
gtprprty \
gtscrnsvr \
gtslctnown \
gtwdwattrb \
imgtxt16 \
imgtxt8 \
instllclrm \
intrnatm \
kllclnt \
lkpclr \
lstextnsns \
lstfnts \
lstfntswth \
lsthsts \
lstinstlld \
lstprprts \
mpsbws \
mpwdw \
noprtn \
opndsply \
opnfnt \
plyarc \
plyfllarc \
plyfllrctn \
plyln \
plypnt \
plyrctngl \
plysgmnt \
plytxt16 \
plytxt8 \
ptimg \
qrybstsz \
qryclrs \
qryextnsn \
qryfnt \
qrykymp \
qrypntr \
qrytr \
qrytxtextn \
rclrcrsr \
rprntwdw \
rttprprts \
sndevnt \
staccsscnt \
stclprctng \
stclsdwnmd \
stdshs \
stfntpth \
stinptfcs \
stmdfrmppn \
stpntrmppn \
strclrs \
strnmdclr \
stscrnsvr \
stslctnown \
trnsltcrds \
ungrbbttn \
ungrbky \
ungrbkybrd \
ungrbpntr \
ungrbsrvr \
unmpsbws \
unmpwdw \
unnstllclr \
wrppntr

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

# The X Protocol test library is made if it doesn't exist
$(XSTLIB):
	cd $(XTESTROOT)/src/libproto; $(TET_BUILD_TOOL) install


clean:	cletests clesubdirs

cletests:
	$(RM) test $(OFILES) $(ALLTESTS) core

clesubdirs:
	for i in $(DIRLIST); do \
		if [ -d $$i ]; then (cd $$i; $(TET_CLEAN_TOOL) ); fi; done

clobber: clean

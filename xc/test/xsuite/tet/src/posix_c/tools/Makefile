#
#  Copyright 1990 Open Software Foundation (OSF)
#  Copyright 1990 Unix International (UI)
#  Copyright 1990 X/Open Company Limited (X/Open)
# 
#  Permission to use, copy, modify, and distribute this software and its
#  documentation for any purpose and without fee is hereby granted, provided
#  that the above copyright notice appear in all copies and that both that
#  copyright notice and this permission notice appear in supporting
#  documentation, and that the name of OSF, UI or X/Open not be used in 
#  advertising or publicity pertaining to distribution of the software 
#  without specific, written prior permission.  OSF, UI and X/Open make 
#  no representations about the suitability of this software for any purpose.  
#  It is provided "as is" without express or implied warranty.
# 
#  OSF, UI and X/Open DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO 
#  EVENT SHALL OSF, UI or X/Open BE LIABLE FOR ANY SPECIAL, INDIRECT OR 
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF 
#  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
#  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
#  PERFORMANCE OF THIS SOFTWARE.
# 
##########################################################################
#
#    SCCS:            @(#)makefile	1.9 03/09/92
#    NAME:            TET Tools Makefile
#    PRODUCT:         TET (Test Environment Toolkit)
#    AUTHOR:          OSF Validation & SQA
#    DATE CREATED:    14 May 1991
#    TARGETS:         tcc, rpt & tet_ins
#    MODIFICATIONS:
#                     "TET Rework"
#                     UniSoft Ltd, August 1991.
#
#		Stuart Boutell, 12 March 1992
#			Modified for easy use with MIT X Test suite
#			* added behaviour for make install
#			* ensure that only required target (tcc) is made
#
##########################################################################

# these are normally inherited from the top level makefile.
SIG_IGNORE =
SIG_LEAVE  =

BIN        = $(TET_ROOT)/bin
INCDIR     = $(TETBASE)/src/posix_c/inc
INC        = -I$(INCDIR)

CFLOCAL    = $(INC) -DTET_ROOT='"$(TET_ROOT)"'  \
		-DTET_SIG_IGNORE="$(SIG_IGNORE)" -DTET_SIG_LEAVE="$(SIG_LEAVE)"

LINT       = lint
LINTFLAGS  = $(INC) $(DEFINES) -n
LINTLIBS   = -lposix

TCCOBJS	   = again.o config.o exec.o journal.o scenario.o startit.o tcc.o tool.o
TCCFILES   = again.c config.c exec.c journal.c scenario.c startit.c tcc.c tool.c
TCCHDRS    = $(INCDIR)/tcc_env.h $(INCDIR)/tcc_mac.h $(INCDIR)/tet_jrnl.h \
            $(INCDIR)/tcc_prot.h
TCCLIBS    = $(LDFLAGS) $(PORTLIB) $(SYSLIBS)

XTS_TARGETS= tcc


INSTALL: all
	cp tcc rpt tet_ins $(BIN)

install: $(XTS_TARGETS)
	cp $(XTS_TARGETS) $(XTESTBIN)

all:	tcc rpt tet_ins


tcc:	$(TCCOBJS)	
	$(CC) -o tcc $(TCCOBJS) $(TCCLIBS)

rpt:	rpt.o
	$(CC) -o rpt rpt.o

tet_ins: tet_ins.o
	$(CC) -o tet_ins tet_ins.o

$(TCCOBJS): $(TCCHDRS)

rpt.o: $(INCDIR)/tet_jrnl.h 

clean:	CLEAN

CLEAN:
	-$(RM) $(TCCOBJS) tcc rpt rpt.o tet_ins tet_ins.o

CLOBBER: CLEAN
	-$(RM) $(BIN)/tcc $(BIN)/rpt $(BIN)/tet_ins


FORCE:	CLOBBER all


LINT:	LINT_TCC LINT_RPT LINT_INS

LINT_TCC:
	$(LINT) $(LINTFLAGS) $(TCCFILES) $(LINTLIBS)

LINT_RPT:
	$(LINT) $(LINTFLAGS) rpt.c $(LINTLIBS)

LINT_INS:
	$(LINT) $(LINTFLAGS) tet_ins.c $(LINTLIBS)


#
# Copyright 1990 Open Software Foundation (OSF)
# Copyright 1990 Unix International (UI)
# Copyright 1990 X/Open Company Limited (X/Open)
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation, and that the name of OSF, UI or X/Open not be used in 
# advertising or publicity pertaining to distribution of the software 
# without specific, written prior permission.  OSF, UI and X/Open make 
# no representations about the suitability of this software for any purpose.  
# It is provided "as is" without express or implied warranty.
#
# OSF, UI and X/Open DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO 
# EVENT SHALL OSF, UI or X/Open BE LIABLE FOR ANY SPECIAL, INDIRECT OR 
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF 
# USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
# PERFORMANCE OF THIS SOFTWARE.

##########################################################################
#
#	SCCS:		@(#)makefile	1.9 03/09/92
#	NAME:		'C' API
#	PRODUCT:	TET (Test Environment Toolkit)
#	AUTHOR:		Geoff Clare, UniSoft Ltd.
#	DATE CREATED:	23 June 1990
#	TARGETS:	tcm.o tcmchild.o libapi.a
#	MODIFICATIONS:
#
#		Geoff Clare, 10 Sept 1990
#			Use -I$(TET_ROOT)/inc
#			Add install target
#
#		Geoff Clare, 10 Oct 1990
#			Change file names and install directories
#
#		Geoff Clare, 22 Oct 1990
#			Default target is INSTALL
#			Distribute tet_api.h in $(TET_ROOT)/inc/posix_c
#				instead of "." & modify references to it
#
#		Geoff Clare, 30 Nov 1990
#			Change -I$(TET_ROOT)/inc to -I../inc
#
#		Stuart Boutell, 13 March 1992
#			Modified for easy use with the MIT X Test suite
#
##########################################################################

# Default directory locations
INSTLIB =	$(TETLIB)

LINT =	lint

# Compiler options: these need to be changed to enable NSIG to be defined.
# Either add another feature test macro which will make it visible in
# <signal.h>, or add -DNSIG=<value>.  NSIG should be 1 greater than
# the highest supported signal number on the system.
LINTFLAGS =	$(INCS) $(DEFINES) -n
LINTLIBS =	-lposix

CFILES_TCM =	tcm.c
OFILES_TCM =	tcm.o
CFILES_TCMCH =	tcmchild.c
OFILES_TCMCH =	tcmchild.o
CFILES_API =	cancel.c config.c resfile.c tet_exec.c tet_fork.c
OFILES_API =	cancel.o config.o resfile.o tet_exec.o tet_fork.o

CFILES =	$(CFILES_TCM) $(CFILES_TCMCH) $(CFILES_API)
OFILES =	$(OFILES_TCM) $(OFILES_TCMCH) $(OFILES_API)

CFLOCAL= -I../inc

##############################################################################

INSTALL:	all
		cp $(OFILES_TCM) $(OFILES_TCMCH) libapi.a $(INSTLIB)
		$(RANLIB) $(INSTLIB)/libapi.a

install:	INSTALL

all:		$(OFILES_TCM) $(OFILES_TCMCH) libapi.a

libapi.a:	$(OFILES_API)
		$(AR) $@ `$(LORDER) $(OFILES_API) | $(TSORT)`
		: $(RANLIB) $@

$(OFILES):	$(TETINCDIR)/tet_api.h

FORCE:		CLOBBER all

LINT:
		$(LINT) $(LINTFLAGS) $(CFILES) $(LINTLIBS)

LINTLIB:	llib-ltcm.ln llib-ltcmc.ln

llib-ltcm.ln:	llib-ltcm.c
		$(LINT) -o tcm $(LINTFLAGS) -u $?

llib-ltcmc.ln:	llib-ltcmc.c
		$(LINT) -o tcmc $(LINTFLAGS) -u $?

CLEAN:
		$(RM) $(OFILES) makefile.bak llib-ltcm.ln llib-ltcmc.ln

CLOBBER:	CLEAN
		$(RM) $(INSTLIB)/tcm.o $(INSTLIB)/tcmchild.o \
			$(INSTLIB)/libapi.a 

clean:		CLEAN

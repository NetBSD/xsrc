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
#	NAME:		Shell API
#	PRODUCT:	TET (Test Environment Toolkit)
#	AUTHOR:		Andrew Dingwall, UniSoft Ltd.
#	DATE CREATED:	15 November 1990
#	TARGETS:	tcm.sh tetapi.sh
#	MODIFICATIONS:
#			Geoff Clare, 26 Sep 1991
#			Added chmod of installed files, CLOBBER commands
#
#			Geoff Clare, 11 Oct 1991
#			Allow signal numbers to be specified in makefile
#
##########################################################################

# standard signal numbers - change to correct numbers for your system
# SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGPIPE, SIGALRM,
# SIGTERM, SIGUSR1, SIGUSR2, SIGTSTP, SIGCONT, SIGTTIN, SIGTTOU
STD_SIGNALS =	1 2 3 4 6 8 13 14 15 16 17 25 26 27 28

# signals that are always unhandled - change for your system
# May need to include SIGSEGV and others if the shell can't trap them
# SIGKILL, SIGCHLD, SIGSTOP, (SIGSEGV, ...)
SPEC_SIGNALS =	9 18 24 11

# Default directory locations
TET_ROOT =	../../..
INSTLIB =	$(TET_ROOT)/lib/xpg3sh

RM =		rm -f

SHFILES =	tcm.sh tetapi.sh

##############################################################################

INSTALL:	all
		sed -e 's/STD_SIGNAL_LIST/$(STD_SIGNALS)/' \
		    -e 's/SPEC_SIGNAL_LIST/$(SPEC_SIGNALS)/' \
		    tcm.sh > $(INSTLIB)/tcm.sh
		cp tetapi.sh $(INSTLIB)
		chmod 755 $(INSTLIB)/tcm.sh $(INSTLIB)/tetapi.sh

all:		$(SHFILES)

FORCE:		CLOBBER all

CLEAN:
		$(RM) makefile.bak

CLOBBER:	CLEAN
		$(RM) $(INSTLIB)/tcm.sh $(INSTLIB)/tetapi.sh


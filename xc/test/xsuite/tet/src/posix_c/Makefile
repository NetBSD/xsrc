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
#    NAME:            TET Top Level Makefile
#    PRODUCT:         TET (Test Environment Toolkit)
#    AUTHOR:          OSF Validation & SQA
#    DATE CREATED:    14 May 1991
#    TARGETS:         tools & api
#    MODIFICATIONS:
#                     "TET Rework"
#                     UniSoft Ltd, August 1991.
#
#  			Modified for MIT X Test Suite, 10/3/92
#			Stuart Boutell, UniSoft Ltd
#
##########################################################################
 
all:	tools api

tools:	nofile
	cd tools ; pmake TET_ROOT="$(TET_ROOT)" CC="$(CC)" COPTS="$(COPTS)" \
		DEFINES="$(DEFINES)" SIG_IGNORE="$(SIG_IGNORE)" \
		SIG_LEAVE="$(SIG_LEAVE)" install

api:	nofile
	cd api ; pmake CC="$(CC)" COPTS="$(COPTS)" DEFINES="$(DEFINES)" \
		AR="$(AR)" RANLIB="$(RANLIB)" LORDER="$(LORDER)" \
		TSORT="$(TSORT)" install

clean:
	cd tools ; pmake clean
	cd api ; pmake clean

nofile:


# 
#  Copyright 1990, 1991 by the Massachusetts Institute of Technology and
#  UniSoft Group Limited.
#  
#  Permission to use, copy, modify, distribute, and sell this software and
#  its documentation for any purpose is hereby granted without fee,
#  provided that the above copyright notice appear in all copies and that
#  both that copyright notice and this permission notice appear in
#  supporting documentation, and that the names of MIT and UniSoft not be
#  used in advertising or publicity pertaining to distribution of the
#  software without specific, written prior permission.  MIT and UniSoft
#  make no representations about the suitability of this software for any
#  purpose.  It is provided "as is" without express or implied warranty.
# 
# $XConsortium: Makefile,v 1.6 92/06/11 15:54:44 rws Exp $
#
CFILES=ajax.c vblowup_c.c
OFILES=ajax.o vblowup_c.o
RM_FILES=*.o blowup core

blowup:	$(OFILES)
	$(CC) $(LDFLAGS) -o $@ $(OFILES) $(PORTLIB) $(SYSLIBS) $(SYSMATHLIB)

install: blowup
	$(CP) blowup $(XTESTBIN)

clean:
	$(RM) $(RM_FILES)

lint:
	$(LINT) $(LINTFLAGS) $(CFILES)

doc:
	ctodoc $(CFILES) > $$TET_ROOT/../doc/blowupraw.mm

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
# $XConsortium: Makefile,v 1.8 94/04/17 21:00:08 rws Exp $
#

# This Makefile can be used to print the documentation for the X test suite.
# - "make print00" formats and prints the Release Notes
# - "make print01" formats and prints the User Guide
# - "make print02" formats and prints the Programmers Guide 
# - "make printall" formats and prints all three documents
# You may need to alter the following variables first:

# This command expands .so directives to include the specified files.
SOELIM=soelim

# This command converts tbl requests to nroff format.
TBL=tbl 

MACROS=-mm

# This command formats text using nroff/troff with the mm macros
# and converts to PostScript
#TROFF=ditroff $(MACROS) -Tpsc | psdit
#TROFF=troff $(MACROS) -Tpsc | psdit
TROFF=psroff $(MACROS)

# This command sends output to a printer.
#LPR=cpr -Bps
LPR=lpr

all: relnotes.ps userguide.ps progguide.ps paper.ps

printall:	print00 print01 print02

print00:	relnotes.ps
	cat relnotes.ps | $(LPR)

relnotes.ps:
	$(SOELIM) relnotes.mm | $(TBL) | $(TROFF) >relnotes.ps

print01:	userguide.ps
	cat userguide.ps | $(LPR)

userguide.ps:
	$(SOELIM) userguide.mm | $(TBL) | $(TROFF) >userguide.ps

print02:	progguide.ps
	cat progguide.ps | $(LPR)

progguide.ps:
	$(SOELIM) progguide.mm | $(TBL) | $(TROFF) >progguide.ps

print03:	paper.ps
	cat paper.ps | $(LPR)

paper.ps:
	$(SOELIM) paper.mm | $(TBL) | $(TROFF) >paper.ps

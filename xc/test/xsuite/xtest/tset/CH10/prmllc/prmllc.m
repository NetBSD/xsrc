/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: prmllc.m,v 1.6 94/04/17 21:09:55 rws Exp $
 */
>>TITLE Xpermalloc CH10
char *

unsigned int size;
>>ASSERTION Good A
A call to xname returns a pointer to a block of memory
.A size
bytes large.
>>STRATEGY
Call xname to allocate a block of memory.
Verify that all addresses within the block may be accessed.
>>CODE
int i, badaccess;
char *ret;

/* Call xname to allocate a block of memory. */
	size = 2048;
	ret = XCALL;

	if( ret == (char *)NULL ) {
		delete("%s returned a NULL pointer.", TestName);
		report("Expecting a pointer to a block of %u bytes", size);
	} else
		CHECK;

/* Verify that all addresses within the block may be accessed. */
	for(i=0; i<size; i++) {
		*(ret+i)='s';	/* A SIGSEGV indicates t r o u b l e */
		CHECK;
	}

	badaccess=0;
	for(i=0; i<size; i++) {
		if( *(ret+i) != 's' ) {
			badaccess++;
		} else
			CHECK;
	}

	if( badaccess!=0 ) {
		FAIL;
		report("%s did not return a writable block of memory.",
			TestName);
		report("%d addresses within the allocated block could not be written to.", badaccess);
	} else
		CHECK;

	CHECKPASS(2+size*2);

>>ASSERTION Bad B 1
When sufficient temporary storage cannot be allocated, then a call to
xname returns
.S NULL .

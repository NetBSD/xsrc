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
 * $XConsortium: allcszhnts.m,v 1.7 94/04/17 21:08:29 rws Exp $
 */
>>TITLE XAllocSizeHints CH09
XSizeHints *
XAllocSizeHints()
>>ASSERTION Good A
A call to xname allocates and returns a pointer to a
.S XSizeHints 
structure, which can be freed with
.S XFree ,
in which each component is set to zero.
>>STRATEGY
Allocate an XSizeHints structure with XAllocSizeHints.
Verify NULL is not returned.
Verify that each component of the structure is set to zero.
Release the allocated memory using XFree.
>>CODE
XSizeHints	*shints;

	shints = XAllocSizeHints();

	if (shints == (XSizeHints *)NULL) {
		delete("XAllocSizeHints returned NULL.");
		return;
	} else
		CHECK;

	if(shints->flags != 0L) {
		report("The flags component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->x != 0) {
		report("The x component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->y != 0) {
		report("The y component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->width != 0) {
		report("The width component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->height != 0) {
		report("The height component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->min_width != 0) {
		report("The min_width component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->min_height != 0) {
		report("The min_height component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->max_width != 0) {
		report("The max_width component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->max_height != 0) {
		report("The max_height component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->width_inc != 0) {
		report("The width_inc component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->height_inc != 0) {
		report("The height_inc component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if((shints->min_aspect.x != 0) || (shints->min_aspect.y != 0)){
		report("The min_aspect components of the XSizeHints structure were not zero.");
		FAIL;
	} else
		CHECK;

	if((shints->max_aspect.x != 0) || (shints->max_aspect.y != 0)){
		report("The max_aspect components of the XSizeHints structure were not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->base_width != 0) {
		report("The base_width component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->base_height != 0) {
		report("The base_height component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->win_gravity != 0) {
		report("The win_gravity component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;


	XFree((char *) shints);
	CHECKPASS(17);

>>ASSERTION Good B 1
When insufficient memory is available, then
a call to xname returns NULL.
>># Kieron	Completed	Review

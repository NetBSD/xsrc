/* 
 * 
Copyright 1985, 1986, 1987, 1998 The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * The X Window System is a Trademark of The Open Group.
 *
 */
/* $XFree86: xc/lib/oldX/X10.h,v 1.5 2004/06/01 00:16:57 dawes Exp $ */
/*
 * Copyright (c) 1994-2004 by The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*
 *	X10.h - Header definition and support file for the C subroutine
 *	interface library for V10 support routines.
 */
#ifndef _X10_H_
#define _X10_H_

/* Used in XDraw and XDrawFilled */

typedef struct {
	short x, y;
	unsigned short flags;
} Vertex;

/* The meanings of the flag bits.  If the bit is 1 the predicate is true */

#define VertexRelative		0x0001		/* else absolute */
#define VertexDontDraw		0x0002		/* else draw */
#define VertexCurved		0x0004		/* else straight */
#define VertexStartClosed	0x0008		/* else not */
#define VertexEndClosed		0x0010		/* else not */
/*#define VertexDrawLastPoint	0x0020 	*/      /* else don't */	

/*
The VertexDrawLastPoint option has not been implemented in XDraw and 
XDrawFilled so it shouldn't be defined. 
*/

/*
 * XAssoc - Associations used in the XAssocTable data structure.  The 
 * associations are used as circular queue entries in the association table
 * which is contains an array of circular queues (buckets).
 */
typedef struct _XAssoc {
	struct _XAssoc *next;	/* Next object in this bucket. */
	struct _XAssoc *prev;	/* Previous obejct in this bucket. */
	Display *display;	/* Display which ownes the id. */
	XID x_id;		/* X Window System id. */
	char *data;		/* Pointer to untyped memory. */
} XAssoc;

/* 
 * XAssocTable - X Window System id to data structure pointer association
 * table.  An XAssocTable is a hash table whose buckets are circular
 * queues of XAssoc's.  The XAssocTable is constructed from an array of
 * XAssoc's which are the circular queue headers (bucket headers).  
 * An XAssocTable consists an XAssoc pointer that points to the first
 * bucket in the bucket array and an integer that indicates the number
 * of buckets in the array.
 */
typedef struct {
    XAssoc *buckets;		/* Pointer to first bucket in bucket array.*/
    int size;			/* Table size (number of buckets). */
} XAssocTable;

XAssocTable *XCreateAssocTable(int size);
char *XLookUpAssoc(Display *dpy, XAssocTable *table, register XID x_id);
int XMakeAssoc(Display *dpy, XAssocTable *table, XID x_id, XPointer data);
int XDestroyAssocTable(XAssocTable *table);
int XDeleteAssoc(Display *dpy, XAssocTable *table, XID x_id);
Status XDraw(Display *dpy, Drawable d, GC gc, Vertex *vlist, int vcount);
Status XDrawFilled(Display *dpy, Drawable d, GC gc, Vertex *vlist, int vcount);

#endif /* _X10_H_ */

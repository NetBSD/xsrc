/*
 
Copyright (c) 1990, 1991, 1992  X Consortium

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
 * Copyright 1990, 1991, 1992 by UniSoft Group Limited.
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
 * $XConsortium: getcwd.c,v 1.3 94/04/17 20:59:48 rws Exp $
 */

#include <stdlib.h>
#include <errno.h>
#include <sys/param.h>

/*
 * A simple emulation of getcwd() to allow the tcc to run on BSD systems
 * that don't provide this.
 * getcwd() differs from the BSD getwd() by specifing a size.
 * 
 * Do not use if the system provides getcwd()
 * 
 */
char *
getcwd(buf, size)
char	*buf;
int 	size;
{
char	*bp;
char	*getwd();

	if (size <= 0) {
		errno = EINVAL;
		return((char*) 0);
	}

	/*
	 * get a buffer that hold the max possible BSD pathname.
	 */
	bp = (char *)malloc((size_t)(MAXPATHLEN+1));
	if (bp == (char *)0) {
		errno = ENOMEM;
		return((char *) 0);
	}

	if (getwd(bp) == (char *)0) {
		/* errno should have been set */
		return((char *) 0);
	}

	if (strlen(bp) >= size) {
		errno = ERANGE;
		return((char *) 0);
	}

	strcpy(buf, bp);

	free(bp);

	return(buf);
	
}

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
 * $XConsortium: Xstos.h,v 1.7 94/04/17 21:00:03 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

/* Header file for UNIX library for X Server tests. */

/*
 * System header files.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

/*
 * System calls, library functions and other definitions in UNIX/DYNIX.
 */


/* Comment out the unecessary (or wrong) definitions */

extern int errno;

#ifdef NOT_REQUIRED

char	*alloca();
/* char	*calloc(); */
void	exit();
int	fclose();
/* void	fflush(); */
FILE	*fopen();
int	fprintf();
void	free();
int	fscanf();
int	getitimer();
char	*index();
/* char	*malloc(); */
void	perror();
/* char	*realloc(); */
int	setitimer();
int	sigblock();
/* int	(*signal())(); */
int	sigsetmask();
char	*strcat();
int	strcmp();
char	*strncat();
char	*strncpy();

void	bcopy();
void    bzero();

#endif /* NOT_REQUIRED */

#define SearchString(string, char) strchr((string), (char))


/*
 * Note that some machines do not return a valid pointer for malloc(0), in
 * which case we provide an alternate under the control of the
 * define MALLOC_0_RETURNS_NULL.  This is necessary because some
 * Xst code expects malloc(0) to return a valid pointer to storage.
 */
#define EXTRA 16
#ifdef MALLOC_0_RETURNS_NULL

# define Xstmalloc(size) malloc(max((size)+EXTRA,1))
# define Xstrealloc(ptr, size) realloc((ptr), max((size)+EXTRA,1))
# define Xstcalloc(nelem, elsize) calloc(max((nelem)+EXTRA,1), (elsize))

#else

# define Xstmalloc(size) malloc((size)+EXTRA)
# define Xstrealloc(ptr, size) realloc((ptr), (size)+EXTRA)
# define Xstcalloc(nelem, elsize) calloc((nelem)+EXTRA, (elsize))

#endif

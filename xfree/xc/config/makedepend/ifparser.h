/*
 * $Xorg: ifparser.h,v 1.3 2000/08/17 19:41:51 cpqbld Exp $
 *
 * Copyright 1992 Network Computing Devices, Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Network Computing Devices may not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Network Computing Devices makes
 * no representations about the suitability of this software for any purpose.
 * It is provided ``as is'' without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL NETWORK COMPUTING DEVICES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Fulton
 *          Network Computing Devices, Inc.
 *
 * Simple if statement processor. Please see ifparser.c for the parsing tree.
 *
 *
 * External Entry Points:
 *
 *     ParseIfExpression		parse a string for #if
 */

/* $XFree86: xc/config/makedepend/ifparser.h,v 3.6 2004/03/05 16:02:58 tsi Exp $ */

#include <stdio.h>

typedef int Bool;
#define False 0
#define True 1

typedef struct _if_parser {
    struct {				/* functions */
	const char *(*handle_error) (struct _if_parser *, const char *,
				     const char *);
	long (*eval_variable) (struct _if_parser *, const char *, int,
			       const char *);
	int (*eval_defined) (struct _if_parser *, const char *, int);
    } funcs;
    char *data;
} IfParser;

const char *ParseIfExpression (
    IfParser *,
    const char *,
    long *
);

extern int variable_has_args(IfParser *ip, const char *var, int len);

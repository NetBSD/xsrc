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
 * $XConsortium: shortname.c,v 1.4 94/04/17 21:00:26 rws Exp $
 *
 * Author: Steve Ratcliffe, UniSoft Ltd.
 */
/*
 * Routines to produce abreviated names of the functions.
 * 
 * This algorithm is used as a basis:
 * 
 *   Background -> Bg
 *   Subwindow  -> Sbw
 *   String     -> Str
 *   Window     -> Wdw
 *   Remove all lower case vowels
 *	 Truncate.
 *   If the string ended in 16 then make it end in 16 after truncation.
 */

#include	<stdio.h>
#include	<string.h>
#include	"mc.h"

#define	VOWELS	"aeiou"

char	*
name12(inname)
char	*inname;
{
char	*name;
char	*cp;
char	*np;
int 	end16 = 0;
int 	n;

#ifndef test
	name = mcstrdup(inname);
#else
	name = inname;
#endif

	if (name[strlen(name)-1] == '\n')
		name[strlen(name)-1] = '\0';

	if (name[strlen(name)-1] == '6')
		end16 = 1;

	cp = strinstr(name, "Background");
	if (cp) {
		(void) strcpy(cp, "Bg");
		(void) strcpy(cp+2, cp+10);
	}
	cp = strinstr(name, "Subwindow");
	if (cp) {
		(void) strcpy(cp, "Sbw");
		(void) strcpy(cp+3, cp+9);
	}
	cp = strinstr(name, "String");
	if (cp) {
		(void) strcpy(cp, "Str");
		(void) strcpy(cp+3, cp+6);
	}
	cp = strinstr(name, "Window");
	if (cp) {
		(void) strcpy(cp, "Wdw");
		(void) strcpy(cp+3, cp+6);
	}

	/*
	 * Take out lowercase vowels.
	 */
	np = name;
	cp = name;
	for (;;) {
		n = strcspn(np, VOWELS);
		if (n == 0)
			break;
		/* strncpy(cp, np, n); doesn't work on happy */
		for (; n > 0; n--)
			*cp++ = *np++;

		n = strspn(np, VOWELS);
		np += n;
	}
	*cp = '\0';

	if (strlen(name) > 12) {
		name[12] = '\0';
		if (end16) {
			name[strlen(name)-2] = '1';
			name[strlen(name)-1] = '6';
		}
	}
	return(name);
}

/*
 * Truncate to 12 characters and lowercase everything.
 */
char *
name12lc(name)
char	*name;
{
char	*np;
char	*cp;

	np = name12(name);
	for (cp = np; *cp; cp++)
		*cp = tolower(*cp);

	return(np);
}

/*
 * Truncate to 10 characters.  Like name12 but ommit leading
 * 'X'.
 */
char *
name10(name)
char	*name;
{
char	*np;
int 	end16;

	if (name[strlen(name)-1] == '6')
		end16 = 1;
	else
		end16 = 0;

	np = name12(name);
	if (*np == 'X')
		np++;
	if (strlen(np) > 10) {
		np[10] = '\0';
		if (end16) {
			np[strlen(np)-2] = '1';
			np[strlen(np)-1] = '6';
		}
	}

	return(np);
}

/*
 * Truncate to 10 characters and lowercase everything.
 */
char *
name10lc(name)
char	*name;
{
char	*np;
char	*cp;

	np = name10(name);
	for (cp = np; *cp; cp++)
		*cp = tolower(*cp);

	return(np);
}

#ifdef test

main(argc, argv)
int 	argc;
char	**argv;
{

	while (--argc) {
		printf("%s\n", test(*++argv));
	}

}

/*
 * Find a s2 within s1.
 */
char *
strinstr(s1, s2)
char	*s1;
char	*s2;
{
char	*cp;
int 	len;

	len = strlen(s2);
	if (len == 0)
		return(s1);

	for (cp = s1; *cp; cp++) {
		if (*cp == *s2) {
			if (strncmp(cp, s2, len) == 0) {
				return(cp);
			}
		}
	}

	return(NULL);
}

#endif

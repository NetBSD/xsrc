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
 * $XConsortium: getopt.c,v 1.3 94/04/17 20:59:48 rws Exp $
 */

#include <stdio.h>
#include <string.h>

char	*optarg;
int 	optind = 1;
int 	opterr = 1;

getopt(argc, argv, optstring)
int 	argc;
char	**argv;
char	*optstring;
{
static int 	avplace;
char	*ap;
char	*cp;
int 	c;

	if (optind >= argc)
		return(EOF);

	ap = argv[optind] + avplace;

	/* At begining of arg but not an option */
	if (avplace == 0) {
		if (ap[0] != '-')
			return(EOF);
		else if (ap[1] == '-') {
			/* Special end of options option */
			optind++;
			return(EOF);
		} else if (ap[1] == '\0')
			return(EOF);	/* single '-' is not allowed */
	}

	/* Get next letter */
	avplace++;
	c = *++ap;

	cp = strchr(optstring, c);
	if (cp == NULL || c == ':') {
		if (opterr)
			fprintf(stderr, "Unrecognised option -- %c\n", c);
		return('?');
	}

	if (cp[1] == ':') {
		/* There should be an option arg */
		avplace = 0;
		if (ap[1] == '\0') {
			/* It is a separate arg */
			if (++optind >= argc) {
				if (opterr)
					fprintf(stderr, "Option requires an argument\n");
				return('?');
			}
			optarg = argv[optind++];
		} else {
			/* is attached to option letter */
			optarg = ap + 1;
			++optind;
		}
	} else {

		/* If we are out of letters then go to next arg */
		if (ap[1] == '\0') {
			++optind;
			avplace = 0;
		}

		optarg = NULL;
	}

	return(c);
}

#ifdef test

main(argc, argv)
int 	argc;
char	**argv;
{
extern char *optarg;
extern int 	optind;
int 	c;

	while ((c = getopt(argc, argv, "ab:cd:")) != EOF) {
		switch (c) {
		case 'a':
		case 'c':
			printf("opt -%c\n", c);
			break;
		case 'b':
		case 'd':
			printf("opt -%c %s\n", c, optarg);
			break;
		case '?':
			printf("Bad option %c\n", c);
			break;
		default:
			printf("unexpected default\n");
			break;
		}
	}
}
#endif

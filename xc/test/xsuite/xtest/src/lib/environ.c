/*
Copyright (c) 1990, 1991, 1993  X Consortium

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

 * Copyright 1990, 1991, 1992 and UniSoft Group Limited.
 * 
 * Copyright 1993 by the Hewlett-Packard Company.
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
 * $XConsortium: environ.c,v 1.3 94/04/17 21:00:44 rws Exp $
 */

/*
 * Add a string to the environment. This is sourced from the tet api.
 * References to size_t have been removed for maximum portability
 */

#include "xtest.h"

#include "string.h"

extern char **environ;

int
xtest_putenv(envstr)
char *envstr;
{
        /*
         * This routine mimics putenv(), and is provided purely
         * because putenv() is not in POSIX.1
         */

        char **newenv, **cur, *envname;
        int n, count = 0;
        static char **allocp = NULL;

        if (environ == NULL)
	{
                newenv = (char **)malloc((2*sizeof(char *)));
                if (newenv == NULL)
                        return -1;

                newenv[0] = envstr;
                newenv[1] = NULL;
                environ = newenv;
                allocp = newenv;
                return 0;
        }

        cur = environ;
        while (*cur != NULL)
        {
                count++;
                envname = *cur;
                n = strcspn(envstr, "=");
                if (strncmp(envname, envstr, n) || envname[n] != '=')
                        cur++;
                else
                {
                        *cur = envstr;
                        return 0;
                }
        }
        /*
         * If we previously allocated this environment enlarge it using
         * realloc(), otherwise allocate a new one and copy it over.
         * Note that only the last malloc()/realloc() pointer is saved, so
         * if environ has since been changed the old space will be wasted.
         */
        if (environ == allocp)
                newenv = (char **) realloc((void *) environ,
                                ((count+2)*sizeof(char *)));
        else
                newenv = (char **) malloc(((count+2)*sizeof(char *)));

        if (newenv == NULL)
                return -1;

        if (environ != allocp)
        {
                for (n = 0; environ[n] != NULL; n++)
                        newenv[n] = environ[n];
                allocp = newenv;
        }
        newenv[count] = envstr;
        newenv[count+1] = NULL;
        environ = newenv;

        return 0;
}


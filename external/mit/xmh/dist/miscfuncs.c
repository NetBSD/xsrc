/* $XConsortium: miscfuncs.c,v 1.7 94/12/01 17:15:05 kaleb Exp $ */
/* $XFree86: xc/programs/xmh/miscfuncs.c,v 3.6 2001/10/28 03:34:39 tsi Exp $ */

#include "xmh.h"

#include <X11/Xos.h>

#ifndef X_NOT_POSIX
#include <dirent.h>
#else
#include <sys/dir.h>
#ifndef dirent
#define dirent direct
#endif
#endif

#include <stdlib.h>



/*
**  This code is by Rich Salz (rsalz@bbn.com), and ported to SVR4
**  by David Elliott (dce@smsc.sony.com).  No copyrights were found
**  in the original.  Subsequently modified by Bob Scheifler.
*/

/* A convenient shorthand. */
typedef struct dirent	 ENTRY;

/* Initial guess at directory size. */
#define INITIAL_SIZE	20

static int StrCmp(char **a, char **b)
{
    return strcmp(*a, *b);
}

int
ScanDir(
    const char		  *Name,
    char		***List,
    int			 (*Selector)(char *))
{
    register char	 **names;
    register ENTRY	  *E;
    register DIR	  *Dp = NULL;
    register size_t	   i = 0;
    register size_t	   size;

    /* Get initial list space and open directory. */
    size = INITIAL_SIZE;
    if (!(names = malloc(size * sizeof(char *))) ||
	!(Dp = opendir(Name)))
	goto failure;

    /* Read entries in the directory. */
    while ((E = readdir(Dp))) {
	if (!Selector || (*Selector)(E->d_name)) {
	    /* User wants them all, or he wants this one. */
	    if (++i >= size) {
		char **newnames = NULL;
		size <<= 1;
		newnames = realloc(names, size * sizeof(char*));
		if (!newnames) {
		    i--;
		    goto failure;
		}
		names = newnames;
	    }

	    /* Copy the entry. */
	    names[i - 1] = strdup(E->d_name);
	    if (names[i - 1] == NULL) {
		goto failure;
	    }
	}
    }

    /* Close things off. */
    names[i] = (char *)0;
    *List = names;
    closedir(Dp);

    /* Sort? */
    if (i)
	qsort((char *)names, i, sizeof(char *),
	      (int (*)(const void *, const void *))StrCmp);

    return(i);

  failure:
    for (size_t n = 0; n < i; n++) {
	free(names[i]);
    }
    free(names);
    if (Dp != NULL)
	closedir(Dp);
    return(-1);
}

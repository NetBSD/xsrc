/* $Xorg: lndir.c,v 1.5 2001/02/09 02:03:17 xorgcvs Exp $ */
/* Create shadow link tree (after X11R4 script of the same name)
   Mark Reinhold (mbr@lcs.mit.edu)/3 January 1990 */

/* 
Copyright (c) 1990, 1998 The Open Group

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

*/
/* $XFree86: xc/config/util/lndir.c,v 3.18 2003/06/24 15:44:45 eich Exp $ */

/* From the original /bin/sh script:

  Used to create a copy of the a directory tree that has links for all
  non-directories (except, by default, those named BitKeeper, .git, .hg,
  RCS, SCCS, .svn, CVS or CVS.adm).

  If you are building the distribution on more than one machine,
  you should use this technique.

  If your master sources are located in /usr/local/src/X and you would like
  your link tree to be in /usr/local/src/new-X, do the following:

   	%  mkdir /usr/local/src/new-X
	%  cd /usr/local/src/new-X
   	%  lndir ../X
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/Xos.h>
#include <X11/Xfuncproto.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/param.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 2048
#endif

#include <stdarg.h>

static int silent = 0;			/* -silent */
static int ignore_links = 0;		/* -ignorelinks */
static int with_revinfo = 0;		/* -withrevinfo */

static char *rcurdir;
static char *curdir;

static void _X_ATTRIBUTE_PRINTF(2,3) _X_NORETURN
quit (int code, const char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf (stderr, fmt, args);
    va_end(args);
    putc ('\n', stderr);
    exit (code);
}

static void _X_NORETURN
quiterr (int code, const char *s)
{
    perror (s);
    exit (code);
}

static void _X_ATTRIBUTE_PRINTF(1,2)
msg (const char * fmt, ...)
{
    va_list args;
    if (curdir) {
	fprintf (stderr, "%s:\n", curdir);
	curdir = 0;
    }
    va_start(args, fmt);
    vfprintf (stderr, fmt, args);
    va_end(args);
    putc ('\n', stderr);
}

static void
mperror (const char *s)
{
    if (curdir) {
	fprintf (stderr, "%s:\n", curdir);
	curdir = 0;
    }
    perror (s);
}


static int 
equivalent(char *lname, const char *rname, char **p)
{
    char *s;

    if (!strcmp(lname, rname))
	return 1;
    for (s = lname; *s && (s = strchr(s, '/')); s++) {
	while (s[1] == '/') {
	    size_t len = strlen(s+2);
	    memmove(s+1, s+2, len+1);
	    if (p && *p) (*p)--;
	}
    }
    return !strcmp(lname, rname);
}


/* Recursively create symbolic links from the current directory to the "from"
   directory.  Assumes that files described by fs and ts are directories. */
static int
dodir (const char *fn,		/* name of "from" directory, either absolute or
				   relative to cwd */
       struct stat *fs, 
       struct stat *ts,		/* stats for the "from" directory and cwd */
       int rel)			/* if true, prepend "../" to fn before using */
{
    DIR *df;
    struct dirent *dp;
    char buf[MAXPATHLEN + 1], *p;
    char symbuf[MAXPATHLEN + 1];
    char basesym[MAXPATHLEN + 1];
    struct stat sb, sc;
    unsigned int n_dirs;
    ssize_t symlen;
    ssize_t basesymlen = -1;
    char *ocurdir;

    if ((fs->st_dev == ts->st_dev) && (fs->st_ino == ts->st_ino)) {
	msg ("%s: From and to directories are identical!", fn);
	return 1;
    }

    if (rel)
#ifdef HAVE_STRLCPY
	strlcpy (buf, "../", sizeof(buf));
#else
	strcpy (buf, "../");
#endif
    else
	buf[0] = '\0';
#ifdef HAVE_STRLCAT
    if (strlcat (buf, fn, sizeof(buf)) >= sizeof(buf))
        quit(1, "Pathname too long: %s", fn);
#else
    strcat (buf, fn);
#endif
    
    if (!(df = opendir (buf))) {
	msg ("%s: Cannot opendir", buf);
	return 1;
    }

    p = buf + strlen (buf);
    if (*(p - 1) != '/')
	*p++ = '/';
    n_dirs = fs->st_nlink;
    if (n_dirs == 1)
	n_dirs = INT_MAX;
    while ((dp = readdir (df))) {
	if (dp->d_name[strlen(dp->d_name) - 1] == '~')
	    continue;
#ifdef __APPLE__
	/* Ignore these Mac OS X Finder data files */
	if (!strcmp(dp->d_name, ".DS_Store") || 
	    !strcmp(dp->d_name, "._.DS_Store")) 
	    continue;
#endif

#ifdef HAVE_STRLCAT
	*p = '\0';
	if (strlcat (buf, dp->d_name, sizeof(buf)) >= sizeof(buf)) {
	    *p = '\0';
	    quit(1, "Pathname too long: %s%s", buf, dp->d_name);
	}
#else
	strcpy (p, dp->d_name);
#endif

	if (n_dirs > 0) {
	    if (lstat (buf, &sb) < 0) {
		mperror (buf);
		continue;
	    }

	    if (S_ISDIR(sb.st_mode))
	    {
		/* directory */
		n_dirs--;
		if (dp->d_name[0] == '.' &&
		    (dp->d_name[1] == '\0' || (dp->d_name[1] == '.' &&
					       dp->d_name[2] == '\0')))
		    continue;
		if (!with_revinfo) {
                    if (dp->d_name[0] == '.') {
                        if (!strcmp (dp->d_name, ".git"))
                            continue;
                        if (!strcmp (dp->d_name, ".hg"))
                            continue;
                        if (!strcmp (dp->d_name, ".svn"))
                            continue;
                    } else {
                        if (!strcmp (dp->d_name, "BitKeeper"))
                            continue;
                        if (!strcmp (dp->d_name, "RCS"))
                            continue;
                        if (!strcmp (dp->d_name, "SCCS"))
                            continue;
                        if (!strcmp (dp->d_name, "CVS"))
                            continue;
                        if (!strcmp (dp->d_name, "CVS.adm"))
                            continue;
                    }
                }
		ocurdir = rcurdir;
		rcurdir = buf;
		curdir = silent ? buf : (char *)0;
		if (!silent)
		    printf ("%s:\n", buf);
		if ((stat (dp->d_name, &sc) < 0) && (errno == ENOENT)) {
		    if (mkdir (dp->d_name, 0777) < 0 ||
			stat (dp->d_name, &sc) < 0) {
			mperror (dp->d_name);
			curdir = rcurdir = ocurdir;
			continue;
		    }
		}
		if (readlink (dp->d_name, symbuf, sizeof(symbuf) - 1) >= 0) {
		    msg ("%s: is a link instead of a directory", dp->d_name);
		    curdir = rcurdir = ocurdir;
		    continue;
		}
		if (chdir (dp->d_name) < 0) {
		    mperror (dp->d_name);
		    curdir = rcurdir = ocurdir;
		    continue;
		}
		dodir (buf, &sb, &sc, (buf[0] != '/'));
		if (chdir ("..") < 0)
		    quiterr (1, "..");
		curdir = rcurdir = ocurdir;
		continue;
	    }
	}

	/* non-directory */
	symlen = readlink (dp->d_name, symbuf, sizeof(symbuf) - 1);
	if (symlen >= 0)
	    symbuf[symlen] = '\0';

	/* The option to ignore links exists mostly because
	   checking for them slows us down by 10-20%.
	   But it is off by default because this really is a useful check. */
	if (!ignore_links) {
	    /* see if the file in the base tree was a symlink */
	    basesymlen = readlink(buf, basesym, sizeof(basesym) - 1);
	    if (basesymlen >= 0)
		basesym[basesymlen] = '\0';
	}

	if (symlen >= 0) {
	    /* Link exists in new tree.  Print message if it doesn't match. */
	    if (!equivalent (basesymlen>=0 ? basesym : buf, symbuf,
			     basesymlen>=0 ? (char **) 0 : &p))
		msg ("%s: Keeping existing link to %s", dp->d_name, symbuf);
	} else {
	    char *sympath;

	    if (basesymlen>=0) {
		if ((buf[0] == '.') && (buf[1] == '.') && (buf[2] == '/') &&
		    (basesym[0] == '.') && (basesym[1] == '.') &&
		    (basesym[2] == '/')) {
		    /* It becomes very tricky here. We have
		       ../../bar/foo symlinked to ../xxx/yyy. We
		       can't just use ../xxx/yyy. We have to use
		       ../../bar/foo/../xxx/yyy.  */

		    int i;
		    char *start, *end;

#ifdef HAVE_STRLCPY
		    if (strlcpy (symbuf, buf, sizeof(symbuf)) >= sizeof(symbuf))
			quit(1, "Pathname too long: %s", buf);
#else
		    strcpy (symbuf, buf);
#endif
		    /* Find the first char after "../" in symbuf.  */
		    start = symbuf;
		    do {
			start += 3;
		    } while ((start[0] == '.') && (start[1] == '.') &&
			     (start[2] == '/'));

		    /* Then try to eliminate "../"s in basesym.  */
		    i = 0;
		    end = strrchr (symbuf, '/');
		    if (start < end) {
			do {
			    i += 3;
			    end--;
			    while ((*end != '/') && (end != start))
				end--;
			    if (end == start)
				break;
			} while ((basesym[i] == '.') &&
				 (basesym[i + 1] == '.') &&
				 (basesym[i + 2] == '/'));
		    }
		    if (*end == '/')
			end++;
#ifdef HAVE_STRLCPY
		    *end = '\0';
		    if (strlcat (symbuf, &basesym[i], sizeof(symbuf)) >=
			sizeof(symbuf)) {
			*end = '\0';
			quit(1, "Pathname too long: %s%s", symbuf, &basesym[i]);
		    }
#else
		    strcpy (end, &basesym[i]);
#endif
		    sympath = symbuf;
		}
		else
		    sympath = basesym;
	    }
	    else
		sympath = buf;
	    if (symlink (sympath, dp->d_name) < 0)
		mperror (dp->d_name);
	}
    }

    closedir (df);
    return 0;
}

int
main (int argc, char *argv[])
{
    char *prog_name = argv[0];
    const char *fn, *tn;
    struct stat fs, ts;

    while (++argv, --argc) {
	if (strcmp(*argv, "-silent") == 0)
	    silent = 1;
	else if (strcmp(*argv, "-ignorelinks") == 0)
	    ignore_links = 1;
	else if (strcmp(*argv, "-withrevinfo") == 0)
	    with_revinfo = 1;
	else if (strcmp(*argv, "--") == 0) {
	    ++argv, --argc;
	    break;
	}
	else
	    break;
    }

    if (argc < 1 || argc > 2)
	quit (1,
	      "usage: %s [-silent] [-ignorelinks] [-withrevinfo] fromdir [todir]",
	      prog_name);

    fn = argv[0];
    if (argc == 2)
	tn = argv[1];
    else
	tn = ".";

    /* to directory */
    if (stat (tn, &ts) < 0)
	quiterr (1, tn);
    if (!(S_ISDIR(ts.st_mode)))
	quit (2, "%s: Not a directory", tn);
    if (chdir (tn) < 0)
	quiterr (1, tn);

    /* from directory */
    if (stat (fn, &fs) < 0)
	quiterr (1, fn);
    if (!(S_ISDIR(fs.st_mode)))
	quit (2, "%s: Not a directory", fn);

    exit (dodir (fn, &fs, &ts, 0));
}

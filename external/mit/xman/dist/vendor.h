/*

Copyright (c) 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/* Vendor-specific definitions */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef CSRG_BASED
#include <sys/param.h>
#endif

#define SUFFIX "suffix"
#define FOLD "fold"
#define FOLDSUFFIX "foldsuffix"
#define MNULL 0
#define MSUFFIX 1
#define MFOLD 2
#define MFOLDSUFFIX 3

/*
 * The directories to search.  Assume that the manual directories are more
 * complete than the cat directories.
 */

#define SEARCHDIR  MAN
#define SEARCHOTHER CAT

/*
 * The default manual page directory.
 *
 * The MANPATH environment variable will override this.
 */

#ifndef SYSMANPATH

#if defined(__OpenBSD__) || defined(__DARWIN__)
#  define SYSMANPATH "/usr/share/man:/usr/local/man:/usr/X11R6/man"
#elif defined(SVR4) && defined(sun)
#  define SYSMANPATH "/usr/share/man:/usr/X11/man:/usr/openwin/share/man:/usr/dt/share/man:/usr/sfw/share/man"
#endif

#ifndef SYSMANPATH
#  define SYSMANPATH "/usr/share/man"
#endif

#endif

/*
 * Compression Definitions.
 */

#define COMPRESSION_EXTENSION "Z"
#define UNCOMPRESS_FORMAT     "zcat < %s >> %s"
#define COMPRESS              "compress"
#define GZIP_EXTENSION "gz"
#define GUNZIP_FORMAT "gzip -c -d < %s >> %s"
#define GZIP_COMPRESS "gzip"
#define BZIP2_EXTENSION "bz2"
#define BUNZIP2_FORMAT "bunzip2 -c -d < %s >> %s"
#define BZIP2_COMPRESS "bzip2"
#define LZMA_EXTENSION "lzma"
#define UNLZMA_FORMAT "unlzma -c -d < %s >> %s"
#define LZMA_COMPRESS "lzma"


/*
 * The command filters for the manual and apropos searches.
 */

#define APROPOS_FORMAT ("man -M %s -k %s | pr -h Apropos >> %s")

#ifndef HANDLE_ROFFSEQ
# if defined(CSRG_BASED)
#  define FORMAT "| eqn | tbl | nroff -mandoc"
# elif defined(linux) || defined(__CYGWIN__)
#  define FORMAT "| pic | eqn | tbl -Tlatin1 | GROFF_NO_SGR= groff -Tlatin1 -mandoc"
# else
#  define FORMAT "| neqn | nroff -man"  /* The format command. */
# endif
# define TBL "tbl"
#else                           /* HANDLE_ROFFSEQ */
# if defined(linux)
#  define ZSOELIM	"zsoelim"
# else
#  define ZSOELIM	"soelim"
#endif
# define EQN		"eqn"
# define TBL		"tbl"
# define GRAP		"grap"
# define ROFF_PIC	"pic"
# define VGRIND		"vgrind"
# define REFER		"refer"
# if defined(CSRG_BASED)
#  define FORMAT	"nroff -mandoc"
# elif defined(linux) || defined(__CYGWIN__)
#  define FORMAT	"GROFF_NO_SGR= groff -Tlatin1 -mandoc"
# elif defined(__DARWIN__)
#  define FORMAT	"nroff -man"
# else
#  define FORMAT	"GROFF_NO_SGR= groff -Tlatin1 -man"
# endif
# define DEFAULT_MANROFFSEQ "et"
#endif                          /*HANDLE_ROFFSEQ */

/*
 * Names of the man and cat dirs.
 */

#define MAN "man"
#define CAT "cat"

/* Solaris has nroff man pages in "man" and sgml man pages in "sman" */
#if defined(sun) && defined(SVR4)
#  define SFORMAT		"/usr/lib/sgml/sgml2roff"
#  define SMAN			"sman"
#  undef SEARCHOTHER
#  define SEARCHOTHER 		SMAN
#  define SGMLENT_EXTENSION	"ent"   /* SGML entity files end in ".ent" */
#endif


typedef struct _SectionList {
    struct _SectionList *next;
    char *label;                /* section label */
    char *directory;            /* section directory */
    int flags;
} SectionList;

extern char *CreateManpageName(const char *entry, int section, int flags);
extern void AddStandardSections(SectionList ** list, const char *path);
extern void AddNewSection(SectionList ** list, const char *path,
                          const char *file, const char *label, int flags);

/*
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * Copyright 1992 Claude Lecommandeur.
 */

/***********************************************************************
 *
 * $XConsortium: parse.c,v 1.52 91/07/12 09:59:37 dave Exp $
 *
 * parse the .twmrc file
 *
 * 17-Nov-87 Thomas E. LaStrange       File created
 * 10-Oct-90 David M. Sternlicht       Storing saved colors on root
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 ***********************************************************************/

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#ifdef USEM4
# include <sys/types.h>
# include <sys/wait.h>
#endif

#include "ctwm_atoms.h"
#include "screen.h"
#include "parse.h"
#include "parse_int.h"
#include "deftwmrc.h"
#ifdef SOUNDS
#  include "sound.h"
#endif

#ifndef SYSTEM_INIT_FILE
#error "No SYSTEM_INIT_FILE set"
#endif

static bool ParseStringList(const char **sl);

/*
 * With current bison, this is defined in the gram.tab.h, so this causes
 * a warning for redundant declaration.  With older bisons and byacc,
 * it's not, so taking it out causes a warning for implicit declaration.
 * A little looking around doesn't show any handy #define's we could use
 * to be sure of the difference.  This should quiet it down on gcc/clang
 * anyway...
 */
#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wredundant-decls"
extern int yyparse(void);
# pragma GCC diagnostic pop
#else
extern int yyparse(void);
#endif

// Because of how this winds up shared with callback funcs in the
// parsing, it's difficult to unwind from being global, so just accept
// it.
static FILE *twmrc;

static int ptr = 0;
static int len = 0;
#define BUF_LEN 300
static char buff[BUF_LEN + 1];
static const char **stringListSource, *currentString;

#ifdef NON_FLEX_LEX
/*
 * While these are (were) referenced in a number of places through the
 * file, overflowlen is initialized to 0, only possibly changed in
 * twmUnput(), and unless it's non-zero, neither is otherwise touched.
 * So this is purely a twmUnput()-related var, and with flex, never used
 * for anything.
 */
static char overflowbuff[20];           /* really only need one */
static int overflowlen;
#endif

int ConstrainedMoveTime = 400;          /* milliseconds, event times */
bool ParseError;                        /* error parsing the .twmrc file */
int RaiseDelay = 0;                     /* msec, for AutoRaise */
int (*twmInputFunc)(void);              /* used in lexer */

static int twmrc_lineno;


/* Actual file loader */
static int ParseTwmrc(const char *filename);

/* lex plumbing funcs */
static bool doparse(int (*ifunc)(void), const char *srctypename,
                    const char *srcname);

static int twmStringListInput(void);
#ifndef USEM4
static int twmFileInput(void);
#else
static int m4twmFileInput(void);
#endif

#if defined(YYDEBUG) && YYDEBUG
int yydebug = 1;
#endif


/**
 * Principal entry point from top-level code to parse the config file.
 * This tries the various permutations of config files we could load.
 * For most possible names, we try loading `$NAME.$SCREENNUM` before
 * trying `$NAME`.  If a `-f filename` is given on the command line, it's
 * passed in here, and the normal `~/.[c]twmrc*` attempts are skipped if
 * it's not found.
 *
 * \param filename A filename given in the -f command-line argument (or
 * NULL)
 * \return true/false for whether a valid config was parsed out from
 * somewhere.
 */
bool
LoadTwmrc(const char *filename)
{
	int ret = -1;
	char *tryname = NULL;

	/*
	 * Check for the twmrc file in the following order:
	 *   0.  -f filename.#
	 *   1.  -f filename
	 *       (skip to 6 if -f was given)
	 *   2.  .ctwmrc.#
	 *   3.  .ctwmrc
	 *   4.  .twmrc.#
	 *   5.  .twmrc
	 *   6.  system.ctwmrc
	 */
#define TRY(fn) if((ret = ParseTwmrc(fn)) != -1) { goto DONE_TRYING; }  (void)0

	if(filename) {
		/* -f filename.# */
		asprintf(&tryname, "%s.%d", filename, Scr->screen);
		if(tryname == NULL) {
			// Oh, we're _screwed_...
			return false;
		}
		TRY(tryname);

		/* -f filename */
		TRY(filename);

		/* If we didn't get either from -f, don't try the ~ bits */
		goto TRY_FALLBACK;
	}

	if(Home) {
		/* ~/.ctwmrc.screennum */
		free(tryname);
		asprintf(&tryname, "%s/.ctwmrc.%d", Home, Scr->screen);
		if(tryname == NULL) {
			return false;
		}
		TRY(tryname);

		// All later attempts are guaranteed shorter strings than that,
		// so we can just keep sprintf'ing over it.

		/* ~/.ctwmrc */
		sprintf(tryname, "%s/.ctwmrc", Home);
		TRY(tryname);

		/* ~/.twmrc.screennum */
		sprintf(tryname, "%s/.twmrc.%d", Home, Scr->screen);
		TRY(tryname);

		/* ~/.twmrc */
		sprintf(tryname, "%s/.twmrc", Home);
		TRY(tryname);
	}

TRY_FALLBACK:
	/* system.twmrc */
	if((ret = ParseTwmrc(SYSTEM_INIT_FILE)) != -1) {
		if(ret && filename) {
			// If we were -f'ing, fell back to the system default, and
			// that succeeeded, we warn.  It's "normal"(ish) to not have
			// a personal twmrc and fall back...
			fprintf(stderr,
			        "%s:  unable to open twmrc file %s, using %s instead\n",
			        ProgramName, filename, SYSTEM_INIT_FILE);
		}
		goto DONE_TRYING;
	}


DONE_TRYING:
#undef TRY
	free(tryname);

	/*
	 * If we wound up with -1 all the way, we totally failed to find a
	 * file to work with.  Fall back to builtin config.
	 */
	if(ret == -1) {
		// Only warn if -f.
		if(filename) {
			fprintf(stderr,
			        "%s:  unable to open twmrc file %s, using built-in defaults instead\n",
			        ProgramName, filename);
		}
		return ParseStringList(defTwmrc);
	}


	/* Better have a useful value in ret... */
	return ret;
}


/**
 * Try parsing a file as a ctwmrc.
 *
 * \param filename The filename to try opening and parsing.
 * \return -1,0,1.  0/1 should be treated as false/true for whether
 * parsing the file succeeded.  -1 means the file couldn't be opened.
 */
static int
ParseTwmrc(const char *filename)
{
	bool status;

#if 0
	fprintf(stderr, "%s(): Trying %s\n", __func__, filename);
#endif

	/* See if we can open the file */
	if(!filename) {
		return -1;
	}
	twmrc = fopen(filename, "r");
	if(!twmrc) {
		return -1;
	}


	/* Got it.  Kick off the parsing, however we do it. */
#ifdef USEM4
	FILE *raw = NULL;
	if(CLarg.GoThroughM4) {
		/*
		 * Hold onto raw filehandle so we can fclose() it below, and
		 * swap twmrc over to the output from m4
		 */
		raw = twmrc;
		twmrc = start_m4(raw);
	}
	status = doparse(m4twmFileInput, "file", filename);
	wait(0);
	fclose(twmrc);
	if(raw) {
		fclose(raw);
	}
#else
	status = doparse(twmFileInput, "file", filename);
	fclose(twmrc);
#endif

	/* And we're done */
	return status;

	/* NOTREACHED */
}

static bool
ParseStringList(const char **sl)
{
	stringListSource = sl;
	currentString = *sl;
	return doparse(twmStringListInput, "string list", NULL);
}


/*
 * Util used throughout the code (possibly often wrongly?)
 */
void twmrc_error_prefix(void)
{
	fprintf(stderr, "%s:  line %d:  ", ProgramName, twmrc_lineno);
}



/*
 * Everything below here is related to plumbing and firing off lex/yacc
 */


/*
 * Backend func that takes an input-providing func and hooks it up to the
 * lex/yacc parser to do the work
 */
static bool
doparse(int (*ifunc)(void), const char *srctypename,
        const char *srcname)
{
	ptr = 0;
	len = 0;
	twmrc_lineno = 0;
	ParseError = false;
	twmInputFunc = ifunc;
#ifdef NON_FLEX_LEX
	overflowlen = 0;
#endif

	yyparse();

	if(ParseError) {
		fprintf(stderr, "%s:  errors found in twm %s",
		        ProgramName, srctypename);
		if(srcname) {
			fprintf(stderr, " \"%s\"", srcname);
		}
		fprintf(stderr, "\n");
	}
	return !(ParseError);
}


/*
 * Various input routines for the lexer for the various sources of
 * config.
 */

#ifndef USEM4
#include <ctype.h>

/* This has Tom's include() funtionality.  This is utterly useless if you
 * can use m4 for the same thing.               Chris P. Ross */

#define MAX_INCLUDES 10

static struct incl {
	FILE *fp;
	char *name;
	int lineno;
} rc_includes[MAX_INCLUDES];
static int include_file = 0;


static int twmFileInput(void)
{
#ifdef NON_FLEX_LEX
	if(overflowlen) {
		return (int) overflowbuff[--overflowlen];
	}
#endif

	while(ptr == len) {
		while(include_file) {
			if(fgets(buff, BUF_LEN, rc_includes[include_file].fp) == NULL) {
				free(rc_includes[include_file].name);
				fclose(rc_includes[include_file].fp);
				twmrc_lineno = rc_includes[include_file--].lineno;
			}
			else {
				break;
			}
		}

		if(!include_file)
			if(fgets(buff, BUF_LEN, twmrc) == NULL) {
				return 0;
			}
		twmrc_lineno++;

		if(strncmp(buff, "include", 7) == 0) {
			/* Whoops, an include file! */
			char *p = buff + 7, *q;
			FILE *fp;

			while(isspace(*p)) {
				p++;
			}
			for(q = p; *q && !isspace(*q); q++) {
				continue;
			}
			*q = 0;

			if((fp = fopen(p, "r")) == NULL) {
				fprintf(stderr, "%s: Unable to open included init file %s\n",
				        ProgramName, p);
				continue;
			}
			if(++include_file >= MAX_INCLUDES) {
				fprintf(stderr, "%s: init file includes nested too deep\n",
				        ProgramName);
				continue;
			}
			rc_includes[include_file].fp = fp;
			rc_includes[include_file].lineno = twmrc_lineno;
			twmrc_lineno = 0;
			rc_includes[include_file].name = strdup(p);
			continue;
		}
		ptr = 0;
		len = strlen(buff);
	}
	return ((int)buff[ptr++]);
}
#else /* USEM4 */
/* If you're going to use m4, use this version instead.  Much simpler.
 * m4 ism's credit to Josh Osborne (stripes) */

static int m4twmFileInput(void)
{
	int line;
	static FILE *cp = NULL;

	if(cp == NULL && CLarg.keepM4_filename) {
		cp = fopen(CLarg.keepM4_filename, "w");
		if(cp == NULL) {
			fprintf(stderr,
			        "%s:  unable to create m4 output %s, ignoring\n",
			        ProgramName, CLarg.keepM4_filename);
			CLarg.keepM4_filename = NULL;
		}
	}

#ifdef NON_FLEX_LEX
	if(overflowlen) {
		return((int) overflowbuff[--overflowlen]);
	}
#endif

	while(ptr == len) {
nextline:
		if(fgets(buff, BUF_LEN, twmrc) == NULL) {
			if(cp) {
				fclose(cp);
			}
			return(0);
		}
		if(cp) {
			fputs(buff, cp);
		}

		if(sscanf(buff, "#line %d", &line)) {
			twmrc_lineno = line - 1;
			goto nextline;
		}
		else {
			twmrc_lineno++;
		}

		ptr = 0;
		len = strlen(buff);
	}
	return ((int)buff[ptr++]);
}
#endif /* USEM4 */


static int twmStringListInput(void)
{
#ifdef NON_FLEX_LEX
	if(overflowlen) {
		return (int) overflowbuff[--overflowlen];
	}
#endif

	/*
	 * return the character currently pointed to
	 */
	if(currentString) {
		unsigned int c = (unsigned int) * currentString++;

		if(c) {
			return c;        /* if non-nul char */
		}
		currentString = *++stringListSource;  /* advance to next bol */
		return '\n';                    /* but say that we hit last eol */
	}
	return 0;                           /* eof */
}



/*
 * unput/output funcs for AT&T lex.  No longer supported, and expected to
 * be GC'd in a release or two.
 */
#ifdef NON_FLEX_LEX

void twmUnput(int c)
{
	if(overflowlen < sizeof overflowbuff) {
		overflowbuff[overflowlen++] = (char) c;
	}
	else {
		twmrc_error_prefix();
		fprintf(stderr, "unable to unput character (%c)\n",
		        c);
	}
}

void TwmOutput(int c)
{
	putchar(c);
}

#endif /* NON_FLEX_LEX */

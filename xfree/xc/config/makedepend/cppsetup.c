/* $Xorg: cppsetup.c,v 1.5 2001/02/09 02:03:16 xorgcvs Exp $ */
/*

Copyright (c) 1993, 1994, 1998  The Open Group

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
/* $XFree86: xc/config/makedepend/cppsetup.c,v 3.12 2004/03/05 16:02:58 tsi Exp $ */

#include "def.h"

#ifdef	CPP
/*
 * This file is strictly for the sake of cpy.y and yylex.c (if
 * you indeed have the source for cpp).
 */
#define IB 1
#define SB 2
#define NB 4
#define CB 8
#define QB 16
#define WB 32
#define SALT '#'
#if defined(pdp11) || defined(vax) || defined(ns16000) || defined(mc68000) || defined(ibm032)
#define COFF 128
#else
#define COFF 0
#endif
/*
 * These variables used by cpy.y and yylex.c
 */
extern char	*outp, *inp, *newp, *pend;
extern char	*ptrtab;
extern char	fastab[];
extern char	slotab[];

/*
 * cppsetup
 */
struct filepointer	*currentfile;
struct inclist		*currentinc;

int
cppsetup(char *line, struct filepointer *filep, struct inclist *inc)
{
	char *p, savec;
	static boolean setupdone = FALSE;
	boolean	value;

	if (!setupdone) {
		cpp_varsetup();
		setupdone = TRUE;
	}

	currentfile = filep;
	currentinc = inc;
	inp = newp = line;
	for (p=newp; *p; p++)
		;

	/*
	 * put a newline back on the end, and set up pend, etc.
	 */
	*p++ = '\n';
	savec = *p;
	*p = '\0';
	pend = p;

	ptrtab = slotab+COFF;
	*--inp = SALT;
	outp=inp;
	value = yyparse();
	*p = savec;
	return(value);
}

struct symtab **lookup(symbol)
	char	*symbol;
{
	static struct symtab    *undefined;
	struct symtab   **sp;

	sp = isdefined(symbol, currentinc, NULL);
	if (sp == NULL) {
		sp = &undefined;
		(*sp)->s_value = NULL;
	}
	return (sp);
}

pperror(tag, x0,x1,x2,x3,x4)
	int	tag,x0,x1,x2,x3,x4;
{
	warning("\"%s\", line %d: ", currentinc->i_file, currentfile->f_line);
	warning(x0,x1,x2,x3,x4);
}


yyerror(s)
	register char	*s;
{
	fatalerr("Fatal error: %s\n", s);
}
#else /* not CPP */

#include "ifparser.h"
struct _parse_data {
    struct filepointer *filep;
    struct inclist *inc;
    char *filename;
    const char *line;
};

static const char *
my_if_errors (IfParser *ip, const char *cp, const char *expecting)
{
    struct _parse_data *pd = (struct _parse_data *) ip->data;
    int lineno = pd->filep->f_line;
    char *filename = pd->filename;
    char prefix[300];
    int prefixlen;
    int i;

    sprintf (prefix, "\"%s\":%d", filename, lineno);
    prefixlen = strlen(prefix);
    fprintf (stderr, "%s:  %s", prefix, pd->line);
    i = cp - pd->line;
    if (i > 0 && pd->line[i-1] != '\n') {
	putc ('\n', stderr);
    }
    for (i += prefixlen + 3; i > 0; i--) {
	putc (' ', stderr);
    }
    fprintf (stderr, "^--- expecting %s\n", expecting);
    return NULL;
}


#define MAXNAMELEN 256

static struct symtab **
lookup_variable (IfParser *ip, const char *var, int len)
{
    char tmpbuf[MAXNAMELEN + 1];
    struct _parse_data *pd = (struct _parse_data *) ip->data;

    if (len > MAXNAMELEN)
	return 0;

    strncpy (tmpbuf, var, len);
    tmpbuf[len] = '\0';
    return isdefined (tmpbuf, pd->inc, NULL);
}


static int
my_eval_defined (IfParser *ip, const char *var, int len)
{
    if (lookup_variable (ip, var, len))
	return 1;
    else
	return 0;
}


int
variable_has_args (IfParser *ip, const char *var, int len)
{
    struct symtab **s = lookup_variable (ip, var, len);

    if (!s)
	return 0;

    if ((*s)->s_args)
	return 1;
    else
	return 0;
}

/*
 * this is tiny linked list implementation for temporarily storing
 * and retriving pairs of macro parameter names and passed in macro arguments.
 */
typedef struct keyword_type_rec keyword_type;
struct keyword_type_rec {
    keyword_type* pnext;
    char *name;
    char *value;
};


static keyword_type*
build_keyword_list (const char* keys, const char* values)
{
    keyword_type *phead = NULL, *pnew;
    const char *ptmp;
    int len;

    while (*keys)
    {
	/* alloc new member */
	pnew = malloc(sizeof(*pnew));
	if (!pnew)
	{
	    fprintf(stderr, "out of memory in my_eval_variable\n");
	    exit(1);
	}

	/* extract key */
	ptmp = keys;
	len = 0;
	while (*ptmp && (*ptmp != ','))
	    ptmp++, len++;
	pnew->name = malloc(len+1);
	strncpy(pnew->name, keys, len);
	pnew->name[len] = '\0';
	keys = ptmp;
	if (*keys)
	    keys++;

	/* extract arg */
	ptmp = values;
	len = 0;
	while (*ptmp && (*ptmp != ',') && (*ptmp != ')'))
	    ptmp++, len++;
	pnew->value = malloc(len+1);
	strncpy(pnew->value, values, len);
	pnew->value[len] = '\0';
	values = ptmp;
	if (*values)
	    values++;

	/* chain in this new member */
	pnew->pnext = phead;
	phead = pnew;
    }

    return phead;
}


static const keyword_type*
get_keyword_entry (const keyword_type* phead, const char* keyname, const int keylen)
{
    while (phead)
    {
	if (keylen == strlen(phead->name))
	    if (strncmp(keyname, phead->name, keylen) == 0)
		return phead;
	phead = phead->pnext;
    }

    return phead;
}


static void
free_keyword_list (keyword_type* phead)
{
    keyword_type* pnext;
    while (phead)
    {
	pnext = phead->pnext;
	free(phead->name);
	free(phead->value);
	free(phead);
	phead = pnext;
    }
}


#define isvarfirstletter(ccc) (isalpha(ccc) || (ccc) == '_')

static long
my_eval_variable (IfParser *ip, const char *var, int len, const char *args)
{
    long val;
    char *newline = NULL;
    int newline_len = 0, newline_offset = 0;
    struct symtab **s;

    s = lookup_variable (ip, var, len);
    if (!s)
	return 0;

    if ((*s)->s_args)
    {
	const char *psrc, *psrc_qualifier;
	char *pdst;
	const keyword_type *pkeyword;
	keyword_type *pkeylist;

	newline_len = 64; /* start with some buffer, might increase later */
	newline = malloc(newline_len);
	if (!newline)
	{
	    fprintf(stderr, "out of memory in my_eval_variable\n");
	    exit(1);
	}

	/* build up a list that pairs keywords and args */
	pkeylist = build_keyword_list((*s)->s_args,args);

	/* parse for keywords in macro content */
	psrc = (*s)->s_value;
	pdst = newline;
	while (*psrc)
	{
	    /* parse for next qualifier */
	    psrc_qualifier = psrc;
	    while (isalnum(*psrc) || *psrc == '_')
		psrc++;

	    /* check if qualifier is in parameter keywords listing of macro */
	    pkeyword = get_keyword_entry(pkeylist,psrc_qualifier,psrc - psrc_qualifier);
	    if (pkeyword)
	    { /* convert from parameter keyword to given argument */
		const char *ptmp = pkeyword->value;
		while (*ptmp)
		{
		    *pdst++ = *ptmp++;
		    newline_offset++;
		    if (newline_offset + 2 >= newline_len)
		    {
			newline_len *= 2;
			newline = realloc(newline, newline_len);
			if (!newline)
			{
			    fprintf(stderr, "out of memory in my_eval_variable\n");
			    exit(1);
			}
			pdst = &newline[newline_offset];
		    }
		}
	    }
	    else
	    { /* perform post copy of qualifier that is not a parameter keyword */
		const char *ptmp = psrc_qualifier;
		while (ptmp < psrc)
		{
		    *pdst++ = *ptmp++;
		    newline_offset++;
		    if (newline_offset + 2 >= newline_len)
		    {
			newline_len *= 2;
			newline = realloc(newline, newline_len);
			if (!newline)
			{
			    fprintf(stderr, "out of memory in my_eval_variable\n");
			    exit(1);
			}
			pdst = &newline[newline_offset];
		    }
		}
	    }

	    /* duplicate chars that are not qualifier chars */
	    while (!(isalnum(*psrc) || *psrc == '_' || *psrc == '\0'))
	    {
		*pdst++ = *psrc++;
		newline_offset++;
		if (newline_offset + 2 >= newline_len)
		{
		    newline_len *= 2;
		    newline = realloc(newline, newline_len);
		    if (!newline)
		    {
			fprintf(stderr, "out of memory in my_eval_variable\n");
			exit(1);
		    }
		    pdst = &newline[newline_offset];
		}
	    }
	}

	*pdst = '\0';
	free_keyword_list(pkeylist);
	var = newline;
    }
    else
    {
       var = (*s)->s_value;
    }

    var = ParseIfExpression(ip, var, &val);
    if (var && *var) debug(4, ("extraneous: '%s'\n", var));

    if (newline) free(newline);

    return val;
}

int
cppsetup(char *filename,
	 char *line,
	 struct filepointer *filep,
	 struct inclist *inc)
{
    IfParser ip;
    struct _parse_data pd;
    long val = 0;

    pd.filep = filep;
    pd.inc = inc;
    pd.line = line;
    pd.filename = filename;
    ip.funcs.handle_error = my_if_errors;
    ip.funcs.eval_defined = my_eval_defined;
    ip.funcs.eval_variable = my_eval_variable;
    ip.data = (char *) &pd;

    (void) ParseIfExpression (&ip, line, &val);
    if (val)
	return IF;
    else
	return IFFALSE;
}
#endif /* CPP */


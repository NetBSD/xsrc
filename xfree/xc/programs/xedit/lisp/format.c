/*
 * Copyright (c) 2001 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from the
 * XFree86 Project.
 *
 * Author: Paulo César Pereira de Andrade
 */

/* $XFree86: xc/programs/xedit/lisp/format.c,v 1.11 2001/10/28 03:34:29 tsi Exp $ */

#include "format.h"
#include <ctype.h>
#include <math.h>

/*
 * Initialization
 */
static char *BadArgument = "bad argument to directive, at %s";
/* not very descriptive... */

extern char *LispCharNames[];

/*
 * Implementation
 */
LispObj *
Lisp_Format(LispMac *mac, LispObj *list, char *fname)
{
    int len, princ = mac->princ, newline = mac->newline, tmp, nindirection = 0;
    int num_args, cur_arg, iteration = -1;
    char *fmt, stk[1024], **indirection = NULL, *ptr, *str = NULL;
    LispObj *stream, *format, *arguments, *arg, *plural, *obj, *ilist, *sargs,
	    *alist;
    unsigned char *uptr;

    sargs = NIL;  /* place to temporarily save arguments, num_args and cur_arg */
    stream = CAR(list);
    list = CDR(list);
    if ((format = CAR(list))->type != LispString_t)
	LispDestroy(mac, "expecting string, at %s", fname);
    list = CDR(list);
    arg = arguments = list;

    /* count number of arguments */
    for (num_args = cur_arg = 0; arg->type == LispCons_t; arg = CDR(arg))
	++num_args;
    arg = arguments;

    if (stream == NIL) {
	stream = LispNew(mac, NIL, NIL);
	stream->type = LispStream_t;
	stream->data.stream.source.str = NULL;
	stream->data.stream.size = 0;
	stream->data.stream.idx = 0;
    }
    else if (stream == T)
	stream = NIL;
    else if (stream->type != LispStream_t)
	LispDestroy(mac, "%s is not a stream, at %s",
		    LispStrObj(mac, stream), fname);

    /* ilist is used to store the previous/original stream and a description
     * of the formating directive.
     */
    ilist = CONS(CONS(NIL, stream), NIL);
    /* alist is used to store saved data, to allow temporarily saving
     * arguments or data when processing complex directives.
     */
    alist = NIL;

    plural = NIL;
    len = 0;
    fmt = STRPTR(format);
    while (1) {
	if (*fmt == '\0') {
	    if (iteration >= 0 && STRPTR(CAR(CAR(ilist)))[0] == '{') {
		int done = 0;

		if (iteration != 0 && --iteration == 0) {
		    iteration = -1;
		    done = 1;
		}
		fmt = indirection[(int)CAR(alist)->data.real + 1];
		if (STRPTR(CAR(CAR(ilist)))[1] == '@') {
		    /* using normal arguments */
		    if (STRPTR(CAR(CAR(ilist)))[2] == ':') {
			if (CDR(CAR(alist))->type == LispCons_t) {
			    CAR(alist) = CDR(CAR(alist));
			    plural = NIL;
			    arg = CAR(CAR(alist));
			    if (arg->type != LispCons_t)
				LispDestroy(mac, BadArgument, fname);
			    arguments = obj = arg;
			    for (num_args = cur_arg = 0; obj->type == LispCons_t;
				 obj = CDR(obj))
				++num_args;
			}
			else if (CDR(CAR(alist)) != NIL)
			    goto not_enough_args;
			else
			    done = 1;	    /* no more arguments */
		    }
		    else if (arg == NIL)
			done = 1;
		}
		else {
		    if (STRPTR(CAR(CAR(ilist)))[1] == ':') {
			if (CDR(CAR(CAR(alist)))->type == LispCons_t) {
			    CAR(CAR(alist)) = CDR(CAR(CAR(alist)));
			    arg = CAR(CAR(CAR(alist)));
			    if (arg->type != LispCons_t)
				LispDestroy(mac, BadArgument, fname);
			    arguments = obj = arg;
			    for (num_args = cur_arg = 0; obj->type == LispCons_t;
				 obj = CDR(obj))
				++num_args;
			}
			else if (CDR(CAR(CAR(alist))) != NIL)
			    goto not_enough_args;
			else
			    done = 1;
		    }
		    else if (arg->type != LispCons_t) {
			arg = CAR(ilist);
			done = 1;
		    }
		    if (arg->type != LispCons_t)
			done = 1;	/* no more arguments */
		}
		if (done) {
		    iteration = -1;
		    nindirection = (int)CAR(alist)->data.real;
		    fmt = indirection[nindirection];
		    if (STRPTR(CAR(CAR(ilist)))[1] == ':' ||
			STRPTR(CAR(CAR(ilist)))[2] == ':') {
			arg = CDR(CDR(CAR(ilist)));
			plural = NIL;
			arguments = CAR(CAR(sargs));
			num_args = (int)CAR(CDR(CAR(sargs)))->data.real;
			cur_arg = (int)CDR(CDR(CAR(sargs)))->data.real;
			sargs = CDR(sargs);
		    }
		    ilist = CDR(ilist);
		    alist = CDR(alist);
		    stream = CDR(CAR(ilist));
		}
	    }
	    else if (nindirection)
		fmt = indirection[--nindirection];
	    else
		break;
	    continue;
	}
	if (*fmt == '~') {
	    char *end;
	    int radix = 10, mincol = 0, minpad = 0, colinc = 1, padchar = ' ';
	    int argc, nargs[7], defs[7], padidx, isradix, isprinc, ise;
	    int atsign = 0, collon = 0;
	    int w = 0, d = 1, e = 1, k = 0, n = 1, overidx, overflowchar = 0,
		expidx, exponentchar = 'E', colnum = 1, hash = -1;

	    if (len) {
		stk[len] = '\0';
		mac->column += LispPrintf(mac, stream, "%s", stk);
		len = 0;
	    }
	    ++fmt;
	    argc = 0;
	    padidx = overidx = expidx = -1;	/* minimal error check */
	    while (1) {
		if (*fmt == ',') {	/* use default value */
		    ++fmt;
		    defs[argc] = 1;
		}
		else if (*fmt == '-' || *fmt == '+' ||
			 isdigit(*fmt)) {	/* mincol specified */
		    nargs[argc] = strtol(fmt, &end, 10);
		    defs[argc] = 0;
		    fmt = end;
		    if (*fmt == ',')	/* more args */
			++fmt;
		}
		else if (*fmt == '\'') {	/* use default value */
		    ++fmt;
		    if (overidx != 1)
			expidx = argc;
		    else {
			overidx = padidx;
			padidx = argc;
		    }
		    if (!*fmt)
			goto error_parsing;
		    nargs[argc] = *fmt++;
		    defs[argc] = 0;
		    if (*fmt == ',')	/* more args */
			++fmt;
		}
		else if (*fmt == ':') {
		    ++fmt;
		    collon = 1;
		    continue;
		}
		else if (*fmt == '@') {
		    ++fmt;
		    atsign = 1;
		    continue;
		}
		else if (*fmt == '#') {
		    nargs[argc] = num_args - cur_arg;
		    ++fmt;
		    hash = argc;
		    defs[argc] = 0;
		    if (*fmt == ',')	/* more args */
			++fmt;
		}
		else
		    break;
		if (++argc > sizeof(nargs) / sizeof(nargs[0]))
		    LispDestroy(mac, "too many arguments to directive, at %s",
				fname);
	    }

	    switch (*fmt) {
		case 'a':	/* Ascii */
		case 'A':	/* ~mincol,colinc,minpad,padcharA */
		    isprinc = 1;
		    goto print_object;
		case 'b':	/* Binary */
		case 'B':	/* ~mincol,padchar,commacharB */
		    isradix = 0;
		    radix = 2;
		    goto print_number;
		case 'w':	/* Write */
		case 'W':	/* XXX must output identically to write */
		case 's':	/* S-expression */
		case 'S':	/* ~mincol,colinc,minpad,padcharS */
		    isprinc = 0;
		    goto print_object;
		case 'd':	/* Decimal */
		case 'D':	/* ~mincol,padcharD */
		    isradix = 0;
		    radix = 10;
		    goto print_number;
		case 'o':	/* Octal */
		case 'O':	/* ~mincol,padchar,commacharB */
		    isradix = 0;
		    radix = 8;
		    goto print_number;
		case 'x':	/* Hexadecimal */
		case 'X':	/* ~mincol,padchar,commacharB */
		    isradix = 0;
		    radix = 16;
		    goto print_number;
		case 'r':	/* Radix */
		case 'R':	/* ~radix,mincol,padchar,commacharR */
		    isradix = 1;
		    goto print_number;
		case 'p':	/* Plural */
		case 'P':
		    mac->newline = 0;
		    if (collon) {
			if (plural != NIL)
			    arg = plural;
			else
			    plural = arg;
			if (plural->type != LispCons_t)
			    goto not_enough_args;
		    }
		    else {
			if (arg->type != LispCons_t)
			    goto not_enough_args;
			plural = arg;
		    }
		    if (!NUMBER_P(CAR(plural)) ||
			NUMBER_VALUE(CAR(plural)) != 1) {
			if (atsign)
			    mac->column += LispPrintf(mac, stream, "ies");
			else
			    mac->column += LispPrintf(mac, stream, "s");
		    }
		    else {
			if (atsign)
			    mac->column += LispPrintf(mac, stream, "y");
		    }
		    break;
		case 'c':	/* Character */
		case 'C':
		    if (arg->type != LispCons_t)
			goto not_enough_args;
		    if (CAR(arg)->type != LispCharacter_t)
			LispDestroy(mac, "expecting character, at %s", fname);
		    mac->newline = atsign || collon ||
				   CAR(arg)->data.integer != '\n';
		    if (atsign && !collon)
			mac->column += LispPrintf(mac, stream, "#\\");
		    if ((atsign || collon) &&
			(CAR(arg)->data.integer <= ' ' ||
			 CAR(arg)->data.integer == 0177))
			mac->column +=
			    LispPrintf(mac, stream, "%s",
				       CAR(arg)->data.integer == 0177 ?
				       "Rubout" :
				       LispCharNames[CAR(arg)->data.integer]);
		    else
			mac->column += LispPrintf(mac, stream, "%c",
						  CAR(arg)->data.integer);
		    break;
		case 'f':	/* Floating-point */
		case 'F':	/* ~w,d,k,overflowchar,padcharF */
		    ise = 0;
		    goto print_float_number;
		case 'e':	/* Exponential floating-point */
		case 'E':	/* ~w,d,e,k,overflowchar,padchar,exponentcharE */
		    ise = 1;
		    goto print_float_number;
		case 'g':	/* General floating-point */
		case 'G':	/* ~w,d,e,k,overflowchar,padchar,exponentcharG */
		    ise = 1;
		    goto print_float_number;
		case '$':	/* Dollars floating-point */
				/* ~d,n,w,padchar$ */
		    ise = 0;
		    goto print_float_number;
		case '&':
		    if (mac->newline)
			len = -1;
		case '%':
		    if (argc && !defs[0])
			len += nargs[0];
		    else
			len += 1;
		    if (padidx >= 0)
			LispDestroy(mac, BadArgument, fname);
		    if (len > 0) {
			mac->newline = 1;
			mac->column = 0;
		    }
		    while (len > 0) {
			LispPrintf(mac, stream, "\n");
			--len;
		    }
		    len = 0;
		    ++fmt;
		    /* no arguments used */
		    continue;
		case '~':
		    if (argc && !defs[0])
			len = nargs[0];
		    else
			len = 1;
		    if (padidx >= 0)
			LispDestroy(mac, BadArgument, fname);
		    while (len) {
			mac->column += LispPrintf(mac, stream, "~");
			--len;
		    }
		    ++fmt;
		    /* no arguments used */
		    continue;
		case '|':
		    if (argc && !defs[0])
			len = nargs[0];
		    else
			len = 1;
		    if (padidx >= 0)
			LispDestroy(mac, BadArgument, fname);
		    if (len > 0) {
			mac->newline = 1;
			mac->column = 0;
		    }
		    while (len) {
			LispPrintf(mac, stream, "\f");
			--len;
		    }
		    ++fmt;
		    /* no arguments used */
		    continue;
		case '\n':
		    ++fmt;
		    while (*fmt && isspace(*fmt)) {
			if (collon)	/* if collon, don't ignore following
					 * whitespaces, just the leading newline
					 */
			    mac->column += LispPrintf(mac, stream, "%c", *fmt);
			++fmt;
		    }
		    /* no arguments used */
		    continue;
		case 't':	/* Tabulate */
		case 'T':	/* ~colnum,colincT */
		    if (argc && !defs[0])
			colnum = nargs[0];
		    if (argc > 1 && !defs[1])
			colinc = nargs[1];
		    if (!atsign) {
			if (mac->column < colnum) {
			    /* output spaces until mac->column == colnum */
			    while (mac->column < colnum)
				mac->column +=
				    LispPrintf(mac, stream, "%c", ' ');
			}
			else {
			    /* output colinc spaces */
			    while (colinc > 0) {
				mac->column +=
				    LispPrintf(mac, stream, "%c", ' ');
				--colinc;
			    }
			}
		    }
		    else {
			/* relative tabulation, use colnum as colrel */
			while (colnum > 0) {
			    /* outputs colrel spaces */
			    mac->column += LispPrintf(mac, stream, "%c", ' ');
			    --colnum;
			}
			if (colinc > 0) {
			    /* outputs spaces until mac->column is
			     * multiple of colinc */
			    while (mac->column % colinc)
				mac->column +=
				    LispPrintf(mac, stream, "%c", ' ');
			}
		    }
		    ++fmt;
		    /* no arguments used */
		    continue;
		case '*':	/* ignore/jump to argument */
		    len = 1;
		    if (argc && !defs[0])
			len = nargs[0];

		    if (len < 0 || argc > 1)
			LispDestroy(mac, BadArgument, fname);

		    if (!collon && !atsign) {
			/* forward */
			if (arg->type != LispCons_t)
			    goto not_enough_args;
			while (len) {
			    plural = arg;
			    if ((arg = CDR(arg)) == NIL)
				break;
			    ++cur_arg;
			    --len;
			}
		    }
		    else {
			/* count how many arguments processed */
			for (tmp = 0, plural = arguments;  plural != arg;
			     tmp++, plural = CDR(plural))
			    ;
			tmp -= len;
			plural = NIL;
			arg = arguments;
			cur_arg = 0;

			if (!atsign)	/* backwards, else goto argument */
			    len = tmp;
			if (len > 0)
			    for (tmp = 0; tmp < len;
				 tmp++, plural = arg, arg = CDR(arg))
				    cur_arg++;
		    }
		    len = 0;
		    ++fmt;
		    /* no arguments used */
		    continue;
		case '?':	/* Indirection */
		    if (atsign) {
			/* cannot be implemented calling Lisp_Format
			 * recursively, as we don't know how many arguments
			 * will be (un)consumed */

			if (arg->type != LispCons_t)
			    goto not_enough_args;
			if (!STRING_P(CAR(arg)))
			    LispDestroy(mac, "expecting string, at %s", fname);
			indirection = LispRealloc(mac, indirection,
						  sizeof(char*) *
						  (nindirection + 1));
			indirection[nindirection++] = ++fmt;
			fmt = STRPTR(CAR(arg));
			plural = arg;
			arg = CDR(arg);
			++cur_arg;
			/* don't increment fmt */
			continue;
		    }
		    else {
			LispObj *fmt, *lst;

			if (arg->type != LispCons_t)
			    goto not_enough_args;
			fmt = CAR(arg);
			arg = CDR(arg);
			++cur_arg;
			if (arg->type != LispCons_t)
			    goto not_enough_args;
			lst = CAR(arg);
			GCProtect();
			Lisp_Format(mac, CONS(stream, CONS(fmt, lst)), fname);
			GCUProtect();
		    }
		    break;

		/* Here start the "complex" format directives */
		case '(':	/* Case-conversion start */
		    if (argc)
			LispDestroy(mac, BadArgument, fname);
		    /* format the data in a temporary stream */
		    GCProtect();
		    obj = LispNew(mac, NIL, NIL);
		    obj->type = LispStream_t;
		    obj->data.stream.source.str = NULL;
		    obj->data.stream.size = 0;
		    obj->data.stream.idx = 0;
		    stk[len++] = '(';
		    if (atsign)
			stk[len++] = '@';
		    if (collon)
			stk[len++] = ':';
		    stk[len] = '\0';
		    ilist = CONS(CONS(STRING(stk), obj), ilist);
		    alist = CONS(NIL, alist);
		    GCUProtect();
		    stream = obj;
		    len = 0;
		    ++fmt;
		    /* no arguments used */
		    continue;
		case ')':	/* Case-conversion end */
		    if (argc)
			LispDestroy(mac, BadArgument, fname);
		    if (CAR(CAR(ilist)) == NIL ||
			STRPTR(CAR(CAR(ilist)))[0] != '(')
			LispDestroy(mac, "mismatched ~), at %s", fname);
		    /* remember if atsign and/or collon was set */
		    strcpy(stk, STRPTR(CAR(CAR(ilist))));
		    len = 1;
		    if (stk[len] == '@') {
			atsign = 1;
			++len;
		    }
		    else
			atsign = 0;
		    if (stk[len] == ':')
			collon = 1;
		    else
			collon = 0;

		    if ((uptr = stream->data.stream.source.str) != NULL) {
			if (atsign && collon) {     /* uppercase everything */
			    while (*uptr) {
				*uptr = toupper(*uptr);
				++uptr;
			    }
			}
			else if (atsign) {	    /* capitalizes the first word */
			    while (*uptr && !isalpha(*uptr))
				++uptr;
			    if (*uptr) {
				*uptr = toupper(*uptr);
				++uptr;
			    }
			    while (*uptr) {
				*uptr = tolower(*uptr);
				++uptr;
			    }
			}
			else if (collon) {	    /* capitalizes all words */
			    while (*uptr) {
				while (*uptr && !isalpha(*uptr))
				    ++uptr;
				if (*uptr) {
				    *uptr = toupper(*uptr);
				    ++uptr;
				}
				while (*uptr && isalpha(*uptr)) {
				    *uptr = tolower(*uptr);
				    ++uptr;
				}
			    }
			}
			else {			    /* lowercase everything */
			    while (*uptr) {
				*uptr = tolower(*uptr);
				++uptr;
			    }
			}
			uptr = stream->data.stream.source.str;
		    }
		    ilist = CDR(ilist);
		    alist = CDR(alist);
		    stream = CDR(CAR(ilist));
		    if (uptr)
			mac->column += LispPrintf(mac, stream, "%s", uptr);
		    len = 0;
		    ++fmt;
		    /* no arguments used */
		    continue;
		case '[': {	/* Conditional-expression start */
		    char *end, **fields;
		    int field, nfields, obrack, oless, def;
		    int done = 0, scollon = 0;

		    if (argc > 1)
			LispDestroy(mac, BadArgument, fname);

		    def = field = -1;
		    obrack = 1;
		    fields = NULL;
		    nfields = oless = 0;
		    /* split fields as strings, to allow easier processing */
		    ptr = str = fmt + 1;
		    while (!done) {
			switch (*ptr) {
			    case '~':
				scollon = 0;
				end = ptr;	/* "maybe" end */
				++ptr;
				while (*ptr) {
				    if (*ptr == ',' || *ptr == '@' ||
					*ptr == '#' || *ptr == '+' ||
					*ptr == '-' || isdigit(*ptr))
					++ptr;
				    else if (*ptr == '\'') {
					++ptr;
					if (!*ptr)
					    goto error_parsing;
					++ptr;
				    }
				    else if (*ptr == ':') {
					scollon = 1;
					++ptr;
				    }
				    else	/* a directive was found */
					break;
				}
				if (*ptr == '<')
				    ++oless;
				else if (*ptr == '>')
				    --oless;	/* don't check if smaller than
						 * zero. this may be a condition
						 * that will never be evaluated
						 */
				else if (*ptr == '[')
				    ++obrack;
				else if (*ptr == ']' || *ptr == ';') {
				    if (obrack == 1 && *ptr == ']')
					done = 1;
				    if (oless <= 0 && obrack == 1) {
					fields = LispRealloc(mac, fields,
							     sizeof(char*) *
							     (nfields + 1));
					len = end - str;
					fields[nfields] = LispMalloc(mac, len + 1);
					strncpy(fields[nfields], str, len);
					fields[nfields][len] = '\0';
					if (scollon)
					    def = nfields + 1;
					++nfields;
					str = ptr + 1;
				    }
				    if (*ptr == ']')
					--obrack;
				}
				++ptr;
				break;
			    case '\0':
				goto error_parsing;
			    default:
				++ptr;
				break;
			}
		    }
		    fmt = ptr;

		    if (collon) {
			if (arg->type != LispCons_t)
			    goto not_enough_args;
			if (CAR(arg) == NIL) {
			    if (nfields && fields[0][0])
				field = 0;
			}
			else if (nfields > 1 && fields[1][0])
			    field = 1;
			/* don't update plural, just consume argument */
			arg = CDR(arg);
			++cur_arg;
		    }
		    else if (atsign) {
			if (arg->type != LispCons_t)
			    goto not_enough_args;
			if (CAR(arg) == NIL) {
			    plural = arg;
			    arg = CDR(arg);
			    ++cur_arg;
			}
			else if (nfields && fields[0][0])
			    field = 0;
		    }
		    else if (argc && !defs[0]) {
			if (nargs[0] < nfields)
			    field = nargs[0];
			else if (def >= 0)
			    field = def;
		    }
		    else {
			if (arg->type != LispCons_t)
			    goto not_enough_args;
			if (!INTEGER_P(CAR(arg)))
			    LispDestroy(mac, "%s is not an index, at %s",
					LispStrObj(mac, CAR(arg)), fname);
			if (NUMBER_VALUE(CAR(arg)) < nfields &&
			    NUMBER_VALUE(CAR(arg)) >= 0)
			    field = NUMBER_VALUE(CAR(arg));
			else if (def >= 0)
			    field = def;
			plural = arg;
			arg = CDR(arg);
			++cur_arg;
		    }
		    if (field >= 0 && field < nfields) {
			indirection = LispRealloc(mac, indirection,
						  sizeof(char*) *
						  (nindirection + 1));
			indirection[nindirection++] = fmt;
			GCProtect();
			fmt = STRPTR(STRING(fields[field]));
			GCUProtect();
		    }
		    while (--nfields >= 0)
			LispFree(mac, fields[nfields]);
		    LispFree(mac, fields);
		    len = 0;
		    /* if any argument used, already updated arg */
		}   continue;
		case ']':	/* Conditional-expression end */
		    LispDestroy(mac, "mismatched ~], at %s", fname);
		case '{': {	/* Iteration start */
		    int op = 1, scollon = 0, done = 0;
		    char *end;

		    if (argc > 1)
			LispDestroy(mac, BadArgument, fname);

		    /* how many iterations, 0 means until all arguments are
		     * consumed */
		    iteration = 0;
		    if (argc && !defs[0])
			iteration = nargs[0];

		    ptr = str = ++fmt;
		    while (!done) {
			switch (*ptr) {
			    case '~':
				scollon = 0;
				end = ptr;	/* "maybe" end */
				++ptr;
				while (*ptr) {
				    if (*ptr == ',' || *ptr == '@' ||
					*ptr == '#' || *ptr == '+' ||
					*ptr == '-' || isdigit(*ptr))
					++ptr;
				    else if (*ptr == '\'') {
					++ptr;
					if (!*ptr)
					    goto error_parsing;
					++ptr;
				    }
				    else if (*ptr == ':') {
					scollon = 1;
					++ptr;
				    }
				    else	/* a directive was found */
					break;
				}
				if (*ptr == '{')
				    ++op;
				else if (*ptr == '}') {
				    if (--op == 0) {
					char *st = str;

					len = end - str;
					str = LispMalloc(mac, len + 1);
					strncpy(str, st, len);
					str[len] = '\0';
					done = 1;
					++ptr;
					continue;	/* i.e. break the loop */
				    }
				}
				++ptr;
				break;
			    case '\0':
				goto error_parsing;
			    default:
				++ptr;
				break;
			}
		    }
		    if (iteration == -1 && (!argc || !defs[0]))
			iteration = scollon; /* iterate at least once, if scollon */
		    else if (iteration <= 0 && scollon)
			iteration = 1;
		    if (iteration < 0 ||
			(iteration == 0 && argc && !defs[0])) {
			/* if no iterations were requested */
			if (!atsign) {
			    /* consume one element even if no iterations */
			    if (arg->type != LispCons_t)
				goto not_enough_args;
			    plural = arg;
			    arg = CDR(arg);
			    ++cur_arg;
			    fmt = ptr;
			    iteration = -1;
			    LispFree(mac, str);
			    /* don't need to do anything more... */
			    continue;
			}
			else if (arg->type != LispCons_t) {
			    /* no arguments left, don't even start iteration */
			    fmt = ptr;
			    iteration = -1;
			    LispFree(mac, str);
			    continue;
			}
		    }

		    if (!atsign) {	/* need to check for errors later */
			if (arg->type != LispCons_t)
			    LispDestroy(mac, BadArgument, fname);
			if (collon && CAR(arg)->type != LispCons_t)
			    LispDestroy(mac, BadArgument, fname);
		    }

		    end = ptr;
		    /* if got here, than no errors detected so far */
		    if (!*str) {
			/* if string is empty, use next argument as control
			 * string */
			LispFree(mac, str);
			if (arg->type != LispCons_t)
			    goto not_enough_args;
			else if (!STRING_P(CAR(arg)))
			    LispDestroy(mac, "expecting string, at %s", fname);
			ptr = STRPTR(CAR(arg));
			plural = arg;
			arg = CDR(arg);
			++cur_arg;
		    }
		    else {
			GCProtect();
			ptr = STRPTR(STRING(str));
			LispFree(mac, str);
			GCUProtect();
		    }

		    len = 0;
		    stk[len++] = '{';
		    if (atsign)
			stk[len++] = '@';
		    if (collon)
			stk[len++] = ':';
		    stk[len] = '\0';
		    GCProtect();
		    ilist = CONS(CONS(STRING(stk), stream), ilist);
		    alist = CONS(REAL((double)nindirection), alist);
		    if (!atsign) {
			CAR(alist) = collon ?
			    CONS(CAR(arg), CDR(arg)) : CDR(arg);
			plural = NIL;
			arg = collon ? CAR(CAR(arg)) : CAR(arg);
			sargs = CONS(CONS(arguments,
					  CONS(REAL((double)num_args),
					       REAL((double)cur_arg))), sargs);
			arguments = obj = arg;
			for (num_args = cur_arg = 0; obj->type == LispCons_t;
			     obj = CDR(obj))
			    ++num_args;
		    }
		    else if (collon && arg->type == LispCons_t) {
			CAR(alist) = arg;
			plural = NIL;
			arg = CAR(arg);
			sargs = CONS(CONS(arguments,
					  CONS(REAL((double)num_args),
					       REAL((double)cur_arg))), sargs);
			arguments = obj = arg;
			for (num_args = cur_arg = 0; obj->type == LispCons_t;
			     obj = CDR(obj))
			    ++num_args;
		    }
		    GCUProtect();
		    indirection = LispRealloc(mac, indirection,
					      sizeof(char*) *
					      (nindirection + 2));
		    indirection[nindirection++] = end;
		    indirection[nindirection++] = ptr;
		    len = 0;
		    fmt = ptr;
		}   continue;
		case '}':	/* Iteration end */
		    LispDestroy(mac, "mismatched ~}, at %s", fname);
		case '<':	/* Justification start */
		    len = 0;
		    stk[len++] = '<';
		    if (atsign)
			stk[len++] = '@';
		    if (collon)
			stk[len++] = ':';
		    stk[len] = '\0';
		    if (argc && !defs[0])
			mincol = nargs[0];
		    if (argc > 1 && !defs[1])
			colinc = nargs[1];
		    if (argc > 2 && !defs[2])
			minpad = nargs[2];
		    if (argc > 3 && !defs[3])
			padchar = nargs[3];
		    if (argc > 4 || (padidx != -1 && padidx != 3))
			LispDestroy(mac, BadArgument, fname);
		    GCProtect();
		    obj = LispNew(mac, NIL, NIL);
		    obj->type = LispStream_t;
		    obj->data.stream.source.str = NULL;
		    obj->data.stream.size = 0;
		    obj->data.stream.idx = 0;
		    stream = obj;
		    ilist = CONS(CONS(STRING(stk), obj), ilist);
		    alist = CONS(CONS(NIL, CONS(obj, NIL)), alist);
		    obj = CONS(REAL((double)mincol),
			       CONS(REAL((double)colinc),
				    CONS(REAL((double)minpad),
					 CONS(REAL((double)padchar),
					      CONS(NIL,
						   CONS(NIL,
							REAL((double)mac->column)))))));
					      /* chars after and line width,
					       * i.e. defaults for first
					       * parameter, if any.
					       */
		    CAR(CAR(alist)) = obj;
		    GCUProtect();
		    len = 0;
		    ++fmt;
		    /* no arguments used */
		    continue;
		case ';':	/* separator for ~[ and ~<, in this code,
				 * only used for ~< */
		    if (!STRING_P(CAR(CAR(ilist))) ||
			STRPTR(CAR(CAR(ilist)))[0] != '<')
			LispDestroy(mac, "~; not allowed here, at %s", fname);

		    GCProtect();
		    if (collon && CDR(CDR(CAR(alist))) == NIL) {
			double dtmp;

			if (argc && !defs[0]) {		/* chars after */
			    dtmp = nargs[0];
			    CAR(CDR(CDR(CDR(CDR(CAR(CAR(alist))))))) =
				REAL(dtmp);
			}
			if (argc > 1 && !defs[1]) {	/* line width */
			    dtmp = nargs[1];
			    CAR(CDR(CDR(CDR(CDR(CDR(CAR(CAR(alist)))))))) =
				REAL(dtmp);
			}
		    }
		    obj = LispNew(mac, NIL, NIL);
		    obj->type = LispStream_t;
		    obj->data.stream.source.str = NULL;
		    obj->data.stream.size = 0;
		    obj->data.stream.idx = 0;
		    CDR(CAR(alist)) = CONS(obj, CDR(CAR(alist)));
		    stream = CDR(CAR(ilist)) = obj;
		    GCUProtect();
		    ++fmt;
		    /* no arguments used */
		    continue;
		case '>': {	/* Justification end */
		    double left, inc;
		    int bytes, count, pos;
		    LispObj *charsafter, *linewidth, *pad, *otmp;

    		    if (argc)
			LispDestroy(mac, BadArgument, fname);
		    if (CAR(CAR(ilist)) == NIL ||
			STRPTR(CAR(CAR(ilist)))[0] != '<')
			LispDestroy(mac, "mismatched ~>, at %s", fname);

		    /* remember if atsign and/or collon was set */
		    strcpy(stk, STRPTR(CAR(CAR(ilist))));
		    len = 1;
		    if (stk[len] == '@') {
			atsign = 1;
			++len;
		    }
		    else
			atsign = 0;
		    if (stk[len] == ':')
			collon = 1;
		    else
			collon = 0;

		    /* restore parameters */
		    obj = CAR(CAR(alist));
		    mincol = (int)CAR(obj)->data.real;
		    colinc = (int)CAR(CDR(obj))->data.real;
		    minpad = (int)CAR(CDR(CDR(obj)))->data.real;
		    padchar = (int)CAR(CDR(CDR(CDR(obj))))->data.real;
		    charsafter = CAR(CDR(CDR(CDR(CDR(obj)))));
		    linewidth = CAR(CDR(CDR(CDR(CDR(CDR(obj))))));
		    mac->column = (int)CDR(CDR(CDR(CDR(CDR(CDR(obj))))))->
				       data.real;

		    /* if use default for either charsafter or linewidth */
		    GCProtect();
		    if (charsafter != NIL && linewidth == NIL)
			linewidth = REAL(72.0);
		    else if (charsafter == NIL && linewidth != NIL)
			charsafter = REAL(0.0);
		    GCUProtect();

		    /* count number of fields and set pad variable */
		    for (argc = bytes = 0, pad = obj = CDR(CAR(alist));
			 obj != NIL; argc++, pad = obj, obj = CDR(obj)) {
			if (CAR(obj)->data.stream.source.str)
			    bytes += strlen((char*)
					    CAR(obj)->data.stream.source.str);
		    }
		    if (charsafter == NIL)
			pad = NIL;
		    else if (pad != NIL)
			--argc;

		    obj = LispReverse(CDR(CAR(alist)));
		    if (pad != NIL) {
			obj = CDR(obj);
			pad = CAR(pad);
			if (pad->data.stream.source.str)
			    str = (char*)pad->data.stream.source.str;
			else
			    str = "";
			bytes -= strlen(str);
		    }

		    /* first, adds minpad, if any to strings */
		    if (minpad) {
			for (otmp = pad || collon ? obj : CDR(obj), tmp = 1;
			     otmp != NIL;
			     otmp = CDR(otmp), tmp++) {
			    len = CAR(obj)->data.stream.source.str ?
				  strlen((char*)
					 CAR(obj)->data.stream.source.str) : 0;
			    len += minpad + 1;
			    if (len > CAR(obj)->data.stream.size) {
				ptr = realloc(CAR(obj)->data.stream.source.str,
					      len);
				if (ptr == NULL)
				    LispDestroy(mac, "out of memory");
				CAR(obj)->data.stream.source.str =
				    (unsigned char*)ptr;
				CAR(obj)->data.stream.size = len;
			    }
			    else
				ptr = (char*)CAR(obj)->data.stream.source.str;
			    CAR(obj)->data.stream.idx = len;
			    if (len > minpad + 1 && (tmp < argc || !atsign))
				memmove(ptr + minpad, ptr, len - minpad);
			    if (tmp == argc && atsign) {	/* right pad */
				int tlen = len;

				for (len -= minpad + 1; len < tlen; len++)
				    ptr[len] = padchar;
			    }
			    else {
				len = minpad;
				while (--len >= 0)
				    ptr[len] = padchar;
			    }
			    bytes += minpad;
			}
		    }

		    /* adjust mincol */
		    if (colinc > 0) {
			while (bytes > mincol)
			    mincol += colinc;
		    }
		    else if (mincol < bytes)
			mincol = bytes;

		    left = mincol - bytes;
		    if (left < 0)
			left = 0;
		    if (argc > 1) {
			if (collon ^ atsign)
			    inc = left / (double)argc;
			else if (collon)
			    inc = left / (double)(argc + 1);
			else
			    inc = left / (double)(argc - 1);
		    }
		    else
			inc = left;

		    ilist = CDR(ilist);
		    alist = CDR(alist);
		    stream = CDR(CAR(ilist));
		    /* format strings in the output stream */
		    for (tmp = pos = 0; obj != NIL; tmp++, obj = CDR(obj)) {
			if (CAR(obj)->data.stream.source.str)
			    ptr = (char*)CAR(obj)->data.stream.source.str;
			else
			    ptr = "";
			count = len = strlen(ptr);
			if (pad != NIL &&
			    mac->column + len + charsafter->data.real >
			    linewidth->data.real) {
			    char *nl;

			    mac->column += LispPrintf(mac, stream, "%s", str);
			    if ((nl = strrchr(str, '\n')) != NULL) {
				mac->column = 0;
				++nl;
				while (*nl++)
				    ++mac->column;
			    }
			}

			if (tmp + 1 == argc) {
			    if (!atsign || collon) {
				if (atsign) {
				    count += (int)(left / 2.0);
				    left -= (int)(left / 2.0);
				}
				else {
				    count += (int)left;
				    left -= (int)left;
				}
			    }
			    else if (!atsign) {
				count += (int)left;
				left -= (int)left;
			    }
			}
			else if (tmp != 0 || collon) {
			    double dleft = left;

			    left -= inc;
			    count += dleft - left;
			    if ((int)(left + 0.5) != (int)left)
				++count;
			}

			while (len < count) {
			    mac->column += LispPrintf(mac, stream,
						      "%c", padchar);
			    ++len;
			}
			mac->column += LispPrintf(mac, stream, "%s", ptr);
		    }

		    while (left > 0.0) {
			mac->column += LispPrintf(mac, stream, "%c", padchar);
			left -= 1.0;
		    }

		    len = 0;
		    ++fmt;
		    /* no arguments used */
		}   continue;
		case '^':	/* Up and out */
		    if (argc > 3 || padidx != -1)
			LispDestroy(mac, BadArgument, fname);
		    tmp = -1;
		    if (argc && !defs[0])	/* terminate if tmp == 0 */
			tmp = nargs[0];
		    if (argc > 1 && !defs[1])	/* terminate if tmp == len */
			len = nargs[1];
		    if (argc > 2 && !defs[2]) {	/* if tmp <= len <= nargs[2] */
			if (tmp <= len && len <= nargs[2])
			    tmp = 0;
			else
			    tmp = 1;
		    }
		    if (argc == 2)
			tmp = (len == tmp) ? 0 : 1;
		    len = 0;

		    if (CAR(CAR(ilist)) != NIL &&
			STRPTR(CAR(CAR(ilist)))[0] == '{' && collon) {
			/* needs special handling */
			LispObj *next = T;

			if (STRPTR(CAR(CAR(ilist)))[1] == '@' &&
			    STRPTR(CAR(CAR(ilist)))[2] == ':')
			    next = CDR(CAR(alist));
			else if (STRPTR(CAR(CAR(ilist)))[1] == ':')
			    next = CDR(CAR(CAR(alist)));

			if (hash != -1 || next == NIL) {
			    if (next == NIL ||
				(hash != -1 && arg->type != LispCons_t)) {
				while (*fmt)
				    ++fmt;
			    }
			    else
				++fmt;
			    if ((hash != -1 && arg->type != LispCons_t))
				iteration = 1;	/* force finalization */
			    continue;
			}
		    }

		    if (tmp == 0 || (tmp == -1 && arg->type != LispCons_t)) {
			if (CAR(CAR(ilist)) != NIL) {
			    if (STRPTR(CAR(CAR(ilist)))[0] == '{') {
				if (collon) {
				    /* passed the test above */
				    ++fmt;
				    continue;
				}
				while (*fmt)
				    ++fmt;
			    }
			    else if (STRPTR(CAR(CAR(ilist)))[0] == '(')
				fmt = "~)";	/* make the loop find
						 * the end... */
			    else if (STRPTR(CAR(CAR(ilist)))[0] == '<') {
				/* need to remove the last stream, to
				 * format correctly */
				if (CDR(CAR(alist)) != NIL)
				    /* don't need to update stream */
				    CDR(CAR(alist)) = CDR(CDR(CAR(alist)));
				fmt = "~>";
			    }
			}
			else
			    goto format_done;
		    }
		    else
			++fmt;
		    /* no arguments used */
		    continue;
		default:
		    LispDestroy(mac, "unknown directive ~%c, at %s",
				*fmt, fname);
	    }
	    ++fmt;
	    plural = arg;
	    arg = CDR(arg);
	    ++cur_arg;
	    continue;

print_number:
	    mac->newline = 0;
	    if (arg->type != LispCons_t)
		goto not_enough_args;
	    /* if not an integer */
	    if (!INTEGER_P(CAR(arg))) {
		/* print just as 'A' */
		isprinc = 1;
		goto print_object;
	    }
	    else {
		int sign;
		long num = (long)NUMBER_VALUE(CAR(arg));

		len = 0;
		if ((sign = num < 0) != 0)
		    num = -num;

		/* check for radix */
		if (isradix) {
		    if (argc == 0 || defs[0]) {
			radix = 0;
			++len;
			goto print_number_args;
		    }
		    radix = nargs[0];
		    ++len;
		}
		if (radix < 2 || radix > 32)
		    LispDestroy(mac, "radix must be in the range 2 to 32,"
				" at %s", fname);

print_number_args:
		/* get print arguments */
		if (len < argc && !defs[len])
		    mincol = nargs[len];
		++len;
		if (len < argc && !defs[len])
		    padchar = nargs[len];

		if (padidx >= 0 && padidx != len)
		    LispDestroy(mac, BadArgument, fname);

		if (radix) {
		    len = 0;
		    do {
			int val;

			val = num % radix;
			num -= val;
			num /= radix;
			if (len)
			    memmove(stk + 1, stk, len);
			*stk = val < 10 ? val + '0' : (val - 10) + 'A';
			++len;
		    } while (num);
		    if (sign || atsign) {
			memmove(stk + 1, stk, len);
			*stk = sign ? '-' : '+';
			++len;
		    }
		}
		else if (atsign) {	/* roman */
		    long num = (long)NUMBER_VALUE(CAR(arg));

		if ((double)num != NUMBER_VALUE(CAR(arg)) ||
		    num <= 0 || num > (3999 + (collon ? 1000 : 0)))
		    LispDestroy(mac, BadArgument, fname);

		    /* if collon, print in old roman format */
		    len = 0;
		    while (num > 1000) {
			stk[len++] = 'M';
			num -= 1000;
		    }
		    if (!collon) {
			if (num >= 900) {
			    strcpy(stk + len, "CM");
			    len += 2,
			    num -= 900;
			}
		        else if (num < 500 && num >= 400) {
			    strcpy(stk + len, "CD");
			    len += 2;
			    num -= 400;
			}
		    }
		    if (num >= 500) {
			stk[len++] = 'D';
			num -= 500;
		    }
		    while (num >= 100) {
			stk[len++] = 'C';
			num -= 100;
		    }

		    if (!collon) {
			if (num >= 90) {
			    strcpy(stk + len, "XC");
			    len += 2,
			    num -= 90;
			}
			else if (num < 50 && num >= 40) {
			    strcpy(stk + len, "XL");
			    len += 2;
			    num -= 40;
			}
		    }
		    if (num >= 50) {
			stk[len++] = 'L';
			num -= 50;
		    }
		    while (num >= 10) {
			stk[len++] = 'X';
			num -= 10;
		    }

		    if (!collon) {
			if (num == 9) {
			    strcpy(stk + len, "IX");
			    len += 2,
			    num -= 9;
	 		}
			else if (num == 4) {
			    strcpy(stk + len, "IV");
			    len += 2;
			    num -= 4;
			}
		    }
		    if (num >= 5) {
			stk[len++] = 'V';
			num -= 5;
		    }
		    while (num) {
			stk[len++] = 'I';
			num -= 1;
		    }
		}
		else {			/* english */
		    len = 0;
#define SIGNLEN		6		/* strlen("minus ") */
		    if (sign) {
			strcpy(stk, "minus ");
			len += SIGNLEN;
		    }
		    else if (num == 0) {
			if (collon) {
			    strcpy(stk, "zeroth");
			    len += 6;  /*123456*/
			}
			else {
			    strcpy(stk, "zero");
			    len += 4;  /*1234*/
			}
		    }
		    while (1) {
			char *d, *h, *t;
			int l, count = 0;
			long val = num;
			static char *ds[] = {
			    "",         "one",      "two",        "three",
			    "four",     "five",     "six",        "seven",
			    "eight",    "nine",      "ten",       "eleven",
			    "twelve",   "thirteen",  "fourteen",  "fifteen",
			    "sixteen",  "seventeen",  "eighteen", "nineteen"
			};
			static char *dsth[] = {
			    "",           "first",      "second",
			    "third",      "fourth",     "fifth",
			    "sixth",      "seventh",    "eighth",
			    "ninth",      "tenth",      "eleventh",
			    "twelfth",    "thirteenth", "fourteenth",
			    "fifteenth",  "sixteenth",  "seventeenth",
			    "eighteenth", "nineteenth"
			};
			static char *hs[] = {
			    "",      "",      "twenty",  "thirty", "forty",
			    "fifty", "sixty", "seventy", "eighty", "ninety"
			};
			static char *hsth[] = {
			    "",          "",         "twentieth",  "thirtieth",
			    "fortieth",  "fiftieth", "sixtieth",   "seventieth",
			    "eightieth", "ninetieth"
			};
			static char *ts[] = {
			    "", "thousand", "million", "billion", "trillion"
			};
			static char *tsth[] = {
			    "",          "thousandth", "millionth", "billionth",
			    "trillionth"
			};

			while (val >= 1000) {
			    val /= 1000;
			    ++count;
			}
			if (count > sizeof(ts) / sizeof(ts[0]))
			    LispDestroy(mac, "format is too large, at %s",
					fname);

			t = ds[val / 100];
			if (collon && !count && (val % 10) == 0)
			    h = hsth[(val % 100) / 10];
			else
			    h = hs[(val % 100) / 10];

			if (collon && !count)
			    d = *h ? dsth[val % 10] : dsth[val % 20];
			else
			    d = *h ? ds[val % 10] : ds[val % 20];

			if (((!sign && len) || len > SIGNLEN) &&
			    (*t || *h || *d)) {
			    if (!collon || count || *h || *t) {
				strcpy(stk + len, ", ");
				len += 2;
			    }
			    else {
				strcpy(stk + len, " ");
				++len;
			    }
			}
			if (*t) {
			    if (collon && !count && (val % 100) == 0)
				l = sprintf(stk + len, "%s hundredth", t);
			    else
				l = sprintf(stk + len, "%s hundred", t);
			    len += l;
			}
			if (*h) {
			    if (*t) {
				if (collon && !count) {
				    strcpy(stk + len, " ");
				    ++len;
				}
				else {
				    strcpy(stk + len, " and ");
				    len += 5;        /*12345*/
				}
			    }
			    l = sprintf(stk + len, "%s", h);
			    len += l;
			}
			if (*d) {
			    if (*h) {
				strcpy(stk + len, "-");
				++len;
			    }
			    else if (*t) {
				if (collon && !count) {
				    strcpy(stk + len, " ");
				    ++len;
				}
				else {
				    strcpy(stk + len, " and ");
				    len += 5;        /*12345*/
				}
			    }
			    l = sprintf(stk + len, "%s", d);
			    len += l;
			}
			if (!count)
			    break;
			else
			    tmp = count;
			if (count > 1) {
			    val *= 1000;
			    while (--count)
				val *= 1000;
			    num -= val;
			}
			else
			    num %= 1000;

			if (collon && num == 0 && !*t && !*h)
			    l = sprintf(stk + len, " %s", tsth[tmp]);
			else
			    l = sprintf(stk + len, " %s", ts[tmp]);
			len += l;

			if (num == 0)
			    break;
		    }
		}

		stk[len] = '\0';
		while (mincol > len) {
		    mac->column += LispPrintf(mac, stream, "%c", padchar);
		    --mincol;
		}
		mac->column += LispPrintf(mac, stream, "%s", stk);
		len = 0;
	    }
	    ++fmt;
	    plural = arg;
	    arg = CDR(arg);
	    ++cur_arg;
	    continue;

print_float_number:
	    mac->newline = 0;
	    if (arg->type != LispCons_t)
		goto not_enough_args;
	    if (CAR(arg)->type != LispReal_t) {
		/* print just as 'A' */
		isprinc = 1;
		goto print_object;
	    }
	    else {
		double num = CAR(arg)->data.real;
		char sprint[1024];
		int l, sign, ee = 0, expt = 0, elen = 1, kset = 0, dset = 0,
		    eset = 0;

		/* get print arguments */
		l = 0;
		if (*fmt == '$') {
		    if (argc && !defs[l]) {
			d = nargs[l];
			dset = 1;
		    }
		    else
			d = 2;		/* defaults to 2 for '$' */
		    ++l;
		    if (argc > l && !defs[l])
			n = nargs[l];
		    ++l;
		    if (argc > l && !defs[l])
			w = nargs[l];
		    ++l;
		    if (argc > l && !defs[l])
			padchar = nargs[l];
		    if (argc > 4 || d < 0 || n < 0 ||
			(argc > 2 && !defs[2] && w < 2) ||
			(padidx != -1 && (padidx != 3)))
			LispDestroy(mac, BadArgument, fname);
		    kset = 1;
		}
		else {
		    if (argc && !defs[l])
			w = nargs[l];
		    ++l;
		    if (argc > l && !defs[l]) {
			d = nargs[l];
			dset = 1;
		    }
		    ++l;
		    if (ise) {
			if (argc > l && !defs[l]) {
			    e = nargs[l];
			    eset = 1;
			}
			++l;
		    }
		    if (argc > l && !defs[l]) {
			k = nargs[l];
			kset = 1;
		    }
		    ++l;
		    if (argc > l && !defs[l])
			overflowchar = nargs[l];
		    ++l;
		    if (argc > l && !defs[l])
			padchar = nargs[l];
		    ++l;
		    if (argc > l && !defs[l])
			exponentchar = nargs[l];

		    if (overidx == -1 && padidx != -1) {
			overidx = padidx;
			padidx = -1;
		    }
		    if ((argc > 2 && !defs[2] && w < 2) ||
			(argc > 1 && !defs[1] && d < 0) ||
			(overidx != -1 && (overidx != 3 + ise)) ||
			(padidx != -1 && (padidx != 4 + ise)))
			LispDestroy(mac, BadArgument, fname);
		}

		sign = num < 0.0;
		if (sign)
		    num = -num;

		if (toupper(*fmt) == 'G') {
		    double dtmp = 10.0;
		    int ww, dd, n = 0, q;

		    /* decide if print as ~F or ~E
		     */
		    if (num == 0.0)
			q = 1;
		    else {
			l = sprintf(stk, "%f", num);
			while (l > 1 && stk[l - 1] == '0')
 			    --l;
			q = l + (sign || atsign);
		    }

		    if (num >= 10.0)
			for (n = 1; dtmp <= num; n++, dtmp *= 10.0)
			    ;
		    else
			for (dtmp /= 10.0, n = 1; dtmp >= num; n--, dtmp /= 10.0)
			    ;

		    if (w)
			ww = w - ((e ? e : 2) + 2);
		    else
			ww = 0;
		    if (!dset) {
			d = n > 7 ? 7 : n;
			if (d < q)
			    d = q;
		    }
		    dd = d - n;

		    if (d >= dd && dd >= 0) {
			dset = kset = 1;
			w = ww;
			d = dd;
			k = 0;
			ise = 0;
			/* add that amount of pads after number,
			 * to "emulate" ~ee,@T */
			ee = eset ? e + 2 : 4;
		    }
		}

		if (ise) {
		    if (k > 0 && d) {
			if ((d -= (k - 1)) < 0)
			    LispDestroy(mac, BadArgument, fname);
		    }
		    else if (k < 0 && -k > d)
			LispDestroy(mac, BadArgument, fname);
		}

		len = 0;
		if (*fmt != '$') {
		    if (sign)
			stk[len++] = '-';
		    else if (atsign)
			stk[len++] = '+';
		}

		/* adjust scale factor/exponent */
		l = k;
		while (l > 0) {
		    --l;
		    --expt;
		    num *= 10.0;
		}
		while (l < 0) {
		    ++l;
		    ++expt;
		    num /= 10.0;
		}
		if (ise) {
		    if (!kset)
			k = 1;
		    if (num > 1.0) {
			l = sprintf(sprint, "%1.1f", num);
			while (l > 1 && sprint[--l] != '.')
			    ;
		    }
		    else {
			int pos;
			char sprint2[1024];

			if (dset) {
			    sprintf(sprint2, "%%1.%df", d);
			    l = sprintf(sprint, sprint2, num);
			}
			else
			    l = sprintf(sprint, "%f", num);
			for (pos = 0; sprint[pos] && sprint[pos] != '.'; pos++)
			    ;
			if (sprint[pos]) {
			    for (l = 0, pos++; sprint[pos] == '0'; pos++, l--)
				;
			    if (!sprint[pos])
				l = k;
			}
			else
			    l = k;
		    }
		    while (l > k) {
			--l;
			num /= 10.0;
			++expt;
		    }
		    while (l < k) {
			++l;
			num *= 10.0;
			--expt;
		    }
		}

		if (!dset) {
		    int left = 20;
		    double integral, fractional;

		    fractional = modf(num, &integral);
		    if (w) {
			l = sprintf(sprint, "%f", integral);
			while (l > 1 && sprint[l - 1] == '0')
			    --l;
			if (l && sprint[l - 1] == '.')
			    --l;
			left = w - l - 1 - sign;
		    }
		    l = sprintf(sprint, "%f", fractional);
		    while (l && sprint[l - 1] == '0')
			--l;
		    l -= 2 + (w && sign);
		    if (l > left)
			l = left;
		    sprintf(sprint, "%%1.%df", l > 0 ? l : 0);
		}
		else
		    sprintf(sprint, "%%1.%df", d);
		l = sprintf(stk + len, sprint, num);

		len += l;
		if (ise) {
		    l = sprintf(stk + len, "%c%c", exponentchar,
				expt < 0 ? '-' : '+');
		    len += l;
		    if (e)
			sprintf(sprint, "%%0%dd", e);
		    else
			strcpy(sprint, "%d");
		    l = sprintf(stk + len, sprint, expt < 0 ? -expt : expt);
		    len += l;
		    elen = l + 2;	/* sign and exponentchar */
		}

		/* adjust width */
		l = len;
		if (ise)
		    len -= elen;

		/* find '.' in string */
		for (tmp = 0; tmp < len; tmp++)
		    if (stk[tmp] == '.')
			break;

		if (tmp == len) {
		    /* '.' not found */
		    memmove(stk + len + 1 + d, stk + len, l - tmp);
		    stk[tmp] = '.';
		    len = tmp + 1;
		    for (tmp = 0; tmp < d; len++, tmp++)
			stk[len] = '0';
		    len = l + 1 + d;
		}
		else if (dset) {
		    int tmp2;

		    ++tmp;
		    tmp2 = tmp;
		    /* correct exponent string */
		    if (ise)
			memmove(stk + tmp + d, stk + l - elen, elen);
		    /* pad with '0' chars if required */
		    for (tmp = len; tmp < l - elen; tmp++)
			stk[tmp] = '0';
		    len = tmp2 + d + (ise ? elen : 0);
		}

		/* '$' does not have an overflowchar parameter */
		if (w && *fmt != '$') {
		    if (len > w && num < 1) {
			int inc = sign || atsign;

			/* cut the leading '0' */
			memmove(stk + inc, stk + inc + 1, len - inc - 1);
			--len;
		    }
		    if (((ise && elen - 2 > e) || len > w) && overflowchar) {
			for (len = 0; len < w; len++)
			    stk[len] = overflowchar;
		    }
		}

		l = len;
		stk[len] = '\0';
		if (*fmt == '$') {
		    if (collon) {
			if (sign)
			    mac->column += LispPrintf(mac, stream, "%c", '-');
			else if (atsign)
			    mac->column += LispPrintf(mac, stream, "%c", '+');
		    }
		    /* make sure not too much padchars are printed */
		    if (len < n + d + 1)
			len = n + d + 1 + (sign || atsign);
		}

		while (len < w) {
		    mac->column += LispPrintf(mac, stream, "%c", padchar);
		    ++len;
		}

		if (*fmt == '$') {
		    if (!collon) {
			if (sign)
			    mac->column += LispPrintf(mac, stream, "%c", '-');
			else if (atsign)
			    mac->column += LispPrintf(mac, stream, "%c", '+');
		    }
		    for (tmp = 0; tmp < l; tmp++)
			if (stk[tmp] == '.')
			    break;
		    if (tmp < l)
			--tmp;
		    while (tmp < n) {
			mac->column += LispPrintf(mac, stream, "%c", '0');
			++tmp;
		    }
		}
		mac->column += LispPrintf(mac, stream, "%s", stk);
		while (ee > 0) {
		    /* not padchar, but real spaces.
		     * Note that this is only executed if ~G decided to
		     * print the number in ~F format. This is a ~ee,@T
		     * emulation */
		    mac->column += LispPrintf(mac, stream, "%c", ' ');
		    --ee;
		}
		len = 0;
	    }
	    ++fmt;
	    plural = arg;
	    arg = CDR(arg);
	    ++cur_arg;
	    continue;

print_object:
	    mac->newline = 0;
	    if (arg->type != LispCons_t)
		goto not_enough_args;

	    if (padidx >= 0 && padidx != 3)
		LispDestroy(mac, BadArgument, fname);

	    /* get print arguments */
	    if (argc && !defs[0])
		mincol = nargs[0];
	    if (argc > 1 && !defs[1])
		colinc = nargs[1];
	    if (argc > 2 && !defs[2])
		minpad = nargs[2];
	    if (argc > 3 && !defs[3])
		padchar = nargs[3];

	    if (atsign) {
		int justsize = mac->justsize;

		mac->justsize = 1;
		len = LispPrintObj(mac, stream, CAR(arg), 1);
		mac->justsize = justsize;
		while (len < mincol) {
		    mac->column += LispPrintf(mac, stream, "%c", padchar);
		    ++len;
		}
	    }

	    if (isprinc)
		mac->princ = 1;
	    if (collon && toupper(*fmt) == 'A' && CAR(arg) == NIL)
		len = LispPrintf(mac, stream, "%s", "()");
	    else
		len = LispPrintObj(mac, stream, CAR(arg), 1);
	    mac->column += len;	/* XXX maybe should look if the object has
				 * newlines, and adjust mac->column in
				 * that case */
	    if (!atsign) {
		while (len < mincol) {
		    mac->column += LispPrintf(mac, stream, "%c", padchar);
		    ++len;
		}
	    }
	    if (isprinc)
		mac->princ = princ;
	    len = 0;
	    ++fmt;
	    plural = arg;
	    arg = CDR(arg);
	    ++cur_arg;
	    continue;

not_enough_args:
	    LispDestroy(mac, "no arguments left, at %s", fname);
error_parsing:
	    LispDestroy(mac, "error parsing directive, at %s", fname);
	}
	else {
	    mac->newline = 0;
	    if (len + 1 < sizeof(stk))
		stk[len++] = *fmt;
	    else {
		stk[len] = '\0';
		mac->column += LispPrintf(mac, stream, "%s", stk);
		len = 0;
	    }
	}
	++fmt;
    }
format_done:
    if (len) {
	stk[len] = '\0';
	mac->column += LispPrintf(mac, stream, "%s", stk);
    }

    if (stream != NIL && (stream->data.stream.size >= 0 ||
	stream->data.stream.source.fp != lisp_stdout))
	mac->newline = newline;
    else
	fflush(lisp_stdout);

    LispFree(mac, indirection);

    if (CAR(CAR(ilist)) != NIL) {
	char c;

	switch (STRPTR(CAR(CAR(ilist)))[0]) {
	    case '(':
		c = ')';
		break;
	    case '{':
		c = '}';
		break;
	    case '<':
		c = '>';
		break;
	    default:
		c = '?';
		break;
	}
	LispDestroy(mac, "expecting ~%c, at %s", c, fname);
    }

    if (stream != NIL && stream->data.stream.size >= 0) {
	if (stream->data.stream.source.str == NULL)
	    return (STRING(""));
	return (STRING((char*)stream->data.stream.source.str));
    }

    return (stream);
}

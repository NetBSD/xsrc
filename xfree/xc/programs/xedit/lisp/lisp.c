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

/* $XFree86: xc/programs/xedit/lisp/lisp.c,v 1.27 2002/01/10 04:29:18 dawes Exp $ */

#include <stdlib.h>
#include <string.h>
#ifdef sun	/* Don't ask.... */
#include <strings.h>
#endif
#include <ctype.h>
#include <stdarg.h>
#include <signal.h>

#ifndef X_NOT_POSIX
#include <unistd.h>	/* for sysconf(), and getpagesize() */
#endif

#if defined(linux)
#include <asm/page.h>	/* for PAGE_SIZE */
#define HAS_GETPAGESIZE
#define HAS_SC_PAGESIZE	/* _SC_PAGESIZE may be an enum for Linux */
#endif

#if defined(CSRG_BASED)
#define HAS_GETPAGESIZE
#endif

#if defined(sun)
#define HAS_GETPAGESIZE
#endif

#if defined(QNX4)
#define HAS_GETPAGESIZE
#endif

#if defined(__QNXNTO__)
#define HAS_SC_PAGESIZE
#endif

#include "private.h"

#include "format.h"
#include "require.h"
#include "struct.h"
#include "time.h"

/*
 * Prototypes
 */
/* run a user function, to be called only by LispEval */
LispObj *LispRunFunMac(LispMac*, LispObj*, LispObj*);
/* expands and executes a setf method, to be called only by Lisp_Setf */
LispObj *LispRunSetf(LispMac*, LispObj*, LispObj*, LispObj*);

/* build argument list, assigning defaults and/or positioning &key arguments
 * return value is gc protected, and should be called only by
 * LispEval and LispRunSetf
 */
static LispObj *LispBuildArguments(LispMac*, LispObj*, LispObj*,
				   char*, int);

/* build argument list for builtin functions or functions that don't have
 * any of the &some-name special parameters.
 * return value is gc protected, and should be called only by
 * LispEval and LispRunSetf
 */
static LispObj *LispBuildSimpleArguments(LispMac*, LispObj*,
					 char*, int, int, int);

int LispGet(LispMac*);
int LispUnget(LispMac*);
static int LispSkipComment(LispMac*);
static int LispSkipWhiteSpace(LispMac*);
static char *LispIntToOpaqueType(LispMac*, int);
static LispObj *LispEvalBackquote(LispMac*, LispObj*);

static LispProperty *LispAllocAtomProperty(LispMac*);

	/* if no properties remaining, free atom->property,
	 * and this way, make string candidate to gc */
static void LispCheckAtomProperty(LispMac*, LispAtom*);

static LispObj *LispDoGetAtomProperty(LispMac*, LispAtom*, LispObj*, int);


void LispSnprint(LispMac*, LispObj*, char*, int);
void LispSnprintObj(LispMac*, LispObj*, char**, int*, int);

void LispCheckMemLevel(LispMac*);

void LispAllocSeg(LispMac*);
void LispMark(LispObj*);

#ifdef SIGNALRETURNSINT
int LispAbortSignal(int);
int LispFPESignal(int);
#else
void LispAbortSignal(int);
void LispFPESignal(int);
#endif

static int GetPageSize(void);

/*
 * Initialization
 */
static LispObj lispnil = {LispNil_t};
static LispObj lispt = {LispTrue_t};
static LispObj lispdot = {LispAtom_t};
LispObj *NIL = &lispnil, *T = &lispt;
static LispObj *DOT = &lispdot;
static LispObj **objseg, *freeobj = &lispnil;
static int pagesize, segsize, numseg;
static int nfree, nobjs;
int gcpro;

char *ExpectingListAt = "expecting list, at %s";
char *ExpectingNumberAt = "expecting number, at %s";
FILE *lisp_stdin, *lisp_stdout, *lisp_stderr;

char *LispCharNames[] = {
"Null",		"Soh",		"Stx",		"Etx",
"Eot",		"Enq",		"Ack",		"Bel",
"Backspace",	"Tab",		"Newline",	"Vt",
"Page",		"Return",	"So",		"Si",
"Dle",		"Dc1",		"Dc2",		"Dc3",
"Dc4",		"Nak",		"Syn",		"Etb",
"Can",		"Em",		"Sub",		"Escape",
"Fs",		"Gs",		"Rs",		"Us",
"Space"
};

static LispBuiltin lispbuiltins[] = {
    {"*",			Lisp_Mul,			  1,  0,  0,},
    {"+",			Lisp_Plus,			  1,  0,  0,},
    {"-",			Lisp_Minus,			  1,  1,  0,},
    {"/",			Lisp_Div,			  1,  1,  0,},
    {"1+",			Lisp_OnePlus,			  1,  1,  1,},
    {"1-",			Lisp_OneMinus,			  1,  1,  1,},
    {"<",			Lisp_Less,			  0,  1,  0,},
    {"<=",			Lisp_LessEqual,			  0,  1,  0,},
    {"=",			Lisp_Equal_,			  0,  1,  0,},
    {">",			Lisp_Greater,			  0,  1,  0,},
    {">=",			Lisp_GreaterEqual,		  0,  1,  0,},
    {"/=",			Lisp_NotEqual,			  1,  1,  0,},
    {"APPEND",			Lisp_Append,			  0,  0,  0,},
    {"APPLY",			Lisp_Apply,			  0,  2,  0,},
    {"AND",			Lisp_And,			  0,  0,  0,},
    {"AREF",			Lisp_Aref,			  1,  2,  0,},
    {"ASSOC",			Lisp_Assoc,			  1,  2,  2,},
    {"ATOM",			Lisp_Atom,			  1,  0,  1,},
    {"BLOCK",			Lisp_Block,			  0,  1,  0,},
    {"BUTLAST",			Lisp_Butlast,			  1,  1,  2,},
    {"CAR",			Lisp_Car,			  1,  1,  1,},
    {"CASE",			Lisp_Case,			  0,  1,  0,},
    {"CATCH",			Lisp_Catch,			  0,  1,  0,},
    {"CDR",			Lisp_Cdr,			  1,  1,  1,},
    {"CHAR",			Lisp_Char,			  1,  2,  2,},
    {"SCHAR",			Lisp_Char,			  1,  2,  2,},
    {"CHAR<",			Lisp_CharLess,			  0,  1,  0,},
    {"CHAR<=",			Lisp_CharLessEqual,		  0,  1,  0,},
    {"CHAR=",			Lisp_CharEqual_,		  0,  1,  0,},
    {"CHAR>",			Lisp_CharGreater,		  0,  1,  0,},
    {"CHAR>=",			Lisp_CharGreaterEqual,		  0,  1,  0,},
    {"CHAR/=",			Lisp_CharNotEqual_,		  1,  1,  0,},
    {"CHAR-LESSP",		Lisp_CharLessp,			  0,  1,  0,},
    {"CHAR-NOT-GREATERP",	Lisp_CharNotGreaterp,		  0,  1,  0,},
    {"CHAR-EQUAL",		Lisp_CharEqual,			  0,  1,  0,},
    {"CHAR-GREATERP",		Lisp_CharGreaterp,		  0,  1,  0,},
    {"CHAR-NOT-LESSP",		Lisp_CharNotLessp,		  0,  1,  0,},
    {"CHAR-NOT-EQUAL",		Lisp_CharNotEqual,		  1,  1,  0,},
    {"CHAR-DOWNCASE",		Lisp_CharDowncase,		  1,  1,  1,},
    {"CHAR-INT",		Lisp_CharInt,			  1,  1,  1,},
    {"CHAR-UPCASE",		Lisp_CharUpcase,		  1,  1,  1,},
    {"CHARACTER",		Lisp_Character,			  1,  1,  1,},
    {"COERCE",			Lisp_Coerce,			  1,  2,  2,},
    {"COND",			Lisp_Cond,			  0,  0,  0,},
    {"CONS",			Lisp_Cons,			  1,  2,  2,},
    {"DECF",			Lisp_Decf,			  0,  1,  2,},
    {"DEFMACRO",		Lisp_Defmacro,			  0,  2,  0,},
    {"DEFSTRUCT",		Lisp_Defstruct,			  0,  1,  0,},
    {"DEFUN",			Lisp_Defun,			  0,  2,  0,},
    {"DEFSETF",			Lisp_Defsetf,			  0,  2,  0,},
    {"DO",			Lisp_Do,			  0,  2,  0,},
    {"DO*",			Lisp_DoP,			  0,  2,  0,},
    {"DOLIST",			Lisp_DoList,			  0,  1,  0,},
    {"DOTIMES",			Lisp_DoTimes,			  0,  1,  0,},
    {"ELT",			Lisp_Elt,			  1,  2,  2,},
    {"ENDP",			Lisp_Null,			  1,  1,  1,},
    {"EQUAL",			Lisp_Equal,			  1,  2,  2,},
    {"ERROR",			Lisp_Error,			  1,  1,  0,},
    {"EVAL",			Lisp_Eval,			  1,  1,  1,},
    {"EVENP",			Lisp_Evenp,			  1,  1,  1,},
    {"FIRST",			Lisp_Car,			  1,  1,  1,},
    {"FMAKUNBOUND",		Lisp_FMakunbound,		  1,  1,  1,},
    {"FORMAT",			Lisp_Format,			  1,  2,  0,},
    {"FUNCALL",			Lisp_Funcall,			  0,  1,  0,},
    {"GC",			Lisp_Gc,			  1,  0,  0,},
    {"GET",			Lisp_Get,			  1,  2,  3,},
    {"GO",			Lisp_Go,			  0,  1,  1,},
    {"IF",			Lisp_If,			  0,  2,  3,},
    {"INCF",			Lisp_Incf,			  0,  1,  2,},
    {"INT-CHAR",		Lisp_IntChar,			  1,  1,  1,},
    {"INTEGERP",		Lisp_Integerp,			  1,  1,  1,},
    {"LAST",			Lisp_Last,			  1,  1,  0,},
    {"LAMBDA",			Lisp_Lambda,			  0,  1,  0,},
    {"LENGTH",			Lisp_Length,			  1,  1,  1,},
    {"LET",			Lisp_Let,			  0,  1,  0,},
    {"LET*",			Lisp_LetP,			  0,  1,  0,},
    {"LIST",			Lisp_List,			  1,  0,  0,},
    {"LIST*",			Lisp_ListP,			  0,  1,  0,},
    {"LISTP",			Lisp_Listp,			  1,  1,  1,},
    {"LOAD",			Lisp_Load,			  1,  1,  0,},
    {"LOOP",			Lisp_Loop,			  0,  0,  0,},
    {"MAKE-ARRAY",		Lisp_Makearray,			  1,  1,  0,},
    {"MAKE-LIST",		Lisp_Makelist,			  1,  1,  3,},
    {"MAKUNBOUND",		Lisp_Makunbound,		  1,  1,  1,},
    {"MAPCAR",			Lisp_Mapcar,			  0,  2,  0,},
    {"MAX",			Lisp_Max,			  0,  1,  0,},
    {"MEMBER",			Lisp_Member,			  1,  2,  2,},
    {"MIN",			Lisp_Min,			  0,  1,  0,},
    {"MINUSP",			Lisp_Minusp,			  1,  1,  1,},
    {"NOT",			Lisp_Null,			  1,  0,  1,},
    {"NTH",			Lisp_Nth,			  1,  2,  2,},
    {"NTHCDR",			Lisp_Nthcdr,			  1,  2,  2,},
    {"NULL",			Lisp_Null,			  1,  0,  1,},
    {"NUMBERP",			Lisp_Numberp,			  1,  1,  1,},
    {"ODDP",			Lisp_Oddp,			  1,  1,  1,},
    {"OR",			Lisp_Or,			  0,  0,  0,},
    {"PLUSP",			Lisp_Plusp,			  1,  1,  1,},
    {"PRIN1",			Lisp_Prin1,			  1,  1,  2,},
    {"PRINC",			Lisp_Princ,			  1,  1,  2,},
    {"PRINT",			Lisp_Print,			  1,  1,  2,},
    {"PROG1",			Lisp_Prog1,			  0,  1,  0,},
    {"PROG2",			Lisp_Prog2,			  0,  2,  0,},
    {"PROGN",			Lisp_Progn,			  0,  0,  0,},
    {"PROGV",			Lisp_Progv,			  0,  2,  0,},
    {"PROVIDE",			Lisp_Provide,			  1,  1,  1,},
    {"QUIT",			Lisp_Quit,			  1,  0,  1,},
    {"QUOTE",			Lisp_Quote,			  0,  1,  1,},
    {"READ",			Lisp_Read,			  1,  0,  0,},
    {"REPLACE",			Lisp_Replace,			  1,  2, 10,},
    {"READ-FROM-STRING",	Lisp_ReadFromString,		  1,  1,  9,},
    {"REQUIRE",			Lisp_Require,			  1,  1,  2,},
    {"REST",			Lisp_Cdr,			  1,  1,  1,},
    {"RETURN",			Lisp_Return,			  0,  0,  1,},
    {"RETURN-FROM",		Lisp_ReturnFrom,		  0,  1,  2,},
    {"REVERSE",			Lisp_Reverse,			  1,  1,  1,},
    {"RPLACA",			Lisp_Rplaca,			  1,  2,  2,},
    {"RPLACD",			Lisp_Rplacd,			  1,  2,  2,},
    {"SET",			Lisp_Set,			  1,  2,  0,},
    {"SETF",			Lisp_Setf,			  0,  2,  0,},
    {"SETQ",			Lisp_SetQ,			  0,  2,  0,},
    {"STRING",			Lisp_String,			  1,  1,  1,},
    {"STRINGP",			Lisp_Stringp,			  1,  1,  1,},
    {"STRING=",			Lisp_StringEqual_,		  1,  2, 11,},
    {"STRING<",			Lisp_StringLess,		  1,  2, 11,},
    {"STRING>",			Lisp_StringGreater,		  1,  2, 11,},
    {"STRING<=",		Lisp_StringLessEqual,		  1,  2, 11,},
    {"STRING>=",		Lisp_StringGreaterEqual,	  1,  2, 11,},
    {"STRING/=",		Lisp_StringNotEqual_,		  1,  2, 11,},
    {"STRING-EQUAL",		Lisp_StringEqual,		  1,  2, 11,},
    {"STRING-GREATERP",		Lisp_StringGreaterp,		  1,  2, 11,},
    {"STRING-NOT-EQUAL",	Lisp_StringNotEqual,		  1,  2, 11,},
    {"STRING-NOT-GREATERP",	Lisp_StringNotGreaterp,		  1,  2, 11,},
    {"STRING-NOT-LESSP",	Lisp_StringNotLessp,		  1,  2, 11,},
    {"STRING-LESSP",		Lisp_StringLessp,		  1,  2, 11,},
    {"STRING-TRIM",		Lisp_StringTrim,		  1,  2,  2,},
    {"STRING-LEFT-TRIM",	Lisp_StringLeftTrim,		  1,  2,  2,},
    {"STRING-RIGHT-TRIM",	Lisp_StringRightTrim,		  1,  2,  2,},
    {"STRING-UPCASE",		Lisp_StringUpcase,		  1,  1,  5,},
    {"STRING-DOWNCASE",		Lisp_StringDowncase,		  1,  1,  5,},
    {"STRING-CAPITALIZE",	Lisp_StringCapitalize,		  1,  1,  5,},
    {"SUBSEQ",			Lisp_Subseq,			  1,  2,  3,},
    {"SYMBOLP",			Lisp_Symbolp,			  1,  1,  1,},
    {"SYMBOL-PLIST",		Lisp_SymbolPlist,		  1,  1,  1,},
    {"TAGBODY",			Lisp_Tagbody,			  0,  0,  0,},
    {"TERPRI",			Lisp_Terpri,			  1,  0,  1,},
    {"TYPEP",			Lisp_Typep,			  1,  2,  2,},
    {"THROW",			Lisp_Throw,			  0,  2,  2,},
    {"TIME",			Lisp_Time,			  0,  1,  1,},
    {"UNLESS",			Lisp_Unless,			  0,  1,  0,},
    {"UNTIL",			Lisp_Until,			  0,  1,  0,},
    {"UNWIND-PROTECT",		Lisp_UnwindProtect,		  0,  1,  0,},
    {"VECTOR",			Lisp_Vector,			  1,  0,  0,},
    {"WHEN",			Lisp_When,			  0,  1,  0,},
    {"WHILE",			Lisp_While,			  0,  1,  0,},
    {"XEDIT::CHAR-STORE",	Lisp_XeditCharStore,		  1,  3,  3,},
    {"XEDIT::ELT-STORE",	Lisp_XeditEltStore,		  1,  3,  3,},
    {"XEDIT::MAKE-STRUCT",	Lisp_XeditMakeStruct,		  1,  1,  0,},
    {"XEDIT::PUT",		Lisp_XeditPut,			  1,  3,  3,},
    {"XEDIT::STRUCT-ACCESS",	Lisp_XeditStructAccess,		  1,  2,  2,},
    {"XEDIT::STRUCT-TYPE",	Lisp_XeditStructType,		  1,  2,  2,},
    {"XEDIT::STRUCT-STORE",	Lisp_XeditStructStore,		  1,  3,  3,},
    {"XEDIT::VECTOR-STORE",	Lisp_XeditVectorStore,		  1,  3,  0,},
    {"ZEROP",			Lisp_Zerop,			  1,  1,  1,},
};

/*
 * Implementation
 */
static int
GetPageSize(void)
{
    static int pagesize = -1;

    if (pagesize != -1)
	return pagesize;

    /* Try each supported method in the preferred order */

#if defined(_SC_PAGESIZE) || defined(HAS_SC_PAGESIZE)
    pagesize = sysconf(_SC_PAGESIZE);
#endif

#ifdef _SC_PAGE_SIZE
    if (pagesize == -1)
	pagesize = sysconf(_SC_PAGE_SIZE);
#endif

#ifdef HAS_GETPAGESIZE
    if (pagesize == -1)
	pagesize = getpagesize();
#endif

#ifdef PAGE_SIZE
    if (pagesize == -1)
	pagesize = PAGE_SIZE;
#endif

    if (pagesize == -1)
	pagesize = 0;

    return pagesize;
}

void
LispDestroy(LispMac *mac, char *fmt, ...)
{
    if (!mac->destroyed) {
	va_list ap;

	if (!mac->newline)
	    fputc('\n', lisp_stderr);
	fprintf(lisp_stderr, "%s", "*** Error: ");

	va_start(ap, fmt);
	vfprintf(lisp_stderr, fmt, ap);
	va_end(ap);

	fputc('\n', lisp_stderr);
	fflush(lisp_stderr);

	mac->column = 0;
	mac->newline = 1;

	if (mac->debugging) {
	    LispDebugger(mac, LispDebugCallWatch, NIL, NIL);
	    LispDebugger(mac, LispDebugCallFatal, NIL, NIL);
	}

	mac->destroyed = 1;
	LispBlockUnwind(mac);
	if (mac->errexit)
	    exit(1);
    }

    if (mac->debugging) {
	/* when stack variables could be changed, this must be also changed! */
	mac->debug_level = -1;
	mac->debug = LispDebugUnspec;
    }

    while (mac->mem.mem_level)
	free(mac->mem.mem[--mac->mem.mem_level]);

    /* panic */
    LispTopLevel(mac);
    if (mac->st) {
	mac->cp = &(mac->st[strlen(mac->st)]);
	mac->tok = 0;
    }

    if (!mac->running) {
	fprintf(lisp_stderr, "*** Fatal: nowhere to longjmp.\n");
	abort();
    }

    siglongjmp(mac->jmp, 1);
}

void
LispTopLevel(LispMac *mac)
{
    ENV = GLB;
    LEX = COD = FRM = NIL;
    if (mac->debugging) {
	DBG = NIL;
	if (mac->debug == LispDebugFinish)
	    mac->debug = LispDebugUnspec;
	mac->debug_level = -1;
	mac->debug_step = 0;
    }
    gcpro = 0;
    mac->block.block_level = 0;
    if (mac->block.block_size) {
	while (mac->block.block_size)
	    free(mac->block.block[--mac->block.block_size]);
	free(mac->block.block);
	mac->block.block = NULL;
    }

    mac->destroyed = 0;
    mac->princ = mac->justsize = 0;
    if (mac->stream.stream_level) {
	free(mac->st);
	if (mac->stream.stream[mac->stream.stream_level].fp)
	    fclose(mac->stream.stream[mac->stream.stream_level].fp);
	--mac->stream.stream_level;
	while (mac->stream.stream_level) {
	    if (mac->stream.stream[mac->stream.stream_level].fp)
		fclose(mac->stream.stream[mac->stream.stream_level].fp);
	    free(mac->stream.stream[mac->stream.stream_level].st);
	    --mac->stream.stream_level;
	}
	mac->fp = mac->stream.stream[0].fp;
	mac->st = mac->stream.stream[0].st;
	mac->cp = mac->stream.stream[0].cp;
	mac->tok = mac->stream.stream[0].tok;
    }
    if (mac->mem.mem_level) {
	fprintf(lisp_stderr, "*** Warning: %d raw memory pointer(s) left. "
		"Probably a leak.\n", mac->mem.mem_level);
	mac->mem.mem_level = 0;
    }

    fflush(lisp_stdout);
    fflush(lisp_stderr);
}

void
LispGC(LispMac *mac, LispObj *car, LispObj *cdr)
{
    LispObj *entry;
    unsigned i, j;
    LispAtom *atom, *patom, *natom;
#ifdef DEBUG
    struct timeval start, end;
    long sec, msec;
    int count = nfree;
    int strings_free = 0;
#endif

    if (gcpro)
	return;

#ifdef DEBUG
    fprintf(lisp_stderr, "gc: ");
    gettimeofday(&start, NULL);
#endif

    LispMark(MOD);
    LispMark(GLB);
    LispMark(ENV);
    LispMark(COD);
    LispMark(FRM);
    LispMark(RUN[0]);
    LispMark(RUN[1]);
    LispMark(RUN[2]);
    LispMark(RES[0]);
    LispMark(RES[1]);
    LispMark(RES[2]);
    LispMark(DBG);
    LispMark(BRK);
    LispMark(PRO);
    LispMark(car);
    LispMark(cdr);

    /* Make all strings candidate to be released */
    for (i = 0; i < STRTBLSZ; i++) {
	atom = mac->strs[i];
	while (atom) {
	    atom->mark = LispNil_t;
	    if (atom->property) {
		if (atom->property->property)
		    LispMark(atom->property->properties);
		if (atom->property->function)
		    LispMark(atom->property->fun.function);
		if (atom->property->defsetf)
		    LispMark(atom->property->setf);
		if (atom->property->defstruct)
		    LispMark(atom->property->structure.definition);
	    }
	    atom = atom->next;
	}
    }

    for (j = 0; j < numseg; j++)
	for (i = 0, entry = objseg[j]; i < segsize; i++, entry++) {
	    if (entry->prot)
		entry->dirty = entry->mark = LispTrue_t;
	    else if (entry->mark)
		entry->mark = LispNil_t;
	    else if (entry->dirty) {
		if (entry->type == LispStream_t) {
		    if (entry->data.stream.size < 0)
			fclose(entry->data.stream.source.fp);
		    else
			free(entry->data.stream.source.str);
		}
		CAR(entry) = NIL;
		CDR(entry) = freeobj;
		freeobj = entry;
		entry->dirty = LispNil_t;
		++nfree;
	    }
	}

    /* Needs a new pass because of the 'prot' field of cells. */
    for (j = 0; j < numseg; j++)
	for (i = 0, entry = objseg[j]; i < segsize; i++, entry++) {
	    if (entry->dirty || entry->mark) {
		switch (entry->type) {
		    case LispAtom_t:
		    case LispString_t:
			entry->data.atom->mark = LispTrue_t;
			break;
		    default:
			break;
		}
	    }
	}

    /* Free unused strings */
    for (i = 0; i < STRTBLSZ; i++) {
	patom = atom = mac->strs[i];
	while (atom) {
	    natom = atom->next;
	    if (!atom->property &&
		!atom->prot && atom->mark == LispNil_t) {
		/* it is not required to call LispFree here */
		if (!atom->prot)
		    free(atom->string);
		free(atom);
		if (patom == atom)
		    patom = mac->strs[i] = natom;
		else
		    patom->next = natom;
#ifdef DEBUG
		++strings_free;
#endif
	    }
	    else
		patom = atom;
	    atom = natom;
	}
    }

#ifdef DEBUG
    gettimeofday(&end, NULL);
    sec = end.tv_sec - start.tv_sec;
    msec = end.tv_usec - start.tv_usec;
    if (msec < 0) {
	--sec;
	msec += 1000000;
    }
    fprintf(lisp_stderr, "%ld sec, %ld msec, ", sec, msec);
    fprintf(lisp_stderr, "%d recovered, %d free, %d protected, %d total\n", nfree - count, nfree, nobjs - nfree, nobjs);
    fprintf(lisp_stderr, "%d string(s) freed\n", strings_free);
#endif
}

void
LispCheckMemLevel(LispMac *mac)
{
    if (mac->mem.mem_level + 1 >= mac->mem.mem_size) {
	void **ptr = (void**)realloc(mac->mem.mem,
				     (mac->mem.mem_size + 16) * sizeof(void*));

	if (ptr == NULL)
	    LispDestroy(mac, "out of memory");
	mac->mem.mem = ptr;
	mac->mem.mem_size += 16;
    }
}

void
LispMused(LispMac *mac, void *pointer)
{
    int i;

    for (i = 0; i < mac->mem.mem_level; i++)
	if (mac->mem.mem[i] == pointer) {
	    --mac->mem.mem_level;
	    if (mac->mem.mem_level > i)
		memmove(mac->mem.mem + i, mac->mem.mem + i + 1,
			sizeof(void*) * (mac->mem.mem_level - i));
	    break;
	}
}

void *
LispMalloc(LispMac *mac, unsigned size)
{
    void *pointer;

    LispCheckMemLevel(mac);
    if ((pointer = malloc(size)) == NULL)
	LispDestroy(mac, "out of memory, couldn't allocate %u bytes", size);

    mac->mem.mem[mac->mem.mem_level++] = pointer;

    return (pointer);
}

void *
LispCalloc(LispMac *mac, unsigned nmemb, unsigned size)
{
    void *pointer;

    LispCheckMemLevel(mac);
    if ((pointer = calloc(nmemb, size)) == NULL)
	LispDestroy(mac, "out of memory, couldn't allocate %u bytes", size);

    mac->mem.mem[mac->mem.mem_level++] = pointer;

    return (pointer);
}

void *
LispRealloc(LispMac *mac, void *pointer, unsigned size)
{
    void *ptr;
    int i;

    for (i = 0; i < mac->mem.mem_level; i++)
	if (mac->mem.mem[i] == pointer)
	    break;
    if (i == mac->mem.mem_level)
	LispCheckMemLevel(mac);

    if ((ptr = realloc(pointer, size)) == NULL)
	LispDestroy(mac, "out of memory, couldn't realloc");

    if (i == mac->mem.mem_level)
	mac->mem.mem[mac->mem.mem_level++] = ptr;
    else
	mac->mem.mem[i] = ptr;

    return (ptr);
}

char *
LispStrdup(LispMac *mac, char *str)
{
    char *ptr = LispMalloc(mac, strlen(str) + 1);

    strcpy(ptr, str);

    return (ptr);
}

void
LispFree(LispMac *mac, void *pointer)
{
    int i;

    for (i = 0; i < mac->mem.mem_level; i++)
	if (mac->mem.mem[i] == pointer)
	    break;

    /* If the memory was allocated on a previous form, just free it */
    if (i < mac->mem.mem_level) {
	memmove(mac->mem.mem + i, mac->mem.mem + i + 1,
		sizeof(void*) * (mac->mem.mem_level - i - 1));
	--mac->mem.mem_level;
    }

    free(pointer);
}

LispObj *
LispSetVariable(LispMac *mac, LispObj *var, LispObj *val, char *fname, int eval)
{
    return (_LispSet(mac, var, val, fname, eval));
}

int
LispRegisterOpaqueType(LispMac *mac, char *desc)
{
    LispOpaque *opaque;
    int ii = 0;
    char *pp = desc;

    while (*pp)
	ii = (ii << 1) ^ *pp++;
    if (ii < 0)
	ii = -ii;
    ii %= STRTBLSZ;
    for (opaque = mac->opqs[ii]; opaque; opaque = opaque->next)
	if (strcmp(opaque->desc, desc) == 0)
	    return (opaque->type);
    opaque = (LispOpaque*)LispMalloc(mac, sizeof(LispOpaque));
    opaque->desc = LispDoGetAtom(mac, desc, 1, 0)->string;
    opaque->next = mac->opqs[ii];
    mac->opqs[ii] = opaque;
    LispMused(mac, opaque);

    return (opaque->type = ++mac->opaque);
}

static char *
LispIntToOpaqueType(LispMac *mac, int type)
{
    int i;
    LispOpaque *opaque;

    if (type) {
	for (i = 0; i < STRTBLSZ; i++) {
	    opaque = mac->opqs[i];
	    while (opaque) {
		if (opaque->type == type)
		    return (opaque->desc);
		opaque = opaque->next;
	    }
	}
	LispDestroy(mac, "Opaque type %d not registered", type);
    }

    return ("NIL");
}

LispAtom *
LispDoGetAtom(LispMac *mac, char *str, int prot, int perm)
{
    LispAtom *atom;
    int ii = 0;
    char *pp = str;

    while (*pp)
	ii = (ii << 1) ^ *pp++;
    if (ii < 0)
	ii = -ii;
    ii %= STRTBLSZ;
    for (atom = mac->strs[ii]; atom; atom = atom->next)
	if (strcmp(atom->string, str) == 0) {
	    if (prot && !atom->prot)
		atom->prot = 1;
	    return (atom);
	}
    atom = (LispAtom*)LispMalloc(mac, sizeof(LispAtom));
    if (perm)
	atom->string = str;
    else
	atom->string = LispStrdup(mac, str);
    LispMused(mac, atom);
    if (!perm)
	LispMused(mac, atom->string);
    atom->property = NULL;
    atom->next = mac->strs[ii];
    mac->strs[ii] = atom;
    atom->dirty = 1;
    atom->mark = 0;
    atom->prot = !!prot;

    return (atom);
}

static LispProperty *
LispAllocAtomProperty(LispMac *mac)
{
    LispProperty *prop = LispCalloc(mac, 1, sizeof(LispProperty));

    return (prop);
}

static void
LispCheckAtomProperty(LispMac *mac, LispAtom *atom)
{
    if (atom->property) {
	LispProperty *prop = atom->property;

	if (prop->object && prop->value == NULL)
		prop->object = 0;
	if ((prop->function || prop->builtin) && prop->fun.function == NULL)
		prop->function = prop->builtin = 0;
	if (prop->property && prop->properties == NULL)
	    prop->property = 0;
	if (prop->defsetf && prop->setf == NULL)
	    prop->defsetf = 0;
	if (prop->defstruct && prop->structure.definition == NULL)
	    prop->defstruct = 0;

	if (prop->object == 0 && prop->function == 0 && prop->builtin == 0 &&
	    prop->properties == 0 && prop->defsetf == 0 &&
	    prop->defstruct == 0) {
	    LispFree(mac, atom->property);
	    atom->property = NULL;
	}
    }
}

void
LispSetAtomObjectProperty(LispMac *mac, LispAtom *atom, LispObj *object)
{
    LispProperty *prop = atom->property;

    if (prop == NULL)
	prop = LispAllocAtomProperty(mac);

    prop->object = 1;
    prop->value = object;

    if (atom->property == NULL) {
	LispMused(mac, prop);
	atom->property = prop;
    }
}

void
LispRemAtomObjectProperty(LispMac *mac, LispAtom *atom)
{
    if (atom->property && atom->property->value) {
	atom->property->value = NULL;
	LispCheckAtomProperty(mac, atom);
    }
    else
	LispDestroy(mac, "internal error at INTERNAL:REMOVE-OBJECT-PROPERTY");
}

void
LispSetAtomFunctionProperty(LispMac *mac, LispAtom *atom, LispObj *function)
{
    if (atom->property == NULL) {
	atom->property = LispAllocAtomProperty(mac);
	LispMused(mac, atom->property);
    }

    if (atom->property->fun.function == NULL)
	atom->property->function = 1;
    else
	atom->property->builtin = 0;	/* just make sure it is unset */

    atom->property->fun.function = function;
}

void
LispRemAtomFunctionProperty(LispMac *mac, LispAtom *atom)
{
    if (atom->property && atom->property->fun.function) {
	atom->property->fun.function = NULL;
	LispCheckAtomProperty(mac, atom);
    }
    else
	LispDestroy(mac, "internal error at INTERNAL:REMOVE-FUNCTION-PROPERTY");
}

void
LispSetAtomBuiltinProperty(LispMac *mac, LispAtom *atom, LispBuiltin *builtin)
{
    if (atom->property == NULL) {
	atom->property = LispAllocAtomProperty(mac);
	LispMused(mac, atom->property);
    }

    if (atom->property->fun.function == NULL)
	atom->property->builtin = 1;
    else
	atom->property->function = 0;	/* just make sure it is unset */

    atom->property->fun.builtin = builtin;
}

void
LispRemAtomBuiltinProperty(LispMac *mac, LispAtom *atom)
{
    if (atom->property && atom->property->fun.builtin) {
	atom->property->fun.function = NULL;
	LispCheckAtomProperty(mac, atom);
    }
    else
	LispDestroy(mac, "internal error at INTERNAL:REMOVE-BUILTIN-PROPERTY");
}

void
LispSetAtomSetfProperty(LispMac *mac, LispAtom *atom, LispObj *setf)
{
    LispProperty *prop = atom->property;

    if (prop == NULL)
	prop = LispAllocAtomProperty(mac);

    prop->defsetf = 1;
    prop->setf = setf;

    if (atom->property == NULL) {
	LispMused(mac, prop);
	atom->property = prop;
    }
}

void
LispRemAtomSetfProperty(LispMac *mac, LispAtom *atom)
{
    if (atom->property && atom->property->setf) {
	atom->property->setf = NULL;
	LispCheckAtomProperty(mac, atom);
    }
    else
	LispDestroy(mac, "internal error at INTERNAL:REMOVE-SETF-PROPERTY");
}

void
LispSetAtomStructProperty(LispMac *mac, LispAtom *atom, LispObj *def, int fun)
{
    LispProperty *prop = atom->property;

    if (prop == NULL)
	prop = LispAllocAtomProperty(mac);

    prop->defstruct = 1;
    prop->structure.definition = def;
    prop->structure.function = fun;

    if (atom->property == NULL) {
	LispMused(mac, prop);
	atom->property = prop;
    }
}

void
LispRemAtomStructProperty(LispMac *mac, LispAtom *atom)
{
    if (atom->property && atom->property->defstruct) {
	atom->property->structure.definition = NULL;
	LispCheckAtomProperty(mac, atom);
    }
    else
	LispDestroy(mac, "internal error at INTERNAL:REMOVE-STRUCTURE-PROPERTY");
}

char *
LispGetString(LispMac *mac, char *str)
{
    return (LispDoGetAtom(mac, str, 0, 0)->string);
}

char *
LispGetPermString(LispMac *mac, char *str)
{
    return (LispDoGetAtom(mac, str, 1, 1)->string);
}

static LispObj *
LispDoGetAtomProperty(LispMac *mac, LispAtom *atom, LispObj *key, int add)
{
    LispObj *obj, *res = NULL;
    LispProperty *prop = atom->property;

    if (prop == NULL && add)
	prop = LispAllocAtomProperty(mac);

    if (add && prop->properties == NULL) {
	prop->property = 1;
	prop->properties = NIL;
    }

    if (prop && prop->property) {
	for (obj = prop->properties; obj != NIL; obj = CDR(CDR(obj))) {
	    if (_LispEqual(mac, key, CAR(obj)) == T) {
		res = CDR(obj);
		break;
	    }
	}
    }
    else
	obj = NIL;

    if (obj == NIL) {
	if (add) {
	    prop->properties = CONS(key, CONS(NIL, prop->properties));
	    res = CDR(prop->properties);
	}
	else
	    res = NIL;
    }

    if (atom->property == NULL && add) {
	LispMused(mac, prop);
	atom->property = prop;
    }

    return (res);
}

LispObj *
LispGetAtomProperty(LispMac *mac, LispAtom *atom, LispObj *key)
{
    return (LispDoGetAtomProperty(mac, atom, key, 0));
}

LispObj *
LispPutAtomProperty(LispMac *mac, LispAtom *atom, LispObj *key, LispObj *val)
{
    LispObj *res = LispDoGetAtomProperty(mac, atom, key, 1);

    CAR(res) = val;

    return (res);
}

void
LispAddBuiltinFunction(LispMac *mac, LispBuiltin *builtin)
{
    LispAtom *atom = LispDoGetAtom(mac, builtin->name, 0, 1);

    LispSetAtomBuiltinProperty(mac, atom, builtin);
}

void
LispAllocSeg(LispMac *mac)
{
    unsigned i;
    LispObj **list, *obj;

    if ((obj = (LispObj*)calloc(1, sizeof(LispObj) * segsize)) == NULL)
	LispDestroy(mac, "out of memory");
    if ((list = (LispObj**)realloc(objseg,
				   sizeof(LispObj*) * (numseg + 1))) == NULL) {
	free(obj);
	LispDestroy(mac, "out of memory");
    }
    objseg = list;
    objseg[numseg] = obj;

    nfree += segsize;
    nobjs += segsize;
    for (i = 0; i < segsize - 1; i++, obj++) {
	CAR(obj) = NIL;
	CDR(obj) = obj + 1;
    }
    CAR(obj) = NIL;
    CDR(obj) = freeobj;
    freeobj = objseg[numseg];
    ++numseg;
#ifdef DEBUG
    fprintf(lisp_stdout, "gc: %d cell(s) allocated at %d segment(s)\n", nobjs, numseg);
#endif
}

void
LispMark(LispObj *obj)
{
    if (obj->mark)
	return;

    switch (obj->type) {
	case LispNil_t:
	    if (obj == NIL)
		return;
	    break;
	case LispTrue_t:
	    if (obj == T)
		return;
	    break;
	case LispLambda_t:
	    LispMark(obj->data.lambda.name);
	    LispMark(obj->data.lambda.code);
	    break;
	case LispQuote_t:
	case LispBackquote_t:
	    LispMark(obj->data.quote);
	    break;
	case LispComma_t:
	    LispMark(obj->data.comma.eval);
	    break;
	case LispCons_t:
	    /* circular list on car */
	    if (CAR(obj) == obj) {
		obj->mark = LispTrue_t;
		obj = CDR(obj);
	    }
	    for (; obj->type == LispCons_t && obj->mark == LispNil_t;
		 obj = CDR(obj)) {
		LispMark(CAR(obj));
		obj->mark = LispTrue_t;
	    }
	    if (obj->type != LispCons_t)
		LispMark(obj);
	    return;
	case LispArray_t:
	    LispMark(obj->data.array.list);
	    LispMark(obj->data.array.dim);
	    break;
	case LispStruct_t:
	    /* def is protected when protecting STR */
	    LispMark(obj->data.struc.fields);
	    break;
	default:
	    break;
    }
    obj->mark = LispTrue_t;
}

/* It is better to keep the prot field unused for now. It should be
 * be set only once, and thus, only used for constants.
 * It was being used to protect arguments to Xt callbacks, but since
 * Xt widgets can be destroyed, and arguments may be shared, it is
 * required to have a "key" associated with every protected object/list.
 */
void
LispProtect(LispMac *mac, LispObj *key, LispObj *list)
{
    PRO = CONS(CONS(key, list), PRO);
}

void
LispUProtect(LispMac *mac, LispObj *key, LispObj *list)
{
    LispObj *prev, *obj;

    for (prev = obj = PRO; obj != NIL; prev = obj, obj = CDR(obj))
	if (CAR(CAR(obj)) == key && CDR(CAR(obj)) == list) {
	    if (obj == PRO)
		PRO = CDR(PRO);
	    else
		CDR(prev) = CDR(obj);
	    return;
	}

    LispDestroy(mac, "no match for (%s %s), at INTERNAL:UPROTECT",
		LispStrObj(mac, key), LispStrObj(mac, list));
}

int
LispGet(LispMac *mac)
{
    int ch;

    if (mac->tok == EOF)
	return (EOF);

    if (mac->cp)
	ch = *mac->cp;
    else
	ch = 0;

    if (!ch) {
	if (mac->fp) {
	    char *ret;
	    char *code = malloc(1024);
	    int len;

	    if (code) {
		ret = fgets(code, 1024, mac->fp);
		len = ret ? strlen(code) : -1;
	    }
	    else
		len = -1;
	    if (len <= 0) {
		free(code);
		return (mac->tok = EOF);
	    }
	    if (mac->level == 0 || !mac->st) {
		if (mac->st)
		    free(mac->st);
		mac->st = mac->cp = code;
	    }
	    else {
		char *tmp = realloc(mac->st, (len = strlen(mac->st)) + strlen(code) + 1);

		if (!tmp) {
		    free(mac->st);
		    mac->st = NULL;
		    return (mac->tok = EOF);
		}
		mac->cp = &tmp[len];
		mac->st = tmp;
		strcpy(mac->cp, code);
		free(code);
	    }
	    return (LispGet(mac));
	}
	else
	    return (mac->tok = EOF);
    }

    ++mac->cp;
    if (ch == '\n' && mac->interactive && mac->fp == lisp_stdin) {
	mac->newline = 1;
	mac->column = 0;
    }

    return (mac->tok = ch);
}

int
LispUnget(LispMac *mac)
{
    if (mac->cp > mac->st) {
	--mac->cp;
	return (1);
    }
    return (0);
}

LispObj *
LispNew(LispMac *mac, LispObj *car, LispObj *cdr)
{
    LispObj *obj;

    if (nfree < (segsize >> 2)) {
	/* changed if test from
	 *	if (freeobj == NIL)
	 * to
	 *	if (nfree < (segsize >> 2))
	 * this is required, because since gc can be disabled, it is possible
	 * to enter loops where code will fill exactly segsize objects, and
	 * gc protect just before allocating a new object.
	 *   An example code that would generate such problem is:
	 *	(setq n 1e6)
	 *	(loop (if (<= n 0) (return n) (setf n (- n 1))))
	 */
	LispGC(mac, car, cdr);
	if (freeobj == NIL)
	    LispAllocSeg(mac);
	else if (nfree < (segsize >> 2)) {
	    /* allocates new segment anyway, to avoid too much calls to gc */
	    obj = freeobj;
	    while (CDR(freeobj) != NIL)
		freeobj = CDR(freeobj);
	    cdr = freeobj;
	    freeobj = NIL;
	    LispAllocSeg(mac);
	    CDR(cdr) = objseg[numseg - 1];
	    freeobj = obj;
	}
    }

    obj = freeobj;
    freeobj = CDR(obj);

    obj->dirty = LispTrue_t;
    obj->prot = LispNil_t;
    --nfree;

    return (obj);
}

LispObj *
LispNewAtom(LispMac *mac, char *str)
{
    char *ptr = str;
    LispObj *atom = LispNew(mac, NIL, NIL);

    /* store atoms as uppercase */
    while (*ptr) {
	if (toupper(*ptr) != *ptr) {
	    unsigned char *tmp;

	    ptr = LispStrdup(mac, str);
	    for (tmp = (unsigned char*)ptr; *tmp; tmp++)
		*tmp = toupper(*tmp);
	    break;
	}
	++ptr;
    }
    if (*ptr == '\0')
	ptr = str;

    atom->type = LispAtom_t;
    atom->data.atom = LispDoGetAtom(mac, ptr, 0, 0);
    if (ptr != str)
	LispFree(mac, ptr);

    return (atom);
}

LispObj *
LispNewReal(LispMac *mac, double value)
{
    LispObj *real = LispNew(mac, NIL, NIL);

    real->type = LispReal_t;
    real->data.real = value;

    return (real);
}

LispObj *
LispNewString(LispMac *mac, char *str)
{
    LispObj *string = LispNew(mac, NIL, NIL);

    string->type = LispString_t;
    string->data.atom = LispDoGetAtom(mac, str, 0, 0);

    return (string);
}

LispObj *
LispNewCharacter(LispMac *mac, long c)
{
    LispObj *character = LispNew(mac, NIL, NIL);

    character->type = LispCharacter_t;
    character->data.integer = c;

    return (character);
}

LispObj *
LispNewInteger(LispMac *mac, long i)
{
    LispObj *integer = LispNew(mac, NIL, NIL);

    integer->type = LispInteger_t;
    integer->data.integer = i;

    return (integer);
}

LispObj *
LispNewQuote(LispMac *mac, LispObj *obj)
{
    LispObj *quote = LispNew(mac, obj, NIL);

    quote->type = LispQuote_t;
    quote->data.quote = obj;

    return (quote);
}

LispObj *
LispNewBackquote(LispMac *mac, LispObj *obj)
{
    LispObj *backquote = LispNew(mac, obj, NIL);

    backquote->type = LispBackquote_t;
    backquote->data.quote = obj;

    return (backquote);
}

LispObj *
LispNewComma(LispMac *mac, LispObj *obj, int atlist)
{
    LispObj *comma = LispNew(mac, obj, NIL);

    comma->type = LispComma_t;
    comma->data.comma.eval = obj;
    comma->data.comma.atlist = atlist;

    return (comma);
}

LispObj *
LispNewCons(LispMac *mac, LispObj *car, LispObj *cdr)
{
    LispObj *cons = LispNew(mac, car, cdr);

    cons->type = LispCons_t;
    CAR(cons) = car;
    CDR(cons) = cdr;

    return (cons);
}

LispObj *
LispNewLambda(LispMac *mac, LispObj *name, LispObj *args, LispObj *code,
	      int num_args, LispFunType type, int key, int optional, int rest)
{
    LispObj *fun = LispNew(mac, args, code);

    fun->type = LispLambda_t;
    fun->data.lambda.name = name;
    GCProtect();
    fun->data.lambda.code = CONS(args, code);
    GCUProtect();
    fun->data.lambda.num_args = num_args;
    fun->data.lambda.type = type;
    fun->data.lambda.key = key;
    fun->data.lambda.optional = optional;
    fun->data.lambda.rest = rest;

    return (fun);
}

LispObj *
LispNewStruct(LispMac *mac, LispObj *fields, LispObj *def)
{
    LispObj *struc = LispNew(mac, fields, def);

    struc->type = LispStruct_t;
    struc->data.struc.fields = fields;
    struc->data.struc.def = def;

    return (struc);
}

LispObj *
LispNewOpaque(LispMac *mac, void *data, int type)
{
    LispObj *opaque = LispNew(mac, NIL, NIL);

    opaque->type = LispOpaque_t;
    opaque->data.opaque.data = data;
    opaque->data.opaque.type = type;

    return (opaque);
}

LispObj *
LispGetVar(LispMac *mac, LispObj *atom)
{
    LispObj *env;
    LispAtom *name = atom->data.atom;
    /* XXX no type checking for maximal speed,
     * if got here, atom must be of an ATOM */

    if (ENV != GLB) {
	for (env = ENV; env != LEX; env = CDR(env))
	    if (CAAR(env)->data.atom == name)
		return (CDAR(env));

	if (LEX != NIL) {
	    for (env = GLB; env != NIL; env = CDR(env))
		if (CAAR(env)->data.atom == name)
		    return (CDAR(env));
	}
	return (NULL);
    }

    /* atom->property is only set for global variables */
    return (name->property ? name->property->value : NULL);
}

LispObj *
LispGetVarCons(LispMac *mac, LispObj *atom)
{
    LispObj *env;
    LispAtom *name = atom->data.atom;
    /* XXX no type checking for maximal speed,
     * if got here, atom must be an ATOM */

    for (env = ENV; env != LEX; env = CDR(env))
	if (CAAR(env)->data.atom == name)
	    return (CAR(env));

    if (LEX != NIL) {
	for (env = GLB; env != NIL; env = CDR(env))
	    if (CAAR(env)->data.atom == name)
		return (CAR(env));
    }

    /* if returns NULL, variable is unbound */
    return (NULL);
}

/* Only removes global variables. To be called by makunbound
 * Local variables are unbounded once their block is closed anyway.
 */
void
LispUnsetVar(LispMac *mac, LispObj *atom)
{
    LispObj *env, *prv;
    LispAtom *name = atom->data.atom;
    /* XXX no type checking for maximal speed,
     * if got here, atom must be an ATOM */

    for (prv = env = GLB; env != NIL; prv = env, env = CDR(env))
	if (CAAR(env)->data.atom == name) {
	    if (env == GLB)
		GLB = CDR(GLB);
	    else
		CDR(prv) = CDR(env);
	    if (env == ENV)
		ENV = CDR(ENV);
	    LispRemAtomObjectProperty(mac, name);
	    break;
	}
}

LispObj *
LispAddVar(LispMac *mac, LispObj *atom, LispObj *obj)
{
    LispObj *env;
    LispAtom *name = atom->data.atom;
    /* XXX no type checking for maximal speed,
     * if got here, atom must be an ATOM */

    if (LEX != NIL) {
	for (env = ENV; env != LEX; env = CDR(env))
	    if (CAAR(env)->data.atom == name)
		return (CDAR(env) = obj);
    }
    else {
	for (env = ENV; env != GLB; env = CDR(env))
	    if (CAAR(env)->data.atom == name)
		return (CDAR(env) = obj);
    }

    GCProtect();
    ENV = CONS(CONS(atom, obj), ENV);
    GCUProtect();

    return (obj);
}

LispObj *
LispSetVar(LispMac *mac, LispObj *atom, LispObj *obj)
{
    LispObj *env;
    LispAtom *name = atom->data.atom;
    /* XXX no type checking for maximal speed,
     * if got here, atom must be an ATOM */

    for (env = ENV; env != GLB; env = CDR(env))
	if (CAAR(env)->data.atom == name)
	    return (CDAR(env) = obj);

    for (env = GLB; env != NIL; env = CDR(env))
	if (CAAR(env)->data.atom == name)
	    return (CDAR(env) = CAAR(env)->data.atom->property->value = obj);

    GCProtect();
    LispSetAtomObjectProperty(mac, name, obj);
    if (GLB == NIL)
	ENV = GLB = CONS(CONS(atom, obj), NIL);
    else {
	CDR(GLB) = CONS(CAR(GLB), CDR(GLB));
	CAR(GLB) = CONS(atom, obj);
    }
    GCUProtect();

    return (obj);
}

LispObj *
LispReverse(LispObj *list)
{
    LispObj *tmp, *res = NIL;

    while (list != NIL) {
	tmp = CDR(list);
	CDR(list) = res;
	res = list;
	list = tmp;
    }

    return (res);
}

LispObj *
LispEnvRun(LispMac *mac, LispObj *args, LispFunPtr fn, char *fname, int refs)
{
    LispObj *old_frm, *old_env, *env, *res, *list, *pair;

    old_frm = FRM;
    old_env = ENV;
    env = CAR(args);
    list = NIL;

    if (env != NIL) {
	if (env->type != LispCons_t)
	    LispDestroy(mac, "%s is not of type list, at %s",
			LispStrObj(mac, env), fname);
    }

    for (; env != NIL; env = CDR(env)) {
	LispObj *var = NIL, *val = NIL;

	pair = CAR(env);
	if (SYMBOL_P(pair)) {
	    var = pair;
	    val = NIL;
	}
	else if (pair->type == LispCons_t) {
	    var = CAR(pair);
	    if (!SYMBOL_P(var))
		LispDestroy(mac, "%s is invalid as a variable name, at %s",
			    LispStrObj(mac, var), fname);
	    pair = CDR(pair);
	    if (pair == NIL)
		val = NIL;
	    else {
		val = CAR(pair);
		if (CDR(pair) != NIL)
		    LispDestroy(mac, "too much arguments to initialize %s, at %s",
				STRPTR(var), fname);
	    }
	}
	else
	    LispDestroy(mac, "%s is not of type list, at %s",
			LispStrObj(mac, pair), fname);
	val = EVAL(val);
	if (!refs) {
	    GCProtect();
	    pair = CONS(var, val);
	    if (list == NIL) {
		list = CONS(pair, NIL);
		FRM = CONS(list, FRM);
	    }
	    else {
		CDR(list) = CONS(CAR(list), CDR(list));
		CAR(list) = pair;
	    }
	    GCUProtect();
	}
	else
	    LispAddVar(mac, var, val);
    }

    if (!refs && list != NIL) {
	/* Need to update CAR(FRM) or will run function without gc protection! */
	list = CAR(FRM) = LispReverse(list);
	for (; list != NIL; list = CDR(list)) {
	    pair = CAR(list);
	    LispAddVar(mac, CAR(pair), CDR(pair));
	}
    }

    res = fn(mac, CDR(args), fname);

    ENV = old_env;
    FRM = old_frm;

    return (res);
}

LispBlock *
LispBeginBlock(LispMac *mac, LispObj *tag, LispBlockType type)
{
    unsigned blevel = mac->block.block_level + 1;
    LispBlock *block = NULL;

    if (blevel > mac->block.block_size) {
	LispBlock **blk = realloc(mac->block.block,
				  sizeof(LispBlock*) * (blevel + 1));

	if (blk == NULL)
	    LispDestroy(mac, "out of memory");
	else if ((block = malloc(sizeof(LispBlock))) == NULL)
	    LispDestroy(mac, "out of memory");
	mac->block.block = blk;
	mac->block.block[mac->block.block_size] = block;
	mac->block.block_size = blevel;
    }
    block = mac->block.block[mac->block.block_level];
    if (type == LispBlockCatch)
	tag = EVAL(tag);
    block->type = type;
    memcpy(&(block->tag), tag, sizeof(LispObj));

    block->level = mac->level;
    block->block_level = mac->block.block_level;

    mac->block.block_level = blevel;

    if (mac->debugging) {
	block->debug_level = mac->debug_level;
	block->debug_step = mac->debug_step;
    }

    return (block);
}

void
LispEndBlock(LispMac *mac, LispBlock *block)
{
    mac->level = block->level;
    mac->block.block_level = block->block_level;

    if (mac->debugging) {
	if (mac->debug_level >= block->debug_level) {
	    while (mac->debug_level > block->debug_level) {
		DBG = CDR(DBG);
		--mac->debug_level;
	    }
	}
	else
	    LispDestroy(mac, "this should never happen: "
			"mac->debug_level < block->debug_level");
	mac->debug_step = block->debug_step;
    }
}

void
LispBlockUnwind(LispMac *mac)
{
    LispBlock *block;
    int blevel = mac->block.block_level;

    while (blevel > 0) {
	block = mac->block.block[--blevel];
	if (block->type == LispBlockProtect)
	    longjmp(block->jmp, 1);
    }
}

static int
LispSkipComment(LispMac *mac)
{
    int ch;

    /*CONSTCOND*/
    while (1) {
	while (ch = LispGet(mac), ch != '\n' && ch != EOF)
	    ;
	if (ch == EOF)
	    return (0);
	while (ch = LispGet(mac), isspace(ch) && ch != EOF)
	    ;
	if (ch == EOF)
	    return (0);
	if (ch != ';') {
	    LispUnget(mac);
	    return (1);
	}
    }
    /*NOTREACHED*/
}

static int
LispSkipWhiteSpace(LispMac *mac)
{
    int ch;

    while (ch = LispGet(mac), isspace(ch) && ch != EOF)
	;
    if (ch == ';') {
	if (!LispSkipComment(mac))
	    return (EOF);
	return (LispGet(mac));
    }
    else if (ch == '#') {
	/* multiline comment */
	if (LispGet(mac) == '|') {
	    int comm = 1;	/* comments may nest */

	    while (1) {
		ch = LispGet(mac);
		if (ch == '|' && LispGet(mac) == '#') {
		    if (--comm == 0)
			return (LispSkipWhiteSpace(mac));		    
		}
		else if (ch == '#' && LispGet(mac) == '|')
		    ++comm;
		else if (ch == EOF)
		    return (EOF);
	    }
	}
	else
	    LispUnget(mac);
    }
    return (ch);
}

void
LispGetKeys(LispMac *mac, char *fname, char *spec, LispObj *list, ...)
{
    va_list ap;
    int nargs, ncvt;
    LispObj *obj, **arg;
    char *ptr, *end, *str;

    /* count how many arguments specified and check arguments */
    for (obj = list, nargs = 0; obj != NIL; obj = CDR(obj)) {
	if (!SYMBOL_P(CAR(obj)) || STRPTR(CAR(obj))[0] != ':')
	    LispDestroy(mac, "&KEY needs arguments as pairs, at %s", fname);
	else if (CDR(obj) == NIL)
	    LispDestroy(mac, "expecting %s value, at %s",
			STRPTR(CAR(obj)), fname);
	obj = CDR(obj);
	++nargs;
    }

    va_start(ap, list);
    for (ncvt = 0, ptr = spec, end = strchr(spec, ':'); end;
	 ptr = end + 1, end = strchr(ptr, ':')) {
	arg = (LispObj**)va_arg(ap, LispObj**);
	*arg = NULL;	/* to know if it was found */

	for (obj = list; obj != NIL; obj = CDR(obj)) {
	    str = STRPTR(CAR(obj)) + 1;
	    obj = CDR(obj);
	    if (strncmp(str, ptr, end - ptr) == 0) {
		*arg = CAR(obj);
		++ncvt;
		break;
	    }
	}
	if (*arg == NULL)
	    *arg = NIL;
    }
    if (ptr) {
	/* last or unique argument */
	arg = (LispObj**)va_arg(ap, LispObj**);
	*arg = NULL;	/* to know if it was found */

	for (obj = list; obj != NIL; obj = CDR(obj)) {
	    str = STRPTR(CAR(obj)) + 1;
	    obj = CDR(obj);
	    if (strcmp(str, ptr) == 0) {
		*arg = CAR(obj);
		++ncvt;
		break;
	    }
	}
	if (*arg == NULL)
	    *arg = NIL;
    }
    va_end(ap);

    /* if got here, arguments are correctly specified as pairs */
    if (ncvt < nargs) {
	/* Possible error. If argument value specified more than once, it is
	 * not triggered as an error, but if an incorrect argument name was
	 * specified, it is a fatal error (probably a typo in the code) */

	for (obj = list; obj != NIL; obj = CDR(CDR(obj))) {
	    int match = 0;

	    for (ptr = spec, end = strchr(ptr, ':'); end;
		 ptr = end + 1, end = strchr(ptr, ':'))
		if (strncmp(STRPTR(CAR(obj)) + 1, ptr, end - ptr) == 0) {
		    match = 1;
		    break;
		}

	    if (!match && ptr && strcmp(STRPTR(CAR(obj)) + 1, ptr) == 0)
		match = 1;

	    if (!match)
		LispDestroy(mac, "%s is not an argument to %s",
			    STRPTR(CAR(obj)), fname);
	}
    }
}

LispObj *
LispRun(LispMac *mac)
{
    static char *DOTMSG = "illegal end of dotted list";
    static char *QUOTMSG = "illegal quoted object";
    int ch, len, dquote = 0, escape = 0, size, dot = 0, hash = 0;
    LispObj *res, *obj, *cons, *code, *frm;
    char stk[1024], *str;

    code = COD;
    frm = FRM;
    switch (ch = LispSkipWhiteSpace(mac)) {
	case '(':
	    if (LispSkipWhiteSpace(mac) == ')') {
		res = NIL;
		break;
	    }
	    (void)LispUnget(mac);
	    res = cons = CONS(NIL, NIL);
	    if (COD == NIL)
		COD = res;
	    else
		FRM = CONS(res, FRM);
	    if ((CAR(cons) = LispRun(mac)) == DOT)
		LispDestroy(mac, "illegal start of dotted list");
	    while ((obj = LispRun(mac)) != EOLIST) {
		if (obj == NULL)
		    goto endofinput;
		if (obj == DOT) {
		    if (dot)
			LispDestroy(mac, DOTMSG);
		    dot = 1;
		}
		else {
		    if (dot) {
			if (++dot > 2)
			    LispDestroy(mac, DOTMSG);
			CDR(cons) = obj;
		    }
		    else {
			CDR(cons) = CONS(obj, NIL);
			cons = CDR(cons);
		    }
		}
	    }
	    if (dot == 1)
		LispDestroy(mac, DOTMSG);
	    break;
	case ')':
	    return (EOLIST);
	case EOF:
	    return (NULL);
	case '\'':
	    if ((obj = LispRun(mac)) == NULL || obj == EOLIST)
		LispDestroy(mac, QUOTMSG);
	    res = QUOTE(obj);
	    break;
	case '`':
	    if ((obj = LispRun(mac)) == NULL || obj == EOLIST)
		LispDestroy(mac, QUOTMSG);
	    res = BACKQUOTE(obj);
	    break;
	case ',': {
	    int atlist = LispGet(mac);

	    if (atlist == EOF)
		goto endofinput;
	    else if (atlist != '@')
		LispUnget(mac);
	    if ((obj = LispRun(mac)) == NULL || obj == EOLIST)
		LispDestroy(mac, QUOTMSG);
	    res = COMMA(obj, atlist == '@');
	}   break;
	case '#':
	    hash = 1;
	    ch = LispGet(mac);
	    if (ch == EOF)
		goto endofinput;
	    else if (ch != '\\')
		LispDestroy(mac, "syntax error at #");
	    ch = '#';
	    goto string_label;
	case '"':
	    dquote = 1;
	    escape = 1;
	    goto string_label;
	default:
string_label:
	    len = 0;
	    size = sizeof(stk);
	    str = stk;
	    while (ch != EOF && ((dquote || (hash && len < 2)) ||
		   (!isspace(ch) && (ch != ')' && ch != '(' && ch != ';')))) {
		if (len >= size - 1) {
		    char *tmp;

		    if (str == stk)
			tmp = (char*)LispMalloc(mac, size + 1024);
		    else
			tmp = (char*)LispRealloc(mac, str, size + 1024);
		    str = tmp;
		    size += 1024;
		}

		if (ch == '\\')
		    escape = !escape;

		if (dquote) {
		    if (!escape) {
			if (ch == '"')
			    break;
			str[len++] = ch;
		    }
		}
		else if (hash) {
		    if (!escape)
			str[len++] = ch;
		}
		else if (!escape)
		    str[len++] = toupper(ch);
		ch = LispGet(mac);
		if (escape)
		    escape = 0;
	    }
	    str[len] = '\0';
	    if (ch == '(' || ch == ')' || ch == ';')
		LispUnget(mac);
	    if (dquote)
		res = STRING(str);
	    else if (isdigit(str[0]) ||
		     ((str[0] == '-' || str[0] == '.' || str[0] == '+') &&
		      isdigit(str[1]))) {
		double value;
		char *cp;

		value = strtod(str, &cp);
		if (cp && *cp)
		    res = ATOM(str);
		else
		    res = REAL(value);
	    }
	    else if (hash) {
		long c = 0;

		if (len == 1)
		    LispDestroy(mac, "syntax error at #");
		else if (len > 2) {
		    if (len == 6 && (str[1] == 'u' || str[1] == 'U')) {
			char *end;

			c = strtoul(str + 2, &end, 16);
			if (!*end)
			    goto gotchar;
		    }
		    for (c = 0; c <= ' '; c++)
			if (strcasecmp(LispCharNames[c], str + 1) == 0)
			    break;
		    if (c > ' ') {
			/* extra or special cases */
			if (strcasecmp(str + 1, "Rubout") == 0)
			    c = 0177;
			else if (strcasecmp(str + 1, "Nul") == 0)
			    c = 0;
			else if (strcasecmp(str + 1, "Bs") == 0)
			    c = 010;
			else if (strcasecmp(str + 1, "Ht") == 0)
			    c = 011;
			else if (strcasecmp(str + 1, "Lf") == 0)
			    c = 012;
			else if (strcasecmp(str + 1, "Ff") == 0)
			    c = 014;
			else if (strcasecmp(str + 1, "Cr") == 0)
			    c = 015;
			else if (strcasecmp(str + 1, "Esc") == 0)
			    c = 033;
			else if (strcasecmp(str + 1, "Del") == 0)
			    c = 0177;
			else if (strcasecmp(str + 1, "Linefeed") == 0)
			    c = 012;
			else if (strcasecmp(str + 1, "Delete") == 0)
			    c = 0177;
			else
			    LispDestroy(mac, "no character named \"%s\"",
					str + 1);
		    }
		}
		else
		    c = *(unsigned char*)(str + 1);
gotchar:
		res = CHAR(c);
	    }
	    else {
		if (!len || strcmp(str, "NIL") == 0)
		    res = NIL;
		else if (strcmp(str, "T") == 0)
		    res = T;
		else if (strcmp(str, ".") == 0)
		    res = DOT;
		else
		    res = ATOM(str);
	    }
	    if (str != stk)
		LispFree(mac, str);
	    break;
    }

    if (code == NIL)
	COD = res;
    FRM = frm;

    return (res);

endofinput:
    LispDestroy(mac, "unexpected end of input");
    /*NOTREACHED*/

    return (NIL);
}

LispObj *
LispEvalBackquote(LispMac *mac, LispObj *arg)
{
    LispObj *res = NIL, *frm = FRM;

    if (arg->type == LispComma_t) {
	if (arg->data.comma.atlist)
	    LispDestroy(mac, ",@ only allowed on lists");
	else if (arg->data.comma.eval->type == LispComma_t)
	    res = arg->data.comma.eval;
	else
	    /* just evaluate it */
	    res = EVAL(arg->data.comma.eval);
    }
    else if (arg->type == LispCons_t) {
	LispObj *obj, *cdr = NIL, *ptr;
	/* create new form, evaluating any commas inside */

	res = NIL;
	for (ptr = arg; ; ptr = CDR(ptr)) {
	    int atcons = 1, atlist = 0;

	    if (ptr->type != LispCons_t) {
		atcons = 0;
		obj = ptr;
	    }
	    else
		obj = CAR(ptr);
	    if (obj->type == LispComma_t) {
		atlist = obj->data.comma.atlist;
		if (obj->data.comma.eval->type == LispComma_t) {
		    if (atlist)
			LispDestroy(mac, ",@ only allowed on lists");
		    obj = obj->data.comma.eval;
		}
		else
		    obj = EVAL(obj->data.comma.eval);
	    }
	    else if (obj->type == LispBackquote_t)
		obj = LispEvalBackquote(mac, obj->data.quote);
	    else if (obj->type == LispCons_t)
		obj = LispEvalBackquote(mac, obj);
	    /* else do nothing */

	    if (res == NIL) {
		if (frm != FRM)
		    /* just free a cons, in case of code like:
		     *  (setq c nil d '(1 2 3))
		     * `(,@c ,@d)
		     */
		    FRM = frm;

		/* link to FRM to protect from gc,
		 * actually, should protect with COD, but this would
		 * require EVAL to also save/restore COD, causing
		 * a bit slower EVAL time.
		 */
		GCProtect();
		if (!atlist) {
		    if (atcons)
			/* easier case */
			res = cdr = CONS(obj, NIL);
		    else
			res = cdr = obj;
		}
		else {
		    /* add list contents */
		    if (obj->type != LispCons_t)
			res = cdr = obj;
		    else {
			res = cdr = CONS(CAR(obj), NIL);
			for (obj = CDR(obj); obj->type == LispCons_t;
			     obj = CDR(obj)) {
			    CDR(cdr) = CONS(CAR(obj), NIL);
			    cdr = CDR(cdr);
			}
			if (obj != NIL) {
			    CDR(cdr) = obj;
			    cdr = CDR(cdr);
			}
		    }
		}
		FRM = CONS(res, FRM);
		GCUProtect();
	    }
	    else {
		if (cdr->type != LispCons_t)
		    LispDestroy(mac, "cannot append to %s",
				LispStrObj(mac, cdr));
		if (!atlist) {
		    if (atcons) {
			CDR(cdr) = CONS(obj, NIL);
			cdr = CDR(cdr);
		    }
		    else {
			CDR(cdr) = obj;
			cdr = obj;
		    }
		}
		else {
		    if (obj->type != LispCons_t) {
			CDR(cdr) = obj;
			if (obj != NIL)
			    cdr = obj;
		    }
		    else {
			for (; obj->type == LispCons_t; obj = CDR(obj)) {
			    CDR(cdr) = CONS(CAR(obj), NIL);
			    cdr = CDR(cdr);
			}
			if (obj != NIL) {
			    CDR(cdr) = obj;
			    cdr = CDR(cdr);
			}
		    }
		}
	    }
	    if (ptr->type != LispCons_t)
		break;
	}
    }
    else if (arg->type == LispBackquote_t)
	res = BACKQUOTE(LispEvalBackquote(mac, arg->data.quote));
    else
	/* 'obj == `obj */
	res = arg;

    return (res);
}

static LispObj *
LispBuildArguments(LispMac *mac, LispObj *desc, LispObj *values,
		   char *fname, int macro)
{
    int rest, optional, key;
    LispObj *args = NIL, *list = desc, *res, *cdr = NIL, *arg, *keyword = NIL;

    /* build argument list */
    while (list != NIL) {
	if (STRPTR(CAR(list))[0] != '&') {
	    if (args == NIL) {
		GCProtect();
		args = cdr = CONS(list, NIL);
		FRM = CONS(args, FRM);
		GCUProtect();
	    }
	    else {
		CDR(cdr) = CONS(list, NIL);
		cdr = CDR(cdr);
	    }
	}
	list = CDR(list);
    }

    /* fill argument list */
    list = desc;
    rest = optional = key = 0;
    cdr = values;
    arg = args;
    while (list != NIL) {
	if (STRPTR(CAR(list))[0] == '&') {
	    if (strcmp(STRPTR(CAR(list)) + 1, "KEY") == 0)
		key = 1;
	    else if (strcmp(STRPTR(CAR(list)) + 1, "REST") == 0) {
		rest = 1;
		break;
	    }
	    else /* "OPTIONAL" */
		optional = 1;
	}
	else {
	    if (cdr == NIL) {
		if (key || optional)
		    break;
		else
		    LispDestroy(mac, "too few arguments to %s", fname);
	    }
	    else {
		if (key) {
		    if (!SYMBOL_P(CAR(cdr)) || STRPTR(CAR(cdr))[0] != ':')
			LispDestroy(mac, "&KEY needs arguments as pairs, at %s",
				    fname);
		    else if (CDR(cdr) == NIL)
			LispDestroy(mac, "expecting %s value, at %s",
				    STRPTR(CAR(cdr)), fname);
		    keyword = CAR(cdr);
		    cdr = CDR(cdr);
		}
		if (macro)
		    res = CAR(cdr);
		else
		    res = EVAL(CAR(cdr));
		if (key) {
		    LispObj *atmp, *cmp, *ltmp = desc;

		    for (; ltmp != NIL; ltmp = CDR(ltmp)) {
			if ((cmp = CAR(ltmp))->type == LispCons_t)
			    cmp = CAR(cmp);
			else if (STRPTR(cmp)[0] == '&')
			    continue;
			if (strcmp(STRPTR(cmp), STRPTR(keyword) + 1) == 0)
			    break;
		    }
		    if (ltmp == NIL)
			LispDestroy(mac, "%s is not an argument to %s",
				    STRPTR(keyword), fname);
		    for (atmp = args; atmp != NIL; atmp = CDR(atmp))
			if (CAR(atmp) == ltmp) {
			    CAR(atmp) = res;
			    break;
			}
			/* else, silently ignore setting argument
			 * more than once? */
		}
		else
		    CAR(arg) = res;
		cdr = CDR(cdr);
	    }
	    arg = CDR(arg);
	}
	list = CDR(list);
    }
    if (rest) {
	res = CAR(cdr);
	if (!macro)
	    res = EVAL(res);
	CAR(arg) = CONS(res, NIL);
	arg = CAR(arg);
	cdr = CDR(cdr);
	while (cdr->type == LispCons_t) {
	    res = CAR(cdr);
	    if (!macro)
		res = EVAL(res);
	    CDR(arg) = CONS(res, NIL);
	    arg = CDR(arg);
	    cdr = CDR(cdr);
	}
    }
    else if (cdr != NIL)
	LispDestroy(mac, "too many arguments to %s", fname);

    /* set to NIL or default any unspecified arguments */
    if (key || optional) {
	arg = args;
	list = desc;

	for (; list != NIL; list = CDR(list))
	    if (STRPTR(CAR(list))[0] != '&') {
		if (CAR(arg) == list) {
		    if (CAR(list)->type == LispCons_t &&
			CDR(CAR(list))->type == LispCons_t) {
			if (macro)
			    CAR(arg) = CAR(CDR(CAR(list)));
			else
			    CAR(arg) = EVAL(CAR(CDR(CAR(list))));
		    }
		    else
			CAR(arg) = NIL;
		}
		arg = CDR(arg);
	    }
    }

    return (args);
}

/* if max_args == 0, then any number of arguments accepted,
 * if min_args < 0 then, if max_args == 0, no args are accepted */
static LispObj *
LispBuildSimpleArguments(LispMac *mac, LispObj *values, char *fname,
			 int min_args, int max_args, int macro)
{
    int num_objs = 0;
    LispObj *res, *args = NIL, *cdr = values;

    while (cdr->type == LispCons_t) {
	++num_objs;
	cdr = CDR(cdr);
    }
    if (num_objs < min_args)
	LispDestroy(mac, "too few arguments to %s", fname);
    else if ((max_args || min_args < 0) && num_objs > max_args)
	LispDestroy(mac, "too many arguments to %s", fname);

    if (macro)
	args = values;
    else if (num_objs) {
	LispObj *arg = values;

	res = EVAL(CAR(arg));
	GCProtect();
	args = cdr = CONS(res, NIL);
	FRM = CONS(args, FRM);
	GCUProtect();
	arg = CDR(arg);
	while (arg->type == LispCons_t) {
	    CDR(cdr) = CONS(EVAL(CAR(arg)), NIL);
	    cdr = CDR(cdr);
	    arg = CDR(arg);
	}
    }

    return (args);
}

LispObj *
LispEval(LispMac *mac, LispObj *obj)
{
    char *strname = NULL;
    LispObj *name = NIL, *fun, *cons = NIL, *frm, *res, *car;
    LispBuiltin *fn;

    if (!obj)
	LispDestroy(mac, "internal error, at INTERNAL:EVAL");

    switch (obj->type) {
	case LispAtom_t:
	    strname = STRPTR(obj);
	    if (mac->debugging)
		LispDebugger(mac, LispDebugCallBegini, NIL, obj);
	    if (strname[0] != ':' && (obj = LispGetVar(mac, obj)) == NULL)
		LispDestroy(mac, "the variable %s is unbound", strname);
	    if (mac->debugging)
		LispDebugger(mac, LispDebugCallEndi, NIL, obj);
	    return (obj);
	case LispQuote_t:
	    if (mac->debugging) {
		LispDebugger(mac, LispDebugCallBegini, NIL, obj);
		LispDebugger(mac, LispDebugCallEndi, NIL, obj->data.quote);
	    }
	    return (obj->data.quote);
	case LispBackquote_t:
	    return LispEvalBackquote(mac, obj->data.quote);
	case LispComma_t:
	    LispDestroy(mac, "illegal comma outside of backquote");
	case LispCons_t:
	    cons = obj;
	    break;
	case LispNil_t:
	case LispTrue_t:
	case LispReal_t:
	case LispString_t:
	case LispOpaque_t:
	default:
	    /* don't {step,next}i on literals */
	    return (obj);
    }
    car = CAR(cons);
    fun = NIL;
    switch (car->type) {
	case LispAtom_t:
	    name = car;
	    strname = STRPTR(name);
	    break;
	case LispLambda_t:
	    name = NIL;
	    strname = "NIL";
	    fun = car;
	    break;
	default:
	    LispDestroy(mac, "%s is invalid as a function, at INTERNAL:EVAL",
			LispStrObj(mac, car));
	    /*NOTREACHED*/
    }

    ++mac->level;
    frm = FRM;

    if (mac->debugging)
	LispDebugger(mac, LispDebugCallBegin, name, CDR(cons));

    if (fun == NIL && name->data.atom->property) {
	LispAtom *atom = name->data.atom;

	if (atom->property->builtin) {
	    LispObj *args;

	    fn = atom->property->fun.builtin;
	    args = LispBuildSimpleArguments(mac, CDR(cons), strname,
					    fn->min_args, fn->max_args,
					    !fn->eval);

	    res = fn->fn(mac, args, strname);
	    if (mac->debugging)
		LispDebugger(mac, LispDebugCallEnd, name, res);
	    FRM = frm;
	    --mac->level;

	    return (res);
	}

	if (atom->property->function)
	    fun = name->data.atom->property->fun.function;
	else if (atom->property->defstruct) {
	    LispObj *args;
	    int min_args, max_args;

	    /* Expands call to xedit::struct-* functions.
	     * Besides this is a implementation dependent trick,
	     * I believe it is better than another implementation I wrote,
	     * were I added a LispBuiltin for every access function, and
	     * also a defsetf. This new way is hacky, but uses plenty less
	     * memory, and is probably faster.
	     * Maybe these atoms should be store in LispMac, for faster
	     * access.
	     */

	    if (atom->property->structure.function == STRUCT_CONSTRUCTOR)
		atom = ATOM("XEDIT::MAKE-STRUCT")->data.atom;
	    else if (atom->property->structure.function == STRUCT_CHECK)
		atom = ATOM("XEDIT::STRUCT-TYPE")->data.atom;
	    else
		atom = ATOM("XEDIT::STRUCT-ACCESS")->data.atom;

	    fn = atom->property->fun.builtin;
	    min_args = fn->min_args > 0 ? fn->min_args - 1 : fn->min_args;
	    max_args = fn->max_args > 0 ? fn->max_args - 1 : fn->max_args;
	    args = LispBuildSimpleArguments(mac, CDR(cons), strname,
					    min_args, max_args, !fn->eval);

	    GCProtect();
	    args = CONS(name, args);
	    FRM = CONS(args, FRM);
	    GCUProtect();

	    res = fn->fn(mac, args, strname);
	    if (mac->debugging)
		LispDebugger(mac, LispDebugCallEnd, name, res);
	    FRM = frm;
	    --mac->level;

	    return (res);
	}
    }

    if (fun != NIL) {
	LispObj *args = NIL;

	if (!fun->data.lambda.key && !fun->data.lambda.optional &&
	    !fun->data.lambda.rest)
	    args = LispBuildSimpleArguments(mac, CDR(cons), strname,
					    -1, fun->data.lambda.num_args,
					    fun->data.lambda.type == LispMacro);
	else
	    args = LispBuildArguments(mac, CAR(fun->data.lambda.code),
				      CDR(cons), strname,
				      fun->data.lambda.type == LispMacro);

	res = LispRunFunMac(mac, fun, args);
	if (mac->debugging)
	    LispDebugger(mac, LispDebugCallEnd, fun->data.lambda.name, res);
	FRM = frm;
	--mac->level;

	return (res);
    }

    LispDestroy(mac, "the function %s is not defined", strname);
    /*NOTREACHED*/

    return (NIL);
}

LispObj *
LispRunFunMac(LispMac *mac, LispObj *fun, LispObj *list)
{
    volatile LispFunType type = fun->data.lambda.type;
    LispObj *old_env, *old_lex, *args, *code, *res, *frm;

    old_env = ENV;
    old_lex = LEX;

    args = CAR(fun->data.lambda.code);
    code = CDR(fun->data.lambda.code);

    LEX = ENV;

    for (; args != NIL; args = CDR(args)) {
	if (CAR(args)->type == LispCons_t) {
	    LispAddVar(mac, CAR(CAR(args)), CAR(list));
	    list = CDR(list);
	}
	else if (STRPTR(CAR(args))[0] != '&') {
	    LispAddVar(mac, CAR(args), CAR(list));
	    list = CDR(list);
	}
    }

    if (type != LispMacro) {
	int did_jump = 1, *pdid_jump = &did_jump;
	LispObj **pres = &res;
	LispBlock *block =
	    LispBeginBlock(mac, fun->data.lambda.name, LispBlockClosure);
	char *strname = type == LispLambda ?
	    "#<LAMBDA>" : STRPTR(fun->data.lambda.name), **pstrname = &strname;

	*pres = NIL;
	if (setjmp(block->jmp) == 0) {
	    *pres = Lisp_Progn(mac, code, *pstrname);
	    *pdid_jump = 0;
	}
	LispEndBlock(mac, block);
	if (*pdid_jump)
	    *pres = mac->block.block_ret;
    }
    else
	res = Lisp_Progn(mac, code, STRPTR(fun->data.lambda.name));

    LEX = old_lex;
    ENV = old_env;

    /* res is not gc protected, link to FRM */
    if (type == LispMacro) {
	frm = FRM;
	FRM = CONS(res, FRM);
	res = EVAL(res);
	FRM = frm;
    }

    return (res);
}

LispObj *
LispRunSetf(LispMac *mac, LispObj *setf, LispObj *place, LispObj *value)
{
    static char *fname = "INTERNAL:EXPAND-SETF";
    LispObj *old_env, *old_lex, *args, *desc, *sto, *code, *exp, *res;

    desc = CAAR(setf->data.lambda.code);

    if (setf->data.lambda.key || setf->data.lambda.optional ||
	setf->data.lambda.rest)
	args = LispBuildArguments(mac, desc, CDR(place), fname, 1);
    else
	args = LispBuildSimpleArguments(mac, CDR(place), fname, -1,
					setf->data.lambda.num_args, 1);

    /* if no errors so far, evaluate setf parameter before setting
     * setting new environment */
    value = QUOTE(EVAL(value));

    old_env = ENV;
    old_lex = LEX;
    sto = CDAR(setf->data.lambda.code);
    code = CDR(setf->data.lambda.code);
    LEX = ENV;

    /* create new environment */

    /* bind store value first, so gc does not need to be disabled.
     * Currently, only first store argument used */
    LispAddVar(mac, CAR(sto), value);

    for (; desc != NIL; desc = CDR(desc)) {
	if (CAR(desc)->type == LispCons_t) {
	    LispAddVar(mac, CAR(CAR(desc)), CAR(args));
	    args = CDR(args);
	}
	else if (STRPTR(CAR(desc))[0] != '&') {
	    LispAddVar(mac, CAR(desc), CAR(args));
	    args = CDR(args);
	}
    }

    /* build expansion macro */
    exp = Lisp_Progn(mac, code, STRPTR(setf->data.lambda.name));

    /* restore enviroment */
    LEX = old_lex;
    ENV = old_env;

    /* protect expansion, and executes it */
    GCProtect();
    old_env = FRM;
    FRM = CONS(exp, FRM);
    GCUProtect();
    res = EVAL(exp);
    FRM = old_env;

    return (res);
}

void
LispSnprintObj(LispMac *mac, LispObj *obj, char **str, int *len, int paren)
{
    int sz;

    if (*len < 1)
	return;
    switch (obj->type) {
	case LispNil_t:
	    sz = snprintf(*str, *len, "NIL");
	    *len -= sz;
	    *str += sz;
	    break;
	case LispTrue_t:
	    sz = snprintf(*str, *len, "T");
	    *len -= sz;
	    *str += sz;
	    break;
	case LispOpaque_t:
	    sz = snprintf(*str, *len, "#0x%08x-%s", (int)obj->data.opaque.data,
			  LispIntToOpaqueType(mac, obj->data.opaque.type));
	    *len -= sz;
	    *str += sz;
	    break;
	case LispAtom_t:
	    sz = snprintf(*str, *len, "%s", STRPTR(obj));
	    *len -= sz;
	    *str += sz;
	    break;
	case LispString_t:
	    sz = snprintf(*str, *len, "\"%s\"", STRPTR(obj));
	    *len -= sz;
	    *str += sz;
	    break;
	case LispCharacter_t:
	    sz = snprintf(*str, *len, "#\\");
	    if ((*len -= sz) <= 0)
		return;
	    *str += sz;
	    if (obj->data.integer >= 0 && obj->data.integer <= ' ')
		sz = snprintf(*str, *len, "%s",
			      LispCharNames[obj->data.integer]);
	    else if (obj->data.integer == 0177)
		sz = snprintf(*str, *len, "Rubout");
	    else if (obj->data.integer > 0xff)
		sz = snprintf(*str, *len, "U%04X", (int)obj->data.integer);
	    else
		sz = snprintf(*str, *len, "%c", (int)obj->data.integer);
	    *str += sz;
	    *len -= sz;
	    break;
	case LispReal_t:
	    sz = snprintf(*str, *len, "%g", obj->data.real);
	    *len -= sz;
	    *str += sz;
	    break;
	case LispInteger_t:
	    sz = snprintf(*str, *len, "%ld", obj->data.integer);
	    *len -= sz;
	    *str += sz;
	    break;
	case LispCons_t: {
	    LispObj *car, *cdr;

	    car = CAR(obj);
	    cdr = CDR(obj);
	    if (!cdr || cdr->type == LispNil_t) {
		if (paren) {
		    sz = snprintf(*str, *len, "(");
		    if ((*len -= sz) <= 0)
			return;
		    *str += sz;
		}
		LispSnprintObj(mac, car, str, len, car->type == LispCons_t);
		if (*len <= 0)
		    return;
		if (paren) {
		    sz = snprintf(*str, *len, ")");
		    if ((*len -= sz) <= 0)
			return;
		    *str += sz;
		}
	    }
	    else {
		if (paren) {
		    sz = snprintf(*str, *len, "(");
		    if ((*len -= sz) <= 0)
			return;
		    *str += sz;
		}
		LispSnprintObj(mac, car, str, len, car->type == LispCons_t);
		if (*len <= 0)
		    return;
		if (cdr->type != LispCons_t) {
		    sz = snprintf(*str, *len, " . ");
		    if ((*len -= sz) <= 0)
			return;
		    *str += sz;
		    LispSnprintObj(mac, cdr, str, len, 0);
		}
		else {
		    sz = snprintf(*str, *len, " ");
		    if ((*len -= sz) <= 0)
			return;
		    *str += sz;
		    LispSnprintObj(mac, cdr, str, len, car->type != LispCons_t &&
				   cdr->type != LispCons_t);
		    if (*len <= 0)
			return;
		}
		if (paren) {
		    sz = snprintf(*str, *len, ")");
		    *len -= sz;
		    *str += sz;
		}
	    }
	}    break;
	case LispQuote_t:
	    sz = snprintf(*str, *len, "'");
	    *len -= sz;
	    *str += sz;
	    LispSnprintObj(mac, obj->data.quote, str, len, 1);
	    break;
	case LispBackquote_t:
	    sz = snprintf(*str, *len, "`");
	    *len -= sz;
	    *str += sz;
	    LispSnprintObj(mac, obj->data.quote, str, len, 1);
	    break;
	case LispComma_t:
	    if (obj->data.comma.atlist)
		sz = snprintf(*str, *len, ",@");
	    else
		sz = snprintf(*str, *len, ",");
	    *len -= sz;
	    *str += sz;
	    LispSnprintObj(mac, obj->data.comma.eval, str, len, 1);
	    break;
	case LispArray_t:
	    if (obj->data.array.rank == 1)
		sz = snprintf(*str, *len, "#(");
	    else
		sz = snprintf(*str, *len, "#%dA(", obj->data.array.rank);
	    if ((*len -= sz) <= 0)
		return;
	    *str += sz;
	    if (!obj->data.array.zero) {
		if (obj->data.array.rank == 1) {
		    LispObj *ary;
		    long count;

		    for (ary = obj->data.array.dim, count = 1;
			 ary != NIL; ary = CDR(ary))
			count *= (int)NUMBER_VALUE(CAR(ary));
		    for (ary = obj->data.array.list; count > 0;
			ary = CDR(ary), count--) {
			LispSnprintObj(mac, CAR(ary), str, len, 0);
			if (*len <= 0)
			    return;
			if (count - 1 > 0) {
			    sz = snprintf(*str, *len, " ");
			    if ((*len -= sz) <= 0)
				return;
			    *str += sz;
			}
		    }
		}
		else {
		    LispObj *ary;
		    int i, k, rank, *dims, *loop;

		    rank = obj->data.array.rank;
		    dims = LispMalloc(mac, sizeof(int) * rank);
		    loop = LispCalloc(mac, 1, sizeof(int) * (rank - 1));

		    /* fill dim */
		    for (i = 0, ary = obj->data.array.dim; ary != NIL;
			 i++, ary = CDR(ary))
			dims[i] = (int)NUMBER_VALUE(CAR(ary));

		    i = 0;
		    ary = obj->data.array.list;
		    while (loop[0] < dims[0]) {
			for (; i < rank - 1; i++) {
			    sz = snprintf(*str, *len, "(");
			    if ((*len -= sz) <= 0)
				goto snprint_array_done;
			    *str += sz;
			}
			--i;
			for (;;) {
			    ++loop[i];
			    if (i && loop[i] >= dims[i])
				loop[i] = 0;
			    else
				break;
			    --i;
			}
			for (k = 0; k < dims[rank - 1] - 1; k++, ary = CDR(ary)) {
			    LispSnprintObj(mac, CAR(ary), str, len, 0);
			    if (*len <= 0)
				goto snprint_array_done;
			    sz = snprintf(*str, *len, " ");
			    if ((*len -= sz) <= 0)
				goto snprint_array_done;
			    *str += sz;
			}
			LispSnprintObj(mac, CAR(ary), str, len, 1);
			if (*len <= 0)
			    goto snprint_array_done;
			ary = CDR(ary);
			for (k = rank - 1; k > i; k--) {
			    sz = snprintf(*str, *len, ")");
			    if ((*len -= sz) <= 0)
				goto snprint_array_done;
			    *str += sz;
			}
			if (loop[0] < dims[0]) {
			    sz = snprintf(*str, *len, " ");
			    if ((*len -= sz) <= 0)
				goto snprint_array_done;
			    *str += sz;
			}
		    }

snprint_array_done:
		    LispFree(mac, dims);
		    LispFree(mac, loop);
		}
	    }
	    sz = snprintf(*str, *len, ")");
	    *len -= sz;
	    *str += sz;
	    break;
	case LispStruct_t: {
	    LispObj *def = obj->data.struc.def;
	    LispObj *field = obj->data.struc.fields;

	    sz = snprintf(*str, *len, "S#(%s", STRPTR(CAR(def)));
	    if ((*len -= sz) <= 0)
		return;
	    *str += sz;
	    def = CDR(def);
	    for (; def != NIL; def = CDR(def), field = CDR(field)) {
		sz = snprintf(*str, *len, " :%s ", SYMBOL_P(CAR(def)) ?
			      STRPTR(CAR(def)) : STRPTR(CAR(CAR(def))));
		if ((*len -= sz) <= 0)
		    return;
		*str += sz;
		LispSnprintObj(mac, CAR(field), str, len, 1);
		if (*len <= 0)
		    return;
	    }
	    sz = snprintf(*str, *len, ")");
	    *len -= sz;
	    *str += sz;
	}   break;
	default:
	    break;
    }
}

char *
LispStrObj(LispMac *mac, LispObj *obj)
{
    static char string[12];

    LispSnprint(mac, obj, string, sizeof(string) - 1);
    return (string);
}

void
LispSnprint(LispMac *mac, LispObj *obj, char *str, int len)
{
    char *s = str;
    int l = len;

    if (!obj || !str || len <= 0)
	LispDestroy(mac, "internal error, at INTERNAL:SPRINT");
    LispSnprintObj(mac, obj, &str, &len, 1);
    if (len <= 0) {
    /* this is a internal function, so I assume that str has enough space */
	if (*s == '(')
	    strcpy(s + l - 5, "...)");
	else
	    strcpy(s + l - 4, "...");
    }
}

/* assumes string is writable, escapes " as \" and \ as \\ */
int
LispPrintString(LispMac *mac, LispObj *stream, char *str)
{
    int len, ch;
    char *prt, *ptr, *pquote, *pslash;

    if (!mac->princ) {
	len = LispPrintf(mac, stream, "%c", '"');
	for (prt = str, pquote = strchr(prt, '"'), pslash = strchr(prt, '\\');
	     pquote || pslash;
	     prt = ptr, pquote = pquote ? strchr(prt, '"') : NULL,
			pslash = pslash ? strchr(prt, '\\') : NULL) {
	    if (pquote && pslash)
		ptr = pquote < pslash ? pquote : pslash;
	    else
		ptr = pquote ? pquote : pslash;
	    ch = ptr == pquote ? '"' : '\\';
	    *ptr = '\0';
	    len += LispPrintf(mac, stream, "%s", prt);
	    len += LispPrintf(mac, stream, "%c%c", '\\', ch);
	    *ptr = ch;
	    ++ptr;
	}
	len += LispPrintf(mac, stream, "%s", prt);
	len += LispPrintf(mac, stream, "%c", '"');
    }
    else
	len = LispPrintf(mac, stream, "%s", str);

    return (len);
}

int
LispPrintf(LispMac *mac, LispObj *stream, char *fmt, ...)
{
    int size;
    va_list ap;
    FILE *fp = NULL;

    if (stream == NIL)
	fp = lisp_stdout;
    else if (stream->data.stream.size < 0)
	fp = stream->data.stream.source.fp;

    va_start(ap, fmt);
    if (fp && !mac->justsize)
	size = vfprintf(fp, fmt, ap);
    else {
	int n;
	unsigned char stk[1024], *ptr = stk;

	size = sizeof(stk);
	n = vsnprintf((char*)stk, size, fmt, ap);
	if (n < 0 || n >= size) {
	    while (1) {
		char *tmp;

		va_end(ap);
		if (n > size)
		    size = n + 1;
		else
		    size *= 2;
		if ((tmp = realloc(ptr, size)) == NULL) {
		    free(ptr);
		    LispDestroy(mac, "out of memory");
		}
		va_start(ap, fmt);
		n = vsnprintf((char*)ptr, size, fmt, ap);
		if (n >= 0 && n < size)
		    break;
	    }
	}
	size = strlen((char*)ptr);

	if (!mac->justsize) {
	    while (stream->data.stream.idx + size >= stream->data.stream.size) {
		unsigned char *tmp = realloc(stream->data.stream.source.str,
					     stream->data.stream.size + pagesize);

		if (tmp == NULL) {
		    if (ptr != stk)
			free(ptr);
		    LispDestroy(mac, "out of memory");
		}
		stream->data.stream.source.str = tmp;
		stream->data.stream.size += pagesize;
	    }
	    strcpy((char*)stream->data.stream.source.str +
		   stream->data.stream.idx, (char*)ptr);
	    stream->data.stream.idx += size;
	}
	if (ptr != stk)
	    free(ptr);
    }
    va_end(ap);

    return (size);
}

int
LispPrintObj(LispMac *mac, LispObj *stream, LispObj *obj, int paren)
{
    int len = 0;

    switch (obj->type) {
	case LispNil_t:
	    len += LispPrintf(mac, stream, "NIL");
	    break;
	case LispTrue_t:
	    len += LispPrintf(mac, stream, "T");
	    break;
	case LispOpaque_t:
	    len += LispPrintf(mac, stream, "#0x%08x-%s",
			      (int)obj->data.opaque.data,
			      LispIntToOpaqueType(mac, obj->data.opaque.type));
	    break;
	case LispAtom_t:
	    len += LispPrintf(mac, stream, "%s", STRPTR(obj));
	    break;
	case LispString_t:
	    len += LispPrintString(mac, stream, STRPTR(obj));
	    break;
	case LispCharacter_t:
	    if (!mac->princ)
		len += LispPrintf(mac, stream, "#\\");
	    if (obj->data.integer >= 0 && obj->data.integer <= ' ')
		len += LispPrintf(mac, stream, "%s",
				  LispCharNames[obj->data.integer]);
	    else if (obj->data.integer == 0177)
		len += LispPrintf(mac, stream, "Rubout");
	    else if (obj->data.integer > 0xff)
		len += LispPrintf(mac, stream, "U%04X", (int)obj->data.integer);
	    else
		len += LispPrintf(mac, stream, "%c", obj->data.integer);
	    break;
	case LispReal_t:
	    len += LispPrintf(mac, stream, "%g", obj->data.real);
	    break;
	case LispInteger_t:
	    len += LispPrintf(mac, stream, "%ld", obj->data.integer);
	    break;
	case LispCons_t: {
	    LispObj *car, *cdr;

	    car = CAR(obj);
	    cdr = CDR(obj);
	    if (!cdr || cdr->type == LispNil_t) {
		if (paren)
		    len += LispPrintf(mac, stream, "(");
		len += LispPrintObj(mac, stream, car, car->type == LispCons_t);
		if (paren)
		    len += LispPrintf(mac, stream, ")");
	    }
	    else {
		if (paren)
		    len += LispPrintf(mac, stream, "(");
		LispPrintObj(mac, stream, car, car->type == LispCons_t);
		if (cdr->type != LispCons_t) {
		    len += LispPrintf(mac, stream, " . ");
		    len += LispPrintObj(mac, stream, cdr, 0);
		}
		else {
		    len += LispPrintf(mac, stream, " ");
		    len += LispPrintObj(mac, stream, cdr,
					car->type != LispCons_t &&
					cdr->type != LispCons_t);
		}
		if (paren)
		    len += LispPrintf(mac, stream, ")");
	    }
	}    break;
	case LispQuote_t:
	    len += LispPrintf(mac, stream, "'");
	    len += LispPrintObj(mac, stream, obj->data.quote, 1);
	    break;
	case LispBackquote_t:
	    len += LispPrintf(mac, stream, "`");
	    len += LispPrintObj(mac, stream, obj->data.quote, 1);
	    break;
	case LispComma_t:
	    if (obj->data.comma.atlist)
		len += LispPrintf(mac, stream, ",@");
	    else
		len += LispPrintf(mac, stream, ",");
	    len += LispPrintObj(mac, stream, obj->data.comma.eval, 1);
	    break;
	case LispArray_t:
	    if (obj->data.array.rank == 1)
		len += LispPrintf(mac, stream, "#(");
	    else
		len += LispPrintf(mac, stream, "#%dA(", obj->data.array.rank);

	    if (!obj->data.array.zero) {
		if (obj->data.array.rank == 1) {
		    LispObj *ary;
		    long count;

		    for (ary = obj->data.array.dim, count = 1;
			 ary != NIL; ary = CDR(ary))
			count *= (int)NUMBER_VALUE(CAR(ary));
		    for (ary = obj->data.array.list; count > 0;
			 ary = CDR(ary), count--) {
			len += LispPrintObj(mac, stream, CAR(ary), 0);
			if (count - 1 > 0)
			    len += LispPrintf(mac, stream, " ");
		    }
		}
		else {
		    LispObj *ary;
		    int i, k, rank, *dims, *loop;

		    rank = obj->data.array.rank;
		    dims = LispMalloc(mac, sizeof(int) * rank);
		    loop = LispCalloc(mac, 1, sizeof(int) * (rank - 1));

		    /* fill dim */
		    for (i = 0, ary = obj->data.array.dim; ary != NIL;
			 i++, ary = CDR(ary))
			dims[i] = (int)NUMBER_VALUE(CAR(ary));

		    i = 0;
		    ary = obj->data.array.list;
		    while (loop[0] < dims[0]) {
			for (; i < rank - 1; i++)
			    len += LispPrintf(mac, stream, "(");
			--i;
			for (;;) {
			    ++loop[i];
			    if (i && loop[i] >= dims[i])
				loop[i] = 0;
			    else
				break;
			    --i;
			}
			for (k = 0; k < dims[rank - 1] - 1; k++, ary = CDR(ary)) {
			    len += LispPrintObj(mac, stream, CAR(ary), 1);
			    len += LispPrintf(mac, stream, " ");
			}
			len += LispPrintObj(mac, stream, CAR(ary), 0);
			ary = CDR(ary);
			for (k = rank - 1; k > i; k--)
			    len += LispPrintf(mac, stream, ")");
			if (loop[0] < dims[0])
			    len += LispPrintf(mac, stream, " ");
		    }
		    LispFree(mac, dims);
		    LispFree(mac, loop);
		}
	    }
	    len += LispPrintf(mac, stream, ")");
	    break;
	case LispStruct_t: {
	    LispObj *def = obj->data.struc.def;
	    LispObj *field = obj->data.struc.fields;

	    len += LispPrintf(mac, stream, "S#(%s", STRPTR(CAR(def)));
	    def = CDR(def);
	    for (; def != NIL; def = CDR(def), field = CDR(field)) {
		len += LispPrintf(mac, stream, " :%s ",
				  SYMBOL_P(CAR(def)) ?
				      STRPTR(CAR(def)) :
				      STRPTR(CAR(CAR(def))));
		len += LispPrintObj(mac, stream, CAR(field), 1);
	    }
	    len += LispPrintf(mac, stream, ")");
	}   break;
	case LispLambda_t:
	    switch (obj->data.lambda.type) {
		case LispLambda:
		    len += LispPrintf(mac, stream, "#<LAMBDA ");
		    break;
		case LispFunction:
		    len += LispPrintf(mac, stream, "#<FUNCTION %s ",
				      STRPTR(obj->data.lambda.name));
		    break;
		case LispMacro:
		    len += LispPrintf(mac, stream, "#<MACRO %s ",
				      STRPTR(obj->data.lambda.name));
		    break;
		case LispSetf:
		    len += LispPrintf(mac, stream, "#<SETF %s ",
				      STRPTR(obj->data.lambda.name));
		    break;
	    }
	    len += LispPrintObj(mac, stream, obj->data.lambda.code, 1);
	    len += LispPrintf(mac, stream, ">");
	    break;
	case LispStream_t:
	    if (obj->data.stream.size < 0)
		len += LispPrintf(mac, stream, "#<STREAM 0x%8x>",
				  (int)obj->data.stream.source.fp);
	    else
		len += LispPrintString(mac, stream,
				       obj->data.stream.source.str ?
				       (char*)obj->data.stream.source.str : "");
	    break;
    }

    return (len);
}

void
LispPrint(LispMac *mac, LispObj *obj, LispObj *stream, int newline)
{
    if (!obj || !stream)
	LispDestroy(mac, "internal error, at INTERNAL:PRINT");
    if (stream != NIL && stream->type != LispStream_t)
	LispDestroy(mac, "%s is not a stream", LispStrObj(mac, stream));
    if (newline && !mac->newline) {
	LispPrintf(mac, stream, "\n");
	mac->column = 0;
    }
    /* XXX maybe should check for newlines in object */
    mac->column = LispPrintObj(mac, stream, obj, 1);
    mac->newline = 0;
    fflush(lisp_stdout);
}

void
LispUpdateResults(LispMac *mac, LispObj *cod, LispObj *res)
{
    GCProtect();
    LispSetVar(mac, RUN[2], LispGetVar(mac, RUN[1]));
    LispSetVar(mac, RUN[1], LispGetVar(mac, RUN[0]));
    LispSetVar(mac, RUN[0], cod);

    LispSetVar(mac, RES[2], LispGetVar(mac, RES[1]));
    LispSetVar(mac, RES[1], LispGetVar(mac, RES[0]));
    LispSetVar(mac, RES[0], res);
    GCUProtect();
}

/* Needs a rewrite to either allow only one LispMac per process or some
 * smarter error handling */
static LispMac *global_mac = NULL;

#ifdef SIGNALRETURNSINT
int
LispAbortSignal(int signum)
{
    if (global_mac != NULL)
	LispDestroy(global_mac, "aborted");
}

int
LispFPESignal(int signum)
{
    if (global_mac != NULL)
	LispDestroy(global_mac, "Floating point exception");
}
#else
void
LispAbortSignal(int signum)
{
    if (global_mac != NULL)
	LispDestroy(global_mac, "aborted");
}

void
LispFPESignal(int signum)
{
    if (global_mac != NULL)
	LispDestroy(global_mac, "Floating point exception");
}
#endif

void
LispMachine(LispMac *mac)
{
    LispObj *cod, *obj;

    LispTopLevel(mac);
    /*CONSTCOND*/
    while (1) {
	mac->sigint = signal(SIGINT, LispAbortSignal);
	mac->sigfpe = signal(SIGFPE, LispFPESignal);
	global_mac = mac;
	if (sigsetjmp(mac->jmp, 1) == 0) {
	    mac->running = 1;
	    if (mac->interactive && mac->prompt) {
		fprintf(lisp_stdout, "%s", mac->prompt);
		fflush(lisp_stdout);
	    }
	    mac->level = 0;
	    if ((cod = LispRun(mac)) != NULL) {
		if (cod == EOLIST)
		    LispDestroy(mac, "object cannot start with #\\)");
		obj = EVAL(cod);
		if (mac->interactive) {
		    LispPrint(mac, obj, NIL, 1);
		    LispUpdateResults(mac, cod, obj);
		    if (!mac->newline) {
			LispPrintf(mac, NIL, "\n");
			mac->newline = 1;
			mac->column = 0;
		    }
		}
	    }
	    signal(SIGINT, mac->sigint);
	    signal(SIGFPE, mac->sigfpe);
	    global_mac = NULL;
	    LispTopLevel(mac);
	    if (mac->tok == EOF)
		break;
	    continue;
	}
	signal(SIGINT, mac->sigint);
	signal(SIGFPE, mac->sigfpe);
	global_mac = NULL;
    }
    mac->running = 0;
}

void
LispExecute(LispMac *mac, char *str)
{
    int level, running = mac->running;
    LispObj *obj;

    if (str == NULL || *str == '\0')
	return;

    if (mac->stream.stream_level + 1 >= mac->stream.stream_size) {
	LispStream *stream = (LispStream*)
	    realloc(mac->stream.stream, sizeof(LispStream) *
		    (mac->stream.stream_size + 1));

	if (stream == NULL) {
	    fprintf(lisp_stderr, "out of memory");
	    return;
	}

	mac->stream.stream = stream;
	++mac->stream.stream_size;
    }
    mac->stream.stream[mac->stream.stream_level].fp = mac->fp;
    mac->stream.stream[mac->stream.stream_level].st = mac->st;
    mac->stream.stream[mac->stream.stream_level].cp = mac->cp;
    mac->stream.stream[mac->stream.stream_level].tok = mac->tok;
    ++mac->stream.stream_level;
    memset(mac->stream.stream + mac->stream.stream_level, 0, sizeof(LispStream));
    mac->stream.stream[mac->stream.stream_level].fp = NULL;
    mac->fp = NULL;
    mac->st = mac->cp = LispStrdup(mac, str);
    mac->tok = 0;

    level = mac->level;
    mac->level = 0;

    if (running || sigsetjmp(mac->jmp, 1) == 0) {
	if (!running)
	    mac->running = 1;

	/*CONSTCOND*/
	while (1) {
	    if ((obj = LispRun(mac)) != NULL) {
		if (obj == EOLIST)
		    LispDestroy(mac, "object cannot start with #\\)");
		GCProtect();
		(void)EVAL(obj);
		GCUProtect();
	    }
	    if (mac->tok == EOF)
		break;
	}

	LispFree(mac, mac->st);
	mac->level = level;
	--mac->stream.stream_level;

	mac->fp = mac->stream.stream[mac->stream.stream_level].fp;
	mac->st = mac->stream.stream[mac->stream.stream_level].st;
	mac->cp = mac->stream.stream[mac->stream.stream_level].cp;
	mac->tok = mac->stream.stream[mac->stream.stream_level].tok;
    }
    mac->running = running;
}

LispMac *
LispBegin(int argc, char *argv[])
{
    int i;
    char results[4];
    char *fname = "INTERNAL:BEGIN";
    LispMac *mac = malloc(sizeof(LispMac));

    if (mac == NULL)
	return (NULL);

    if (lisp_stdin == NULL)
	lisp_stdin = fdopen(0, "r");
    if (lisp_stdout == NULL)
	lisp_stdout = fdopen(1, "w");
    if (lisp_stderr == NULL)
	lisp_stderr = fdopen(2, "w");

    pagesize = GetPageSize();
    segsize = pagesize / sizeof(LispObj);
    bzero((char*)mac, sizeof(LispMac));
    MOD = ENV = GLB = LEX = COD = FRM = DBG = BRK = PRO = NIL;
    LispAllocSeg(mac);

    /* initialize stream management */
    mac->stream.stream = (LispStream*)calloc(1, sizeof(LispStream));
    if (argc > 1) {
	i = 1;

	if (strcmp(argv[1], "-d") == 0) {
	    mac->debugging = 1;
	    mac->debug_level = -1;
	    ++i;
	}
	if (i < argc &&
	    (mac->stream.stream[0].fp = mac->fp = fopen(argv[i], "r")) == NULL) {
	    fprintf(lisp_stderr, "Cannot open %s.\n", argv[i]);
	    exit(1);
	}
    }
    if (mac->fp == NULL) {
	mac->stream.stream[0].fp = mac->fp = lisp_stdin;
	mac->interactive = 1;
    }
    else {
	int ch = LispGet(mac);

	if (ch != '#')
	    LispUnget(mac);
	else if (LispGet(mac) == '!') {
	    for (;;) {
		ch = LispGet(mac);
		if (ch == '\n' || ch == EOF)
		    break;
	    }
	}
	else {
	    LispUnget(mac);
	    LispUnget(mac);
	}
    }
    mac->stream.stream_size = 1;

    /* initialize memory management */
    mac->mem.mem = (void**)calloc(mac->mem.mem_size = 16, sizeof(void*));
    mac->mem.mem_level = 0;

    mac->prompt = "> ";
    mac->newline = 1;
    mac->column = 0;

    mac->errexit = !mac->interactive;

    if (mac->interactive) {
	/* add +, ++, +++, *, **, and *** */
	for (i = 0; i < 3; i++) {
	    results[i] = '+';
	    results[i + 1] = '\0';
	    RUN[i] = ATOM2(results);
	    _LispSet(mac, RUN[i], NIL, fname, 0);
	}
	for (i = 0; i < 3; i++) {
	    results[i] = '*';
	    results[i + 1] = '\0';
	    RES[i] = ATOM2(results);
	    _LispSet(mac, RES[i], NIL, fname, 0);
	}
    }
    else
	RUN[0] = RUN[1] = RUN[2] = RES[0] = RES[1] = RES[2] = NIL;

    for (i = 0; i < sizeof(lispbuiltins) / sizeof(lispbuiltins[0]); i++)
	LispAddBuiltinFunction(mac, &lispbuiltins[i]);

    return (mac);
}

void
LispEnd(LispMac *mac)
{
    if (mac->fp != lisp_stdin)
	fclose(mac->fp);
}

void
LispSetPrompt(LispMac *mac, char *prompt)
{
    mac->prompt = prompt;
}

void
LispSetInteractive(LispMac *mac, int interactive)
{
    mac->interactive = !!interactive;
}

void
LispSetExitOnError(LispMac *mac, int errexit)
{
    mac->errexit = !!errexit;
}

void
LispDebug(LispMac *mac, int enable)
{
    mac->debugging = !!enable;

    /* assumes we are at the toplevel */
    DBG = BRK = NIL;
    mac->debug_level = -1;
    mac->debug_step = 0;
}

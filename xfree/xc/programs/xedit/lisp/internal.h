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

/* $XFree86: xc/programs/xedit/lisp/internal.h,v 1.11 2001/10/20 00:19:34 paulo Exp $ */

#ifndef Lisp_internal_h
#define Lisp_internal_h

#include <stdio.h>
#include "lisp.h"

/*
 * Defines
 */
#define	CAR(list)		((list)->data.cons.car)
#define	CAAR(list)		((list)->data.cons.car->data.cons.car)
#define	CADR(list)		((list)->data.cons.cdr->data.cons.car)
#define CDR(list)		((list)->data.cons.cdr)
#define CDAR(list)		((list)->data.cons.car->data.cons.cdr)
#define CONS(car, cdr)		LispNewCons(mac, car, cdr)
#define EVAL(list)		LispEval(mac, list)
#define ATOM(atom)		LispNewAtom(mac, atom)
#define ATOM2(atom)		LispNewAtom(mac, LispGetString(mac, atom))
#define QUOTE(quote)		LispNewQuote(mac, quote)
#define BACKQUOTE(bquote)	LispNewBackquote(mac, bquote)
#define COMMA(comma, at)	LispNewComma(mac, comma, at)
#define REAL(num)		LispNewReal(mac, num)
#define STRING(str)		LispNewString(mac, str)
#define CHAR(c)			LispNewCharacter(mac, c)
#define INTEGER(i)		LispNewInteger(mac, i)
#define OPAQUE(data, type)	LispNewOpaque(mac, (void*)((long)data), type)
#define CHECKO(obj, typ)						\
	(obj)->type == LispOpaque_t && 					\
	 ((obj)->data.opaque.type == typ || (obj)->data.opaque.type == 0)
#define PROTECT(key, list)	LispProtect(mac, key, list)
#define UPROTECT(key, list)	LispUProtect(mac, key, list)

#define	GCProtect()		++gcpro
#define	GCUProtect()		--gcpro

#define	STRPTR(obj)		(obj)->data.atom->string

#define NUMBER_P(obj)			\
	((obj)->type == LispReal_t || (obj)->type == LispInteger_t)

/* assumes NUMBER_P is true */
#define NUMBER_VALUE(obj)		\
	(obj->type == LispReal_t ? obj->data.real : obj->data.integer)

#define INTEGER_P(obj)			\
	((obj)->type == LispInteger_t ||	\
	 ((obj)->type == LispReal_t && (long)(obj)->data.real == (obj)->data.real))

/* positive integer */
#define INDEX_P(obj)		(INTEGER_P(obj) && NUMBER_VALUE(obj) >= 0)

#define SYMBOL_P(obj)		((obj)->type == LispAtom_t)
#define STRING_P(obj)		((obj)->type == LispString_t)

/*
 * Types
 */
typedef struct _LispObj LispObj;
typedef struct _LispAtom LispAtom;
typedef struct _LispBuiltin LispBuiltin;
typedef struct _LispModuleData LispModuleData;

typedef enum _LispType {
    LispNil_t,
    LispTrue_t,
    LispAtom_t,
    LispInteger_t,
    LispReal_t,
    LispCons_t,
    LispQuote_t,
    LispCharacter_t,
    LispString_t,
    LispLambda_t,
    LispArray_t,
    LispStruct_t,
    LispStream_t,
    LispOpaque_t,
    LispBackquote_t,
    LispComma_t
} LispType;

typedef enum _LispFunType {
    LispLambda,
    LispFunction,
    LispMacro,
    LispSetf
} LispFunType;

struct _LispObj {
    LispType type : 6;
    unsigned int mark : 1;	/* gc protected */
    unsigned int dirty : 1;
    unsigned int prot: 1;	/* protection for constant/unamed variables */
    union {
	LispAtom *atom;
	long integer;
	double real;
	LispObj *quote;
	struct {
	    LispObj *car;
	    LispObj *cdr;
	} cons;
	struct {
	    LispObj *name;
	    LispObj *code;
	    unsigned int num_args : 12;
	    LispFunType type : 4;
	    unsigned int key : 1;
	    unsigned int optional : 1;
	    unsigned int rest : 1;
	} lambda;
	struct {
	    LispObj *list;		/* stored as a linear list */
	    LispObj *dim;		/* dimensions of array */
	    unsigned int rank : 8;	/* i.e. array-rank-limit => 256 */
	    unsigned int type : 7;	/* converted to LispType, if not
					 * Lisp{Nil,True}_t only accepts given
					 * type in array fields */
	    unsigned int zero : 1;	/* at least one of the dimensions
					 * is zero */
	} array;
	struct {
	    LispObj *fields;	/* structure fields */
	    LispObj *def;	/* structure definition */
	} struc;
	struct {
	    union {
		FILE *fp;
		unsigned char *str;
	    } source;
	    int size;		/* if smaller than zero, use source.fp */
	    int idx;		/* index in string */
	} stream;
	struct {
	    void *data;
	    int type;
	} opaque;
	struct {
	    LispObj *eval;
	    int atlist;
	} comma;
    } data;
};

struct _LispBuiltin {
    char *name;
    LispObj *(*fn)(LispMac*, LispObj*, char*);
    int eval : 1;
    int min_args : 15;
    int max_args : 15;
};

typedef	LispObj *(*LispFunPtr)(LispMac*, LispObj*, char*);
typedef int (*LispLoadModule)(LispMac*);
typedef int (*LispUnloadModule)(LispMac*);

#define LISP_MODULE_VERSION		1
struct _LispModuleData {
    int version;
    LispLoadModule load;
    LispUnloadModule unload;
};

/*
 * Prototypes
 */
LispObj *LispEval(LispMac*, LispObj*);

LispObj *LispNew(LispMac*, LispObj*, LispObj*);
LispObj *LispNewAtom(LispMac*, char*);
LispObj *LispNewReal(LispMac*, double);
LispObj *LispNewString(LispMac*, char*);
LispObj *LispNewCharacter(LispMac*, long);
LispObj *LispNewInteger(LispMac*, long);
LispObj *LispNewQuote(LispMac*, LispObj*);
LispObj *LispNewBackquote(LispMac*, LispObj*);
LispObj *LispNewComma(LispMac*, LispObj*, int);
LispObj *LispNewCons(LispMac*, LispObj*, LispObj*);
LispObj *LispNewSymbol(LispMac*, char*, LispObj*);
LispObj *LispNewLambda(LispMac*, LispObj*, LispObj*, LispObj*,
		       int, LispFunType, int, int, int);
LispObj *LispNewStruct(LispMac*, LispObj*, LispObj*);
LispObj *LispNewOpaque(LispMac*, void*, int);

char *LispGetString(LispMac*, char*);

/* This function does not allocate a copy of it's argument, but the argument
 * itself. The argument string should never change. */
char *LispGetPermString(LispMac*, char*);

void *LispMalloc(LispMac*, unsigned);
void *LispCalloc(LispMac*, unsigned, unsigned);
void *LispRealloc(LispMac*, void*, unsigned);
char *LispStrdup(LispMac*, char*);
void LispFree(LispMac*, void*);
/* LispMused means memory is now safe from LispDestroy, and should not be
 * freed in case of an error */
void LispMused(LispMac*, void*);

void LispGC(LispMac*, LispObj*, LispObj*);

char *LispStrObj(LispMac*, LispObj*);

void LispDestroy(LispMac *mac, char *fmt, ...);

LispObj *LispSetVariable(LispMac*, LispObj*, LispObj*, char*, int);

int LispRegisterOpaqueType(LispMac*, char*);

int LispPrintf(LispMac*, LispObj*, char*, ...);
int LispPrintString(LispMac*, LispObj*, char*);
int LispPrintObj(LispMac*, LispObj*, LispObj*, int);

void LispProtect(LispMac*, LispObj*, LispObj*);
void LispUProtect(LispMac*, LispObj*, LispObj*);

/* search argument list for the specified keys.
 * example: LispGetKeys(mac, fname, "START:END", list, &start, &end);
 * note that the separator for key names is the ':' character.
 * values not present in the argument list get the default value of NIL,
 * values specified more than once get only the first specification,
 * and if an unknown is on the argument list, a fatal error happens. */
void LispGetKeys(LispMac*, char*, char*, LispObj*, ...);

/* this function should be called when a module is loaded, and is called
 * when loading the interpreter */
void LispAddBuiltinFunction(LispMac*, LispBuiltin*);

/*
 * Initialization
 */
extern LispObj *NIL, *T;
extern int gcpro;
extern FILE *lisp_stdin, *lisp_stdout, *lisp_stderr;

#endif /* Lisp_internal_h */

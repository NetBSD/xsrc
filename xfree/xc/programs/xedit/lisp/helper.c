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

/* $XFree86: xc/programs/xedit/lisp/helper.c,v 1.15 2001/10/28 03:34:29 tsi Exp $ */

#include "helper.h"
#include <ctype.h>

/*
 * Prototypes
 */
static LispObj *_LispReallyDo(LispMac*, LispObj*, char*, int);
static LispObj *_LispReallyDoListTimes(LispMac*, LispObj*, char*, int);
extern int LispGet(LispMac*);
extern int LispUnget(LispMac*);

/*
 * Implementation
 */
LispObj *
_LispEqual(LispMac *mac, LispObj *left, LispObj *right)
{
    LispObj *res = NIL;

    if (left->type == right->type) {
	switch (left->type) {
	    case LispNil_t:
	    case LispTrue_t:
		res = T;
		break;
	    case LispReal_t:
		if (left->data.real == right->data.real)
		    res = T;
		break;
	    case LispCharacter_t:
	    case LispInteger_t:
		if (left->data.integer == right->data.integer)
		    res = T;
		break;
	    case LispAtom_t:
	    case LispString_t:
		if (STRPTR(left) == STRPTR(right))
		    res = T;
		break;
	    case LispCons_t:
		if (_LispEqual(mac, CAR(left), CAR(right)) == T &&
		    _LispEqual(mac, CDR(left), CDR(right)) == T)
		    res = T;
		break;
	    case LispQuote_t:
		res = _LispEqual(mac, left->data.quote, right->data.quote);
		break;
	    case LispLambda_t:
		res = _LispEqual(mac, left->data.lambda.name,
				 right->data.lambda.name);
		break;
	    case LispOpaque_t:
		if (left->data.opaque.data == right->data.opaque.data)
		    res = T;
		break;
	    default:
		if (left == right)
		    res = T;
		break;
	}
    }

    return (res);
}

LispObj *
_LispNth(LispMac *mac, LispObj *list, char *name, int cdr)
{
    int count, maxpos;
    LispObj *nth = CAR(list), *seq = CDR(list), *setf = list;

    if (!INDEX_P(nth))
	LispDestroy(mac, "bad index %s, at %s", LispStrObj(mac, nth), name);
    if (seq->type != LispCons_t)
	LispDestroy(mac, "%s is not of type list, at %s",
		    LispStrObj(mac, seq), name);
    maxpos = NUMBER_VALUE(nth);
    for (count = 0, seq = CAR(seq);
	 count < maxpos && seq->type == LispCons_t;
	 ++count, setf = seq, seq = CDR(seq))
	;

    if (count == maxpos)
	return (cdr || seq == NIL ? seq : CAR(seq));

    return (NIL);
}

LispObj *
_LispFindPlace(LispMac *mac, LispObj *list, LispObj *ref)
{
    LispObj *place;

    for (; list->type == LispCons_t; list = CDR(list)) {
	if (CAR(list) == ref)
	    return (list);
	else if (CDR(list) == ref)
	    return (list);
	else if ((place = _LispFindPlace(mac, CAR(list), ref)) != NULL)
	    return (place);
    }

    return (NULL);
}

LispObj *
_LispMinMax(LispMac *mac, LispObj *list, char *name, int max)
{
    double real, val;
    LispObj *obj;

    obj = EVAL(CAR(list));
    if (!NUMBER_P(obj))
	LispDestroy(mac, ExpectingNumberAt, name);
    real = NUMBER_VALUE(obj);
    for (list = CDR(list); list != NIL; list = CDR(list)) {
	obj = EVAL(CAR(list));
	if (!NUMBER_P(obj))
	    LispDestroy(mac, ExpectingNumberAt, name);
	val = NUMBER_VALUE(obj); 
	if ((max && val > real) || (!max && val < real))
	    real = val;
    }

    return (REAL(real));
}

LispObj *
_LispBoolCond(LispMac *mac, LispObj *list, char *name, int op)
{
    LispObj *obj;
    double value, val;
    int cond;

    cond = 1;
    obj = op == NOT_EQUAL ? CAR(list) : EVAL(CAR(list));
    if (!NUMBER_P(obj))
	LispDestroy(mac, ExpectingNumberAt, name);
    value = NUMBER_VALUE(obj);

    /* special handling, as (/= 3 6 5 2) => T, but (/= 3 3 5 3) => NIL
     * all elements must be different to be True
     * Also, for NOT_EQUAL, arguments must be evaluated before calling
     * this function.
     */
    if (op == NOT_EQUAL) {
	LispObj *cmp;

	list = CDR(list);
	if (list == NIL)
	    return (T);

	/* check if numbers first */
	for (obj = list; obj != NIL; obj = CDR(obj))
	    if (CAR(obj)->type != LispReal_t)
		LispDestroy(mac, ExpectingNumberAt, name);

	do {
	    for (cmp = list; cmp != NIL; cmp = CDR(cmp)) {
		obj = CAR(cmp);
		if (value == NUMBER_VALUE(obj))
		    return (NIL);
	    }
	    value = NUMBER_VALUE(CAR(list));
	    list = CDR(list);
	} while (list != NIL);
	return (T);
    }

    for (list = CDR(list); list != NIL; list = CDR(list)) {
	obj = EVAL(CAR(list));
	if (!NUMBER_P(obj))
	    LispDestroy(mac, ExpectingNumberAt, name);
	val = NUMBER_VALUE(obj); 
	switch (op) {
	    case LESS:
		if (value >= val)
		    cond = 0;
		break;
	    case LESS_EQUAL:
		if (value > val)
		    cond = 0;
		break;
	    case EQUAL:
		if (value != val)
		    cond = 0;
		break;
	    case GREATER:
		if (value <= val)
		    cond = 0;
		break;
	    case GREATER_EQUAL:
		if (value < val)
		    cond = 0;
		break;
	}
	if (!cond)
	    break;
	value = val;
    }
    return (cond ? T : NIL);
}

LispObj *
_LispCharBoolCond(LispMac *mac, LispObj *list, char *name, int op, int cas)
{
    LispObj *obj;
    long value, comp;
    int cond;

    cond = 1;
    obj = op == NOT_EQUAL ? CAR(list) : EVAL(CAR(list));
    if (obj->type != LispCharacter_t)
	LispDestroy(mac, "expecting character, at %s", name);
    value = obj->data.integer;
    if (cas)
	value = toupper(value);

    if (op == NOT_EQUAL) {
	LispObj *cmp;

	list = CDR(list);
	if (list == NIL)
	    return (T);

	for (obj = list; obj != NIL; obj = CDR(obj))
	    if (CAR(obj)->type != LispCharacter_t)
		LispDestroy(mac, "expecting character, at %s", name);

	do {
	    for (cmp = list; cmp != NIL; cmp = CDR(cmp)) {
		obj = CAR(cmp);
		comp = obj->data.integer;
		if (cas)
		    comp = toupper(comp);
		if (value == comp)
		    return (NIL);
	    }
	    value = CAR(list)->data.integer;
	    if (cas)
		value = toupper(value);
	    list = CDR(list);
	} while (list != NIL);
	return (T);
    }

    for (list = CDR(list); list != NIL; list = CDR(list)) {
	obj = EVAL(CAR(list));
	if (obj->type != LispCharacter_t)
	    LispDestroy(mac, "expecting character, at %s", name);
	comp = obj->data.integer;
	if (cas)
	    comp = toupper(comp);
	switch (op) {
	    case LESS:
		if (value >= comp)
		    cond = 0;
		break;
	    case LESS_EQUAL:
		if (value > comp)
		    cond = 0;
		break;
	    case EQUAL:
		if (value != comp)
		    cond = 0;
		break;
	    case GREATER:
		if (value <= comp)
		    cond = 0;
		break;
	    case GREATER_EQUAL:
		if (value < comp)
		    cond = 0;
		break;
	}
	if (!cond)
	    break;
	value = obj->data.integer;
	if (cas)
	    value = toupper(value);
    }
    return (cond ? T : NIL);
}

LispObj *
_LispDefLambda(LispMac *mac, LispObj *list, LispFunType type)
{
    LispObj *name = NIL, *args, *code, *obj = NIL, *fun, *sto;
    static char *types[4] = {"LAMBDA", "FUNCTION", "MACRO", "SETF-METHOD"};
    static char *fnames[4] = {"LAMBDA", "DEFUN", "DEFMACRO", "DEFSETF"};
    int num_args, rest, optional, key;

    /* name */
    if (type != LispLambda) {
	if ((name = CAR(list))->type != LispAtom_t)
	    LispDestroy(mac, "%s cannot be a %s name, at %s",
			LispStrObj(mac, name), types[type], fnames[type]);
	list = CDR(list);
    }

    /* args */
    args = CAR(list);
    num_args = rest = optional = key = 0;

    if (args->type == LispCons_t) {
	for (obj = args; obj != NIL; obj = CDR(obj), ++num_args)
	    if (CAR(obj)->type == LispCons_t && (key || optional)) {
		/* is this a default value? */
		if (!SYMBOL_P(CAR(CAR(obj))))
		    LispDestroy(mac, "%s cannot be a %s argument name, at %s %s",
				LispStrObj(mac, CAR(CAR(obj))), types[type],
				fnames[type],
				type == LispLambda ? "..." : STRPTR(name));
		else if (CDR(CAR(obj)) != NIL &&
			 (CDR(CAR(obj))->type != LispCons_t ||
			  CDR(CDR(CAR(obj))) != NIL))
		    LispDestroy(mac, "bad argument specification %s, at %s %s",
				LispStrObj(mac, CAR(obj)), types[type],
				fnames[type],
				type == LispLambda ? "..." : STRPTR(name));
	    }
	    else if (!SYMBOL_P(CAR(obj)) ||
		STRPTR(CAR(obj))[0] == ':')
		LispDestroy(mac, "%s cannot be a %s argument name, at %s %s",
			    LispStrObj(mac, CAR(obj)), types[type], fnames[type],
			    type == LispLambda ? "..." : STRPTR(name));
	    else if (STRPTR(CAR(obj))[0] == '&') {
		if (strcmp(STRPTR(CAR(obj)) + 1, "REST") == 0) {
		    if (rest || CDR(obj) == NIL || CDR(CDR(obj)) != NIL)
			LispDestroy(mac, "syntax error parsing &REST,"
				    " at %s %s", fnames[type],
				    type == LispLambda ?
				    "..." : STRPTR(name));
		    rest = 1;
		}
		else if (strcmp(STRPTR(CAR(obj)) + 1, "KEY") == 0) {
		    if (rest)
			LispDestroy(mac, "&KEY not allowed after &REST,"
				    " at %s %s", fnames[type],
				    type == LispLambda ?
				    "..." : STRPTR(name));
		    if (key || optional || CDR(obj) == NIL)
			LispDestroy(mac, "syntax error parsing &KEY,"
				    " at %s %s", fnames[type],
				    type == LispLambda ?
				    "..." : STRPTR(name));
		    key = 1;
		}
		else if (strcmp(STRPTR(CAR(obj)) + 1, "OPTIONAL") == 0) {
		    if (rest)
			LispDestroy(mac, "&OPTIONAL not allowed after &REST,"
				    " at %s %s", fnames[type],
				    type == LispLambda ?
				    "..." : STRPTR(name));
		    if (key || optional || CDR(obj) == NIL)
			LispDestroy(mac, "syntax error parsing &OPTIONAL,"
				    " at %s %s", fnames[type],
				    type == LispLambda ?
				    "..." : STRPTR(name));
		    optional = 1;
		}
		else
		    LispDestroy(mac, "%s not allowed %at %s %s",
				STRPTR(CAR(obj)), fnames[type],
				type == LispLambda ? "..." : STRPTR(name));
	    }
    }
    else if (args != NIL)
	LispDestroy(mac, "%s cannot be a %s argument list, at %s %s",
		    LispStrObj(mac, args), types[type], fnames[type],
		    type == LispLambda ? "..." : STRPTR(name));

    if (type == LispSetf) {
	list = CDR(list);
	sto = CAR(list);
	if (sto->type != LispCons_t)
	    LispDestroy(mac, "%s is a bad store value, at %s",
			LispStrObj(mac, sto), fnames[type]);
	for (obj = CAR(sto); obj->type == LispCons_t; obj = CDR(obj))
	    if (!SYMBOL_P(CAR(obj)) || STRPTR(CAR(obj))[0] == ':')
		LispDestroy(mac, "%s cannot be a variable name, at %s",
			    LispStrObj(mac, CAR(obj)), fnames[type]);
	args = CONS(args, sto);
    }

    /* code */
    code = CDR(list);

    GCProtect();
    fun = LispNewLambda(mac, name, args, code, num_args, type,
			key, optional, rest);
    GCUProtect();

    if (type == LispSetf)
	LispSetAtomSetfProperty(mac, name->data.atom, fun);
    else if (type != LispLambda) {
	if (name->data.atom->property) {
	    if ((name->data.atom->property->function ||
		name->data.atom->property->builtin))
		fprintf(lisp_stderr, "*** Warning: %s is being redefined\n",
			STRPTR(name));

	    if (name->data.atom->property->builtin)
		LispRemAtomBuiltinProperty(mac, name->data.atom);
	}
	LispSetAtomFunctionProperty(mac, name->data.atom, fun);
    }

    return (type != LispLambda ? name : fun);
}

static LispObj *
_LispReallyDo(LispMac *mac, LispObj *list, char *fname, int refs)
{
    LispObj *old_frm, *old_env, *env, *res, *args, *test, *body, *obj;

    env = res = NIL;
    old_frm = FRM;
    old_env = ENV;
    args = CAR(list);
    test = CAR(CDR(list));
    body = CDR(CDR(list));

    if (test->type != LispCons_t)
	LispDestroy(mac, "end test condition must be a list, at %s",
		    LispStrObj(mac, args), fname);

    /* Add variables */
    if (args != NIL && args->type != LispCons_t)
	LispDestroy(mac, "%s is not of type list, at %s",
		    LispStrObj(mac, args), fname);

    for (obj = args; obj != NIL; obj = CDR(obj)) {
	LispObj *var, *val, *step;

	var = val = NIL;
	step = NULL;
	list = CAR(obj);
	if (SYMBOL_P(list))
	    var = list;
	else if (list->type != LispCons_t)
		LispDestroy(mac, "%s is not of type list, at %s",
			    LispStrObj(mac, list), fname);
	else {
	    if ((var = CAR(list))->type != LispAtom_t)
		LispDestroy(mac, "%s is invalid as a variable name, at %s",
			    LispStrObj(mac, var), fname);
	    if ((list = CDR(list)) != NIL) {
		val = EVAL(CAR(list));
		if ((list = CDR(list)) != NIL)
		    step = CAR(list);
	    }
	}
	GCProtect();
	if (step)
	    list = CONS(var, CONS(val, CONS(step, NIL)));
	else
	    list = CONS(var, CONS(val, NIL));
	if (env == NIL) {
	    env = CONS(list, NIL);
	    FRM = CONS(env, FRM);
	}
	else {
	    CDR(env) = CONS(CAR(env), CDR(env));
	    CAR(env) = list;
	}
	GCUProtect();
	if (refs)
	    LispAddVar(mac, var, val);
    }

    /* Need to update CAR(FRM) or will run loop without gc protection! */
    env = CAR(FRM) = LispReverse(env);
    if (!refs) {
	for (obj = env; obj != NIL; obj = CDR(obj)) {
	    list = CAR(obj);
	    LispAddVar(mac, CAR(list), CAR(CDR(list)));
	}
    }

    /* Execute iterations */
    for (;;) {
	if (EVAL(CAR(test)) != NIL) {
	    if (CDR(test) != NIL)
		res = EVAL(CAR(CDR(test)));
	    break;
	}
	(void)Lisp_Progn(mac, body, fname);
	/* Update variables */
	for (obj = env; obj != NIL; obj = CDR(obj)) {
	    list = CAR(obj);
	    if (CDR(CDR(list)) != NIL)
		LispSetVar(mac, CAR(list),
			   EVAL(CAR(CDR(CDR(list)))));
	}
    }

    ENV = old_env;
    FRM = old_frm;

    return (res);
}

LispObj *
_LispDo(LispMac *mac, LispObj *list, char *fname, int refs)
{
    int did_jump, *pdid_jump = &did_jump;
    LispObj *res, **pres = &res;
    LispBlock *block;

    *pres = NIL;
    *pdid_jump = 1;
    block = LispBeginBlock(mac, NIL, LispBlockTag);
    if (setjmp(block->jmp) == 0) {
	*pres = _LispReallyDo(mac, list, fname, refs);
	*pdid_jump = 0;
    }
    LispEndBlock(mac, block);
    if (*pdid_jump)
	*pres = mac->block.block_ret;

    return (*pres);
}

static LispObj *
_LispReallyDoListTimes(LispMac *mac, LispObj *list, char *fname, int times)
{
    double count = 0.0;
    LispObj *var, *val = NIL, *res = NIL, *body, *old_frm, *old_env;

    /* Parse arguments */
    if (CAR(list)->type != LispCons_t)
	LispDestroy(mac, "expecting list, at %s", fname);
    body = CDR(list);
    list = CAR(list);
    if ((var = CAR(list))->type != LispAtom_t)
	LispDestroy(mac, "%s is invalid as a variable name, at %s",
		    LispStrObj(mac, var), fname);
    list = CDR(list);

    /* Save environment */
    old_frm = FRM;
    old_env = ENV;

    if (list == NIL) {
	if (!times)
	    val = res = NIL;
	else
	    LispDestroy(mac, "NIL is not a number, at %s", fname);
    }
    else {
	if (list->type == LispCons_t) {
	    val = CAR(list);
	    list = CDR(list);
	    if (list == NIL)
		res = NIL;
	    else if (list->type == LispCons_t)
		res = CAR(list);
	    else
		LispDestroy(mac, "expecting list, at %s", fname);
	}
	else
	    LispDestroy(mac, "%s is not a list, at %s",
			LispStrObj(mac, val), fname);

	val = EVAL(val);

	if (times && (!INTEGER_P(val)))
	    LispDestroy(mac, "%s is not an integer, at %s",
			LispStrObj(mac, val), fname);
	else if (!times && (val != NIL && val->type != LispCons_t))
	    LispDestroy(mac, "%s is not a list, at %s",
			LispStrObj(mac, val), fname);
    }

    /* Protect iteration control from gc */
    FRM = CONS(val, FRM);

    /* Initialize counter */
    if (times)
	LispAddVar(mac, var, REAL(count));
    else
	LispAddVar(mac, var, CAR(val));

    /* Execute iterations */
    for (;;) {
	/* Check loop */
	if (times) {
	    if ((count += 1.0) > NUMBER_VALUE(val))
		break;
	}
	else if (val == NIL)
	    break;

	(void)Lisp_Progn(mac, body, fname);

	/* Update variables */
	if (times)
	    LispSetVar(mac, var, REAL(count));
	else {
	    val = CDR(val);
	    if (val == NIL)
		break;
	    else if (val->type != LispCons_t)
		LispDestroy(mac, "true list required, at %s", fname);
	    LispSetVar(mac, var, CAR(val));
	}
    }

    ENV = old_env;
    FRM = old_frm;

    return (res == NIL ? NIL : EVAL(res));
}

LispObj *
_LispDoListTimes(LispMac *mac, LispObj *list, char *fname, int times)
{
    int did_jump, *pdid_jump = &did_jump;
    LispObj *res, **pres = &res;
    LispBlock *block;

    *pres = NIL;
    *pdid_jump = 1;
    block = LispBeginBlock(mac, NIL, LispBlockTag);
    if (setjmp(block->jmp) == 0) {
	*pres = _LispReallyDoListTimes(mac, list, fname, times);
	*pdid_jump = 0;
    }
    LispEndBlock(mac, block);
    if (*pdid_jump)
	*pres = mac->block.block_ret;

    return (*pres);
}

LispObj *
_LispSet(LispMac *mac, LispObj *var, LispObj *val, char *fname, int eval)
{
    char *name;

    if (!SYMBOL_P(var))
	LispDestroy(mac, "%s is not a symbol, at %s",
		    LispStrObj(mac, var), fname);

    name = STRPTR(var);
    if (isdigit(name[0]) || name[0] == '(' || name[0] == ')'
	|| name[0] == ';' || name[0] == '\'' || name[0] == '#')
	LispDestroy(mac, "bad name %s, at %s", name, fname);
    if (eval)
	val = EVAL(val);

    return (LispSetVar(mac, var, val));
}

LispObj *
_LispWhenUnless(LispMac *mac, LispObj *list, int op)
{
    LispObj *obj, *res = NIL;

    obj = EVAL(CAR(list));
    if ((obj->type == LispNil_t) ^ op) {
	for (obj = CDR(list); obj != NIL; obj = CDR(obj))
	    res = EVAL(CAR(obj));
    }
    return (res);
}

LispObj *
_LispWhileUntil(LispMac *mac, LispObj *list, int op)
{
    LispObj *obj, *res = NIL;

    /*CONSTCOND*/
    while (1) {
	obj = EVAL(CAR(list));
	if ((obj->type == LispNil_t) ^ op) {
	    for (obj = CDR(list); obj != NIL; obj = CDR(obj))
		res = EVAL(CAR(obj));
	}
	else
	    break;
    }
    return (res);
}

LispObj *
_LispLoadFile(LispMac *mac, char *filename, char *fname,
	      int verbose, int print, int ifdoesnotexist)
{
    LispObj *obj, *res = NIL;
    FILE *fp;
    int ch, level;

    if ((fp = fopen(filename, "r")) == NULL) {
	if (ifdoesnotexist)
	    LispDestroy(mac, "cannot open %s, at %s", filename, fname);
	return (NIL);
    }

    if (verbose)
	fprintf(lisp_stderr, "; Loading %s\n", filename);

    if (mac->stream.stream_level + 1 >= mac->stream.stream_size) {
	LispStream *stream = (LispStream*)
	    realloc(mac->stream.stream, sizeof(LispStream) *
		    (mac->stream.stream_size + 1));

	if (stream == NULL) {
	    fclose(fp);
	    LispDestroy(mac, "out of memory");
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
    mac->stream.stream[mac->stream.stream_level].fp = fp;
    mac->fp = fp;
    mac->st = mac->cp = NULL;
    mac->tok = 0;

    level = mac->level;
    mac->level = 0;

    ch = LispGet(mac);
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

    /*CONSTCOND*/
    while (1) {
	if ((obj = LispRun(mac)) != NULL) {
	    if (obj == EOLIST)
		LispDestroy(mac, "object cannot start with #\\)");
	    res = EVAL(obj);
	    if (print)
		LispPrint(mac, res, NIL, 1);
	}
	if (mac->tok == EOF)
	    break;
    }
    mac->level = level;
    free(mac->st);
    --mac->stream.stream_level;

    mac->fp = mac->stream.stream[mac->stream.stream_level].fp;
    mac->st = mac->stream.stream[mac->stream.stream_level].st;
    mac->cp = mac->stream.stream[mac->stream.stream_level].cp;
    mac->tok = mac->stream.stream[mac->stream.stream_level].tok;

    return (res);
}

void
_LispGetStringArgs(LispMac *mac, LispObj *list, char *fname,
		   char **string1, char **string2,
		   int *start1, int *end1, int *start2, int *end2)
{
    int len1, len2;
    LispObj *lstring1, *lstring2, *lstart1, *lend1, *lstart2, *lend2;

    lstring1 = CAR(list);
    list = CDR(list);
    lstring2 = CAR(list);
    if ((!STRING_P(lstring1) && !SYMBOL_P(lstring1)) ||
	(!STRING_P(lstring2) && !SYMBOL_P(lstring2)))
	LispDestroy(mac, "expecting string, at %s", fname);

    *string1 = STRPTR(lstring1);
    *string2 = STRPTR(lstring2);

    LispGetKeys(mac, fname, "START1:END1:START2:END2", CDR(list),
		&lstart1, &lend1, &lstart2, &lend2);

    if ((lstart1 != NIL && !INDEX_P(lstart1)) ||
	(lend1 != NIL && !INDEX_P(lend1)) ||
	(lstart2 != NIL && !INDEX_P(lstart2)) ||
	(lend2 != NIL && !INDEX_P(lend2)))
	LispDestroy(mac, "expecting positive integer, at %s", fname);

    len1 = strlen(*string1);
    *start1 = lstart1 == NIL ? 0 : NUMBER_VALUE(lstart1);
    *end1 = lend1 == NIL ? len1 : NUMBER_VALUE(lend1);
    len2 = strlen(*string2);
    *start2 = lstart2 == NIL ? 0 : NUMBER_VALUE(lstart2);
    *end2 = lend2 == NIL ? len2 : NUMBER_VALUE(lend2);

    if (*start1 > *end1 || *end1 > len1 || *start2 > *end2 || *end2 > len2)
	LispDestroy(mac, "bad string index, at %s", fname);
}

void
_LispGetStringCaseArgs(LispMac *mac, LispObj *list, char *fname,
		       char **string, int *start, int *end)
{
    int len;
    LispObj *lstring, *lstart, *lend;

    lstring = CAR(list);
    if (!STRING_P(lstring) && !SYMBOL_P(lstring))
	LispDestroy(mac, "expecting string, at %s", fname);

    *string = STRPTR(lstring);

    LispGetKeys(mac, fname, "START:END", CDR(list), &lstart, &lend);

    if ((lstart != NIL && !INDEX_P(lstart)) ||
	(lend != NIL && !INDEX_P(lend)))
	LispDestroy(mac, "expecting positive integer, at %s", fname);

    len = strlen(*string);
    *start = lstart == NIL ? 0 : NUMBER_VALUE(lstart);
    *end = lend == NIL ? len : NUMBER_VALUE(lend);

    if (*start > *end || *end > len)
	LispDestroy(mac, "bad string index, at %s", fname);
}

LispObj *
_LispStringDoTrim(LispMac *mac, LispObj *list, char *fname, int left, int right)
{
    char *str;
    int start, end, sstart, send, len;
    LispObj *chars, *string;

    chars = CAR(list);
    if (!STRING_P(chars) && chars->type != LispCons_t)
	LispDestroy(mac, "%s is not a sequence, at %s",
		    LispStrObj(mac, chars), fname);
    string = CAR(CDR(list));
    if (!STRING_P(string) && !SYMBOL_P(string))
	LispDestroy(mac, "expecting string, at %s", fname);

    sstart = start = 0;
    send = end = strlen(STRPTR(string));

    if (STRING_P(chars)) {
	char *cmp;

	if (left) {
	    for (str = STRPTR(string); *str; str++) {
		for (cmp = STRPTR(chars); *cmp; cmp++)
		    if (*str == *cmp)
			break;
		if (*cmp == '\0')
		    break;
		++start;
	    }
	}
	if (right) {
	    for (str = STRPTR(string) + end - 1; end > 0; str--) {
		for (cmp = STRPTR(chars); *cmp; cmp++)
		    if (*str == *cmp)
			break;
		if (*cmp == '\0')
		    break;
		--end;
	    }
	}
    }
    else {
	LispObj *obj;

	if (left) {
	    for (str = STRPTR(string); *str; str++) {
		for (obj = chars; obj != NIL; obj = CDR(obj))
		    /* Should really ignore non character input ? */
		    if (CAR(obj)->type == LispCharacter_t &&
			*str == CAR(obj)->data.integer)
			break;
		if (obj == NIL)
		    break;
		++start;
	    }
	}
	if (right) {
	    for (str = STRPTR(string) + end - 1; end > 0; str--) {
		for (obj = chars; obj != NIL; obj = CDR(obj))
		    /* Should really ignore non character input ? */
		    if (CAR(obj)->type == LispCharacter_t &&
			*str == CAR(obj)->data.integer)
			break;
		if (obj == NIL)
		    break;
		--end;
	    }
	}
    }

    if (sstart == start && send == end)
	return (string);

    len = end - start;
    str = LispMalloc(mac, len + 1);
    strncpy(str, STRPTR(string) + start, len);
    str[len] = '\0';

    string = STRING(str);
    LispFree(mac, str);

    return (string);
}

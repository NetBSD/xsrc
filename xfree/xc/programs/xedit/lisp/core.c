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

/* $XFree86: xc/programs/xedit/lisp/core.c,v 1.17 2001/10/28 14:18:49 tsi Exp $ */

#include "core.h"
#include "format.h"
#include "helper.h"
#include "private.h"

/*
 * Prototypes
 */
extern LispObj *LispRunSetf(LispMac*, LispObj*, LispObj*, LispObj*);

/*
 * Implementation
 */
LispObj *
Lisp_Mul(LispMac *mac, LispObj *list, char *fname)
{
    double result = 1.0;

    for (; list != NIL; list = CDR(list)) {
	if (!NUMBER_P(CAR(list)))
	    LispDestroy(mac, ExpectingNumberAt, fname);
	result *= NUMBER_VALUE(CAR(list));
    }
    return (REAL(result));
}

LispObj *
Lisp_Plus(LispMac *mac, LispObj *list, char *fname)
{
    double result = 0.0;

    for (; list != NIL; list = CDR(list)) {
	if (!NUMBER_P(CAR(list)))
	    LispDestroy(mac, ExpectingNumberAt, fname);
	result += NUMBER_VALUE(CAR(list));
    }
    return (REAL(result));
}

LispObj *
Lisp_Minus(LispMac *mac, LispObj *list, char *fname)
{
    double result;

    if (!NUMBER_P(CAR(list)))
	LispDestroy(mac, ExpectingNumberAt, fname);
    result = NUMBER_VALUE(CAR(list));
    list = CDR(list);
    if (list == NIL)
	return (REAL(-result));
    for (; list != NIL; list = CDR(list)) {
	if (!NUMBER_P(CAR(list)))
	    LispDestroy(mac, ExpectingNumberAt, fname);
	result -= NUMBER_VALUE(CAR(list));
    }
    return (REAL(result));
}

LispObj *
Lisp_Div(LispMac *mac, LispObj *list, char *fname)
{
    double result;

    if (!NUMBER_P(CAR(list)))
	LispDestroy(mac, ExpectingNumberAt, fname);
    result = NUMBER_VALUE(CAR(list));
    list = CDR(list);
    if (list == NIL) {
	if (result == 0.0)
	    LispDestroy(mac, "divide by 0, at %s", fname);
	return (REAL(1.0 / result));
    }
    for (; list != NIL; list = CDR(list)) {
	if (!NUMBER_P(CAR(list)))
	    LispDestroy(mac, ExpectingNumberAt, fname);
	if (NUMBER_VALUE(CAR(list)))
	    LispDestroy(mac, "divide by 0, at %s", fname);
	result /= NUMBER_VALUE(CAR(list));
    }
    return (REAL(result));
}

LispObj *
Lisp_OnePlus(LispMac *mac, LispObj *list, char *fname)
{
    if (!NUMBER_P(CAR(list)))
	LispDestroy(mac, ExpectingNumberAt, fname);
    return (REAL(NUMBER_VALUE(CAR(list)) + 1.0));
}

LispObj *
Lisp_OneMinus(LispMac *mac, LispObj *list, char *fname)
{
    if (!NUMBER_P(CAR(list)))
	LispDestroy(mac, ExpectingNumberAt, fname);
    return (REAL(NUMBER_VALUE(CAR(list)) - 1.0));
}

LispObj *
Lisp_Less(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispBoolCond(mac, list, fname, LESS));
}

LispObj *
Lisp_LessEqual(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispBoolCond(mac, list, fname, LESS_EQUAL));
}

LispObj *
Lisp_Equal_(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispBoolCond(mac, list, fname, EQUAL));
}

LispObj *
Lisp_Greater(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispBoolCond(mac, list, fname, GREATER));
}

LispObj *
Lisp_GreaterEqual(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispBoolCond(mac, list, fname, GREATER_EQUAL));
}

LispObj *
Lisp_NotEqual(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispBoolCond(mac, list, fname, NOT_EQUAL));
}

LispObj *
Lisp_Aref(LispMac *mac, LispObj *list, char *fname)
{
    long c, count, idx, seq;
    LispObj *ary = CAR(list), *dim = CDR(list), *obj;

    if (ary->type != LispArray_t)
	LispDestroy(mac, "%s is not an array, at %s",
		    LispStrObj(mac, ary), fname);

    for (count = 0, list = dim, obj = ary->data.array.dim; list != NIL;
	 count++, list = CDR(list), obj = CDR(obj)) {
	if (count >= ary->data.array.rank)
	    LispDestroy(mac, "too many subscripts %s, at %s",
			LispStrObj(mac, dim), fname);
	if (!INDEX_P(CAR(list)) ||
	    NUMBER_VALUE(CAR(list)) >= NUMBER_VALUE(CAR(obj)))
	    LispDestroy(mac, "%s is out of range or a bad index, at %s",
			LispStrObj(mac, CAR(list)), fname);
    }
    if (count < ary->data.array.rank)
	LispDestroy(mac, "too few subscripts %s, at %s",
		    LispStrObj(mac, dim), fname);

    for (count = seq = 0, list = dim; list != NIL; list = CDR(list), seq++) {
	for (idx = 0, obj = ary->data.array.dim; idx < seq; obj = CDR(obj), ++idx)
	    ;
	for (c = 1, obj = CDR(obj); obj != NIL; obj = CDR(obj))
	    c *= NUMBER_VALUE(CAR(obj));
	count += c * NUMBER_VALUE(CAR(list));
    }

    for (ary = ary->data.array.list; count > 0; ary = CDR(ary), count--)
	;

    return (CAR(ary));
}

LispObj *
Lisp_Assoc(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *cmp, *obj, *res = NIL;

    cmp = CAR(list);
    list = CDR(list);
    if (list == NIL)
	return (NIL);

    for (list = CAR(list); list != NIL; list = CDR(list)) {
	if (list->type != LispCons_t || (obj = CAR(list))->type != LispCons_t)
	    LispDestroy(mac, ExpectingListAt, fname);
	else if (_LispEqual(mac, cmp, CAR(obj)) == T) {
	    res = obj;
	    break;
	}
    }

    return (res);
}

LispObj *
Lisp_And(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res = T;

    for (; list != NIL; list = CDR(list)) {
	if ((res = EVAL(CAR(list))) == NIL)
	    break;
    }
    return (res);
}

LispObj *
Lisp_Append(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res, *obj, *cdr, *cons, *frm;

    if (list == NIL)
	return (NIL);
    else if (CDR(list) == NIL)
	return (EVAL(CAR(list)));

    frm = FRM;
    res = cdr = NIL;

    for (; list != NIL; list = CDR(list)) {
	if ((obj = EVAL(CAR(list))) == NIL)
	    continue;
	if (obj->type != LispCons_t) {
	    if (CDR(list) != NIL)
		LispDestroy(mac, ExpectingListAt, fname);
	}
	GCProtect();
	if (res == NIL) {
	    if (obj->type != LispCons_t) {
		/* is last argument, as previous check didn't fail */
		res = obj;
		GCUProtect();
		break;
	    }
	    res = cdr = CONS(CAR(obj), CDR(obj));
	    FRM = CONS(res, FRM);
	}
	else {
	    if (CDR(cdr)->type == LispCons_t) {
		cons = CDR(cdr);
		while (CDR(cons)->type == LispCons_t) {
		    CDR(cdr) = CONS(CAR(cons), CDR(cons));
		    cons = CDR(cons);
		    cdr = CDR(cdr);
		}
		CDR(cdr) = CONS(CAR(CDR(cdr)), obj);
	    }
	    else
		CDR(cdr) = obj;
	    cdr = CDR(cdr);
	}
	GCUProtect();
    }
    FRM = frm;

    return (res);
}

LispObj *
Lisp_Apply(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj, *eval, *cdr, *frm = FRM;

    eval = EVAL(CAR(list));
    if (!SYMBOL_P(eval) && eval->type != LispLambda_t)
	LispDestroy(mac, "%s is not a valid function name, at %s",
		LispStrObj(mac, eval), fname);
    obj = NIL;
    /* link eval to FRM to protect from GC */
    GCProtect();
    eval = cdr = CONS(eval, NIL);
    FRM = CONS(eval, FRM);
    GCUProtect();
    for (list = CDR(list); list != NIL; list = CDR(list)) {
	obj = EVAL(CAR(list));
	if (CDR(list) != NIL) {
	    CDR(cdr) = CONS(obj, NIL);
	    cdr = CDR(cdr);
	}
	else
	    CDR(cdr) = obj;
    }
    if (obj != NIL && (obj->type != LispCons_t || CDR(obj)->type != LispCons_t))
	LispDestroy(mac, "last apply argument must be a list");
    /* Need to quote back to avoid double evaluation */
    while (obj != NIL) {
	CAR(obj) = QUOTE(CAR(obj));
	obj = CDR(obj);
    }

    frm = FRM;

    return (EVAL(eval));
}

LispObj *
Lisp_Atom(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res;

    if (list == NIL)
	res = T;
    else
	switch (CAR(list)->type) {
	    case LispCons_t:
		res = NIL;
		break;
	    default:
		res = T;
		break;
	}
    return (res);
}

LispObj *
Lisp_Block(LispMac *mac, LispObj *list, char *fname)
{
    int did_jump, *pdid_jump = &did_jump;
    LispObj *res, **pres = &res;
    LispBlock *block;

    if (CAR(list) != NIL && CAR(list) != T && !SYMBOL_P(CAR(list)))
	LispDestroy(mac, "%s is not a symbol, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    *pres = NIL;
    *pdid_jump = 1;
    block = LispBeginBlock(mac, CAR(list), LispBlockTag);
    if (setjmp(block->jmp) == 0) {
	*pres = Lisp_Progn(mac, CDR(list), fname);
	*pdid_jump = 0;
    }
    LispEndBlock(mac, block);
    if (*pdid_jump)
	*pres = mac->block.block_ret;

    return (*pres);
}

LispObj *
Lisp_Butlast(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res, *obj, *cdr;
    int nlist;

    if ((obj = CAR(list))->type != LispCons_t)
	LispDestroy(mac, "%s is not a list, at %s",
		LispStrObj(mac, obj), fname);
    cdr = NIL;
    nlist = 0;
    while (obj->type == LispCons_t) {
	++nlist;
	obj = CDR(obj);
    }
    --nlist;
    obj = CDR(list);
    if (obj != NIL) {
	obj = CAR(obj);
	if (INDEX_P(obj)) {
	    int pos = NUMBER_VALUE(obj);

	    if (pos == 0) {
		res = CAR(list);
		if (CDR(res)->type != LispCons_t)
		    /* CL compatible return value */
		    return (CONS(CAR(res), NIL));
		return (res);
	    }
	    else {
		if (pos > nlist)
		    return (NIL);
		nlist -= pos - 1;
	    }
	}
	else
	    LispDestroy(mac, "%s is a invalid index, at %s",
			LispStrObj(mac, obj), fname);
    }

    GCProtect();	/* just disable GC, no EVAL's below */
    res = NIL;
    list = CAR(list);
    for (; nlist > 0; list = CDR(list), nlist--) {
	obj = CAR(list);
	if (res == NIL)
	    res = cdr = CONS(obj, NIL);
	else {
	    CDR(cdr) = CONS(obj, NIL);
	    cdr = CDR(cdr);
	}
    }
    GCUProtect();

    return (res);
}

LispObj *
Lisp_Car(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res = NIL;

    switch (CAR(list)->type) {
	case LispNil_t:
	    return (NIL);
	    break;
	case LispCons_t:
	    res = CAR(CAR(list));
	    break;
	default:
	    LispDestroy(mac, ExpectingListAt, fname);
	    /*NOTREACHED*/
    }

    return (res);
}

LispObj *
Lisp_Case(LispMac *mac, LispObj *list, char *fname)
{
    static char *other = "OTHERWISE";
    LispObj *key, *res;

    res = NIL;
    key = EVAL(CAR(list));

    for (list = CDR(list); list != NIL; list = CDR(list)) {
	if (CAR(list)->type != LispCons_t)
	    LispDestroy(mac, "expecting list, at %s", fname);
	else if (CAR(CAR(list)) == T) {
	    if (CDR(list) != NIL)
		LispDestroy(mac, "T must be the last clause, at %s", fname);
	    res = CDR(CAR(list));
	    break;
	}
	else if (SYMBOL_P(CAR(CAR(list))) &&
		 strcmp(other, STRPTR(CAR(CAR(list)))) == 0) {
	    if (CDR(list) != NIL)
		LispDestroy(mac, "%s must be the last clause, at %s",
			    other, fname);
	    res = CDR(CAR(list));
	    break;
	}
	else if (CAR(CAR(list))->type == LispCons_t) {
	    LispObj *keylist = CAR(CAR(list));

	    for (; keylist->type == LispCons_t; keylist = CDR(keylist))
		if (_LispEqual(mac, key, CAR(keylist)) == T) {
		    res = CDR(CAR(list));
		    break;
		}
	    if (keylist->type == LispCons_t)	/* if found match */
		break;
	}
	else if (_LispEqual(mac, key, CAR(CAR(list))) == T) {
	    res = CDR(CAR(list));
	    break;
	}
    }

    return (res->type == LispCons_t ? Lisp_Progn(mac, res, fname) : NIL);
}

LispObj *
Lisp_Catch(LispMac *mac, LispObj *list, char *fname)
{
    int did_jump, *pdid_jump = &did_jump;
    LispObj *res, **pres = &res;
    LispBlock *block;

    *pres = NIL;
    *pdid_jump = 1;
    block = LispBeginBlock(mac, CAR(list), LispBlockCatch);
    if (setjmp(block->jmp) == 0) {
	*pres = Lisp_Progn(mac, CDR(list), fname);
	*pdid_jump = 0;
    }
    LispEndBlock(mac, block);
    if (*pdid_jump)
	*pres = mac->block.block_ret;

    return (*pres);
}

LispObj *
Lisp_Coerce(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *from, *to, *res = NIL;
    LispType type = LispNil_t;

    from = CAR(list);
    to = CAR(CDR(list));
    if (to == NIL)
	LispDestroy(mac, "cannot convert %s to nil, at %s",
		    LispStrObj(mac, from), fname);
    else if (to == T)
	return (from);
    else if (!SYMBOL_P(to))
	LispDestroy(mac, "bad argument %s, at %s", LispStrObj(mac, to), fname);
    else {
	if (strcmp(STRPTR(to), "ATOM") == 0 || strcmp(STRPTR(to), "SYMBOL") == 0)
	    type = LispAtom_t;
	else if (strcmp(STRPTR(to), "REAL") == 0)
	    type = LispReal_t;
	else if (strcmp(STRPTR(to), "CONS") == 0)
	    type = LispCons_t;
	else if (strcmp(STRPTR(to), "STRING") == 0)
	    type = LispString_t;
	else if (strcmp(STRPTR(to), "CHARACTER") == 0)
	    type = LispCharacter_t;
	else if (strcmp(STRPTR(to), "OPAQUE") == 0)
	    type = LispOpaque_t;
	else
	    LispDestroy(mac, "invalid type specification %s, at %s",
			STRPTR(to), fname);
    }

    if (from->type == LispOpaque_t) {
	switch (type) {
	    case LispAtom_t:
		res = ATOM(from->data.opaque.data);
		break;
	    case LispString_t:
		res = STRING(from->data.opaque.data);
		break;
	    case LispCharacter_t:
		res = CHAR((int)from->data.opaque.data);
		break;
	    case LispReal_t:
		res = REAL((double)((int)from->data.opaque.data));
		break;
	    case LispOpaque_t:
		res = OPAQUE(from->data.opaque.data, 0);
		break;
	    default:
		LispDestroy(mac, "cannot convert %s to %s, at %s",
			    LispStrObj(mac, from), STRPTR(to), fname);
	}
    }
    else if (from->type != type) {
	if (type == LispString_t)
	    res = Lisp_String(mac, CONS(from, NIL), fname);
	else if (type == LispCharacter_t)
	    res = Lisp_Character(mac, CONS(from, NIL), fname);
	else
	    LispDestroy(mac, "cannot convert %s to %s, at %s",
			LispStrObj(mac, from), STRPTR(to), fname);
    }
    else
	res = from;

    return (res);
}

LispObj *
Lisp_Cdr(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res = NIL;

    switch (CAR(list)->type) {
	case LispNil_t:
	    return (NIL);
	    break;
	case LispCons_t:
	    res = CDR(CAR(list));
	    break;
	default:
	    LispDestroy(mac, ExpectingListAt, fname);
	    /*NOTREACHED*/
    }

    return (res);
}

LispObj *
Lisp_Cond(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *eval, *res = NIL;

    for (; list != NIL; list = CDR(list)) {
	eval = CAR(list);
	if (eval->type != LispCons_t)
	    LispDestroy(mac, "%s is a illegal clause for %s",
			LispStrObj(mac, eval), fname);
	res = EVAL(CAR(eval));
	if (res->type == LispNil_t)
	    continue;
	for (eval = CDR(eval); eval != NIL; eval = CDR(eval))
	    res = EVAL(CAR(eval));
	break;
    }

    return (res);
}

LispObj *
Lisp_Cons(LispMac *mac, LispObj *list, char *fname)
{
    return (CONS(CAR(list), CAR(CDR(list))));
}

LispObj *
Lisp_Decf(LispMac *mac, LispObj *list, char *fname)
{
    double dval;
    LispObj *sym = CAR(list), *num = EVAL(sym),
	    *obj = CDR(list) == NIL ? NIL : CAR(CDR(list)), *val;

    if (!NUMBER_P(num))
	LispDestroy(mac, "expecting number, at %s", fname);
    if (obj != NIL) {
	obj = EVAL(obj);
	if (!NUMBER_P(obj))
	    LispDestroy(mac, "expecting number, at %s", fname);
    }

    dval = NUMBER_VALUE(num) - (obj != NIL ? NUMBER_VALUE(obj) : 1.0);
    val = REAL(dval);

    if (!SYMBOL_P(sym)) {
	/* a bit more complicated, but still faster than
	 *	(setf place (- place 1))
	 * in most cases, could directly change num, but that would cause
	 * side effects, like:
	 *	(setq n '(1))		=> 1
	 *	(setq m (car n))	=> 1
	 *	(decf (car n))		=> 0
	 *	m			=> 0
	 * this should not happen.
	 *
	 *	incf uses the same logic
	 */
	LispObj *frm = FRM;

	obj = CONS(sym, CONS(val, NIL));
	FRM = CONS(obj, FRM);	/* protect setf arguments */
	num = Lisp_Setf(mac, obj, fname);
	frm = FRM;
    }
    else {
	if (obj != NIL)
	    num = _LispSet(mac, sym, val, fname, 0);
	else
	    num = _LispSet(mac, sym, val, fname, 0);
    }

    return (num);
}

LispObj *
Lisp_Defmacro(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispDefLambda(mac, list, LispMacro));
}

LispObj *
Lisp_Defun(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispDefLambda(mac, list, LispFunction));
}

LispObj *
Lisp_Defsetf(LispMac *mac, LispObj *list, char *fname)
{
    if (CDR(CDR(list)) == NIL) {
	LispObj *nam, *val;

	nam = CAR(list);
	list = CDR(list);
	val = CAR(list);
	if (!SYMBOL_P(nam) || !SYMBOL_P(val))
	    LispDestroy(mac, "syntax error at %s %s", fname,
			LispStrObj(mac, nam));

	LispSetAtomSetfProperty(mac, nam->data.atom, val);

	return (nam);
     }

    return (_LispDefLambda(mac, list, LispSetf));
}

LispObj *
Lisp_Do(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispDo(mac, list, fname, 0));
}

LispObj *
Lisp_DoP(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispDo(mac, list, fname, 1));
}

LispObj *
Lisp_DoList(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispDoListTimes(mac, list, fname, 0));
}

LispObj *
Lisp_DoTimes(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispDoListTimes(mac, list, fname, 1));
}

LispObj *
Lisp_Elt(LispMac *mac, LispObj *list, char *fname)
{
    int len, pos;
    LispObj *seq = CAR(list), *opos = CAR(CDR(list)), *res;

    /* if not a sequence, Lisp_Length will see it */
    len = Lisp_Length(mac, list, fname)->data.real;

    if (!INDEX_P(opos) || NUMBER_VALUE(opos) >= len)
	LispDestroy(mac, "bad index %s, at %s", LispStrObj(mac, opos), fname);
    pos = NUMBER_VALUE(opos);

    if (STRING_P(seq))
	res = CHAR(*(unsigned char*)(STRPTR(seq) + pos));
    else {
	if (seq->type == LispArray_t)
	    seq = seq->data.array.list;

	for (; pos > 0; pos--, seq = CDR(seq))
	    ;
	res = CAR(seq);
    }

    return (res);
}

LispObj *
Lisp_Equal(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispEqual(mac, CAR(list), CAR(CDR(list))));
}

LispObj *
Lisp_Error(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *str;

    str = Lisp_Format(mac, CONS(NIL, list), fname);
    LispDestroy(mac, "%s", STRPTR(str));
    /*NOTREACHED*/

    return (NIL);
}

LispObj *
Lisp_Eval(LispMac *mac, LispObj *list, char *fname)
{
    return (EVAL(CAR(list)));
}

LispObj *
Lisp_Evenp(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list);

    if (!INTEGER_P(obj))
	LispDestroy(mac, "expecting integer, at %s", fname);

    return ((int)NUMBER_VALUE(obj) % 2 ? NIL : T);
}

LispObj *
Lisp_FMakunbound(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *atom;

    if (!SYMBOL_P(CAR(list)))
	LispDestroy(mac, "%s is not a symbol, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    atom = CAR(list);

    if (atom->data.atom->property) {
	if (atom->data.atom->property->function)
	    LispRemAtomFunctionProperty(mac, atom->data.atom);
	else if (atom->data.atom->property->builtin)
	    /* not a smart move, just looses a fast function,
	     * and gains no extra memory... */
	    LispRemAtomBuiltinProperty(mac, atom->data.atom);
    }

    return (atom);
}

LispObj *
Lisp_Funcall(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *fun = EVAL(CAR(list));

    return (EVAL(CONS(fun, CDR(list))));
}

LispObj *
Lisp_Get(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *sym, *key, *res;

    if ((sym = CAR(list))->type != LispAtom_t)
	LispDestroy(mac, "expecting symbol, at %s", fname);
    list = CDR(list);
    key = CAR(list);

    res = LispGetAtomProperty(mac, sym->data.atom, key);

    if (res != NIL)
	res = CAR(res);
    else {
	list = CDR(list);
	if (list == NIL)
	    res = NIL;
	else
	    res = CAR(list);
    }

    return (res);
}

LispObj *
Lisp_Gc(LispMac *mac, LispObj *list, char *fname)
{
    LispGC(mac, NIL, NIL);

    return (list == NIL || CAR(list)->type == LispNil_t ? NIL : T);
}

LispObj *
Lisp_Go(LispMac *mac, LispObj *list, char *fname)
{
    unsigned blevel = mac->block.block_level;
    LispObj *tag = CAR(list);

    if (tag != NIL && tag != T && !SYMBOL_P(tag) && !NUMBER_P(tag))
	goto cannot_go;

    while (blevel) {
	LispBlock *block = mac->block.block[--blevel];

	if (block->type == LispBlockClosure)
	    /* if reached a function call */
	    break;
	if (block->type == LispBlockBody) {
	    mac->block.block_ret = tag;
	    LispBlockUnwind(mac);
	    longjmp(block->jmp, 1);
	}
     }

cannot_go:
    LispDestroy(mac, "cannot go to %s, at %s", LispStrObj(mac, tag), fname);

    /*NOTREACHED*/
    return (NIL);
}

LispObj *
Lisp_Incf(LispMac *mac, LispObj *list, char *fname)
{
    double dval;
    LispObj *sym = CAR(list), *num = EVAL(sym),
	    *obj = CDR(list) == NIL ? NIL : CAR(CDR(list)), *val;

    if (!NUMBER_P(num))
	LispDestroy(mac, "expecting number, at %s", fname);
    if (obj != NIL) {
	obj = EVAL(obj);
	if (!NUMBER_P(obj))
	    LispDestroy(mac, "expecting number, at %s", fname);
    }

    dval = NUMBER_VALUE(num) + (obj != NIL ? NUMBER_VALUE(obj) : 1.0);
    val = REAL(dval);

    if (!SYMBOL_P(sym)) {
	LispObj *frm = FRM;

	obj = CONS(sym, CONS(val, NIL));
	FRM = CONS(obj, FRM);	/* protect setf arguments */
	num = Lisp_Setf(mac, obj, fname);
	frm = FRM;
    }
    else {
	if (obj != NIL)
	    num = _LispSet(mac, sym, val, fname, 0);
	else
	    num = _LispSet(mac, sym, val, fname, 0);
    }

    return (num);
}

LispObj *
Lisp_If(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *cond, *res;

    cond = EVAL(CAR(list));
    list = CDR(list);
    if (cond != NIL)
	res = EVAL(CAR(list));
    else {
	if (CDR(list) == NIL)
	    res = NIL;
	else
	    res = EVAL(CAR(CDR(list)));
    }

    return (res);
}

LispObj *
Lisp_Integerp(LispMac *mac, LispObj *list, char *fname)
{
    return (INTEGER_P(CAR(list)) ? T : NIL);
}

LispObj *
Lisp_Lambda(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispDefLambda(mac, list, LispLambda));
}

LispObj *
Lisp_Last(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *seq, *obj;
    int nseq, count;

    if ((seq = CAR(list)) == NIL)
	return (seq);
    else if (seq->type != LispCons_t)
	LispDestroy(mac, ExpectingListAt, fname);

    if (CDR(list) != NIL) {
	obj = CAR(CDR(list));
	if (!INDEX_P(obj))
	    LispDestroy(mac, "bad index %s, at %s", LispStrObj(mac, obj), fname);
	count = NUMBER_VALUE(obj);
    }
    else
	count = 1;

    for (nseq = 0, obj = seq; obj->type == LispCons_t; nseq++, obj = CDR(obj))
	;

    count = nseq - count;

    if (count > nseq)
	return (NIL);
    else if (count <= 0)
	return (seq);

    for (; count > 0; count--, seq = CDR(seq))
	;

    return (seq);
}

LispObj *
Lisp_Length(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj;
    int length = 0;

    obj = CAR(list);
    switch (obj->type) {
	case LispNil_t:
	    break;
	case LispString_t:
	    length = strlen(STRPTR(obj));
	    break;
	case LispArray_t:
	    if (obj->data.array.rank != 1)
		goto notseq;
	    obj = obj->data.array.list;
	    /*FALLTROUGH*/
	case LispCons_t:
	    while (obj->type == LispCons_t) {
		++length;
		obj = CDR(obj);
	    }
	    break;
	default:
notseq:
	    LispDestroy(mac, "%s is not a sequence, at %s",
		    LispStrObj(mac, obj), fname);
	    /*NOTREACHED*/
    }

    return (REAL(length));
}

LispObj *
Lisp_Let(LispMac *mac, LispObj *list, char *fname)
{
    return (LispEnvRun(mac, list, Lisp_Progn, fname, 0));
}

LispObj *
Lisp_LetP(LispMac *mac, LispObj *list, char *fname)
{
    return (LispEnvRun(mac, list, Lisp_Progn, fname, 1));
}

LispObj *
Lisp_List(LispMac *mac, LispObj *list, char *fname)
{
    return (list);
}

LispObj *
Lisp_ListP(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res, *cdr, *obj;

    obj = EVAL(CAR(list));
    if (CDR(list) == NIL)
	return (obj);

    GCProtect();	/* just disable GC, no EVAL's below */
    res = NIL;
    cdr = obj;
    for (list = CDR(list); list != NIL; list = CDR(list)) {
	obj = EVAL(CAR(list));
	if (res == NIL)
	    res = cdr = CONS(cdr, obj);
	else {
	    CDR(cdr) = CONS(CDR(cdr), obj);
	    cdr = CDR(cdr);
	}
    }
    GCUProtect();

    return (res);
}

LispObj *
Lisp_Listp(LispMac *mac, LispObj *list, char *fname)
{
    switch (CAR(list)->type) {
	case LispNil_t:
	case LispCons_t:
	    return (T);
	default:
	    return (NIL);
    }
    /*NOTREACHED*/
}

LispObj *
Lisp_Loop(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj, *res;
    LispBlock *block;

    res = NIL;
    block = LispBeginBlock(mac, NIL, LispBlockTag);
    if (setjmp(block->jmp) == 0) {
	for (;;)
	    for (obj = list; obj != NIL; obj = CDR(obj))
		(void)EVAL(CAR(obj));
    }
    LispEndBlock(mac, block);
    res = mac->block.block_ret;

    return (res);
}

LispObj *
Lisp_Makearray(LispMac *mac, LispObj *list, char *fname)
{
    LispType type = LispNil_t;
    long rank = 0, count = 1, zero, offset, c;
    LispObj *ary = NIL, *dim = NIL, *init, *typ, *cont, *disp, *off, *obj;

    if (NUMBER_P(CAR(list))) {
	if (!INDEX_P(CAR(list)))
	    LispDestroy(mac, "%s is a bad array dimension, at %s",
			LispStrObj(mac, CAR(list)), fname);
	else
	    dim = CONS(CAR(list), NIL);
	rank = 1;
	count = NUMBER_VALUE(CAR(list));
    }
    else if (CAR(list)->type == LispCons_t) {
	dim = CAR(list);

	for (obj = dim, rank = 0; obj != NIL; obj = CDR(obj), ++rank) {
	    if (obj->type != LispCons_t || !INDEX_P(CAR(obj)))
		LispDestroy(mac, "%s is a bad array dimension, at %s",
			    LispStrObj(mac, dim), fname);
		count *= NUMBER_VALUE(CAR(obj));
	}
    }

    offset = -1;
    LispGetKeys(mac, fname,
		"INITIAL-ELEMENT:ELEMENT-TYPE:INITIAL-CONTENTS"
		":DISPLACED-TO:DISPLACED-INDEX-OFFSET",
		CDR(list), &init, &typ, &cont, &disp, &off);

    /* check element-type */
    if (typ != NIL) {
	if (typ == T)
	    type = LispTrue_t;
	else if (!SYMBOL_P(typ))
	    LispDestroy(mac, "unsupported element type %s, at %s",
			LispStrObj(mac, typ), fname);
	else {
	    if (strcmp(STRPTR(typ), "ATOM") == 0)
		type = LispAtom_t;
	    else if (strcmp(STRPTR(typ), "REAL") == 0)
		type = LispReal_t;
	    else if (strcmp(STRPTR(typ), "STRING") == 0)
		type = LispString_t;
	    else if (strcmp(STRPTR(typ), "LIST") == 0)
		type = LispCons_t;
	    else if (strcmp(STRPTR(typ), "OPAQUE") == 0)
		type = LispOpaque_t;
	    else
		LispDestroy(mac, "unsupported element type %s, at %s",
			    STRPTR(typ), fname);
	}
    }

    /* check initial-contents */
    if (cont != NIL && cont->type != LispCons_t)
	LispDestroy(mac, "%s is not a list, at %s",
		    LispStrObj(mac, cont), fname);

    /* check displaced-to */
    if (disp != NIL && disp->type != LispArray_t)
	LispDestroy(mac, "%s is not an array, at %s",
		    LispStrObj(mac, disp), fname);

    /* check displaced-index-offset */
    if (off != NIL) {
	if (!INDEX_P(off))
	    LispDestroy(mac, "%s is a bad :DISPLACED-INDEX-OFFSET, at %s",
			LispStrObj(mac, off), fname);
	offset = (int)NUMBER_VALUE(off);
    }

    c = 0;
    if (init != NIL)
	++c;
    if (cont != NIL)
	++c;
    if (disp != NIL || offset >= 0)
	++c;
    if (c > 1)
	LispDestroy(mac, "more than one initialization specified, at %s",
		    fname);

    zero = count == 0;
    if (disp != NIL) {
	if (offset < 0)
	    offset = 0;
	for (c = 1, obj = disp->data.array.dim; obj != NIL; obj = CDR(obj))
	    c *= (int)NUMBER_VALUE(CAR(obj));
	if (c < count + offset)
	    LispDestroy(mac, "array-total-size + displaced-index-offset "
			"exceeds total size, at %s", fname);
	for (c = 0, ary = disp->data.array.list; c < offset; c++)
	    ary = CDR(ary);
    }
    else if (cont != NIL) {
	if (rank == 1) {
	    for (ary = cont, c = 0; c < count; ary = CDR(ary), ++c)
		if (ary->type != LispCons_t)
		    LispDestroy(mac, "bad argument or size %s, at %s",
				LispStrObj(mac, ary), fname);
	    if (ary != NIL)
		LispDestroy(mac, "bad argument or size %s, at %s",
			    LispStrObj(mac, ary), fname);
	    ary = cont;
	}
	else {
	    LispObj *err = NIL;
	    /* check if list matches */
	    int i, j, k, *dims, *loop;

	    /* create iteration variables */
	    dims = LispMalloc(mac, sizeof(int) * rank);
	    loop = LispCalloc(mac, 1, sizeof(int) * (rank - 1));
	    for (i = 0, obj = dim; obj != NIL; i++, obj = CDR(obj))
		dims[i] = (int)NUMBER_VALUE(CAR(obj));

	    /* check if list matches specified dimensions */
	    while (loop[0] < dims[0]) {
		for (obj = cont, i = 0; i < rank - 1; i++) {
		    for (j = 0; j < loop[i]; j++)
			obj = CDR(obj);
		    err = obj;
		    if ((obj = CAR(obj))->type != LispCons_t)
			goto make_array_error;
		    err = obj;
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
		for (k = 0; k < dims[rank - 1]; obj = CDR(obj), k++) {
		    if (obj->type != LispCons_t)
			goto make_array_error;
		}
		if (obj == NIL)
		    continue;
make_array_error:
		LispFree(mac, dims);
		LispFree(mac, loop);
		LispDestroy(mac, "bad argument or size %s, at %s",
			    LispStrObj(mac, err), fname);
	    }

	    /* list is correct, use it to fill initial values */

	    /* reset loop */
	    memset(loop, 0, sizeof(int) * (rank - 1));

	    GCProtect();
	    /* fill array with supplied values */
	    while (loop[0] < dims[0]) {
		for (obj = cont, i = 0; i < rank - 1; i++) {
		    for (j = 0; j < loop[i]; j++)
			obj = CDR(obj);
		    obj = CAR(obj);
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
		for (k = 0; k < dims[rank - 1]; obj = CDR(obj), k++) {
		    if (ary == NIL)
			ary = CONS(CAR(obj), NIL);
		    else {
			CDR(ary) = CONS(CAR(ary), CDR(ary));
			CAR(ary) = CAR(obj);
		    }
		}
	    }
	    LispFree(mac, dims);
	    LispFree(mac, loop);
	    ary = LispReverse(ary);
	    GCUProtect();
	}
    }
    else {
	GCProtect();
	/* allocate array */
	if (count) {
	    --count;
	    ary = CONS(init, NIL);
	    while (count) {
		CDR(ary) = CONS(CAR(ary), CDR(ary));
		CAR(ary) = init;
		count--;
	    }
	}
	GCUProtect();
    }

    if (type == LispNil_t)
	type = LispTrue_t;
    obj = LispNew(mac, ary, dim);
    obj->type = LispArray_t;
    obj->data.array.list = ary;
    obj->data.array.dim = dim;
    obj->data.array.rank = rank;
    obj->data.array.type = type;	/* XXX ignored */
    obj->data.array.zero = zero;

    return (obj);
}

LispObj *
Lisp_Makelist(LispMac *mac, LispObj *list, char *fname)
{
    int count;
    LispObj *res, *data, *tail;

    if (!INDEX_P(CAR(list)))
	LispDestroy(mac, "%s is not a positive integer, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    count = NUMBER_VALUE(CAR(list));
    LispGetKeys(mac, fname, "INITIAL-ELEMENT", CDR(list), &data);

    GCProtect();
    res = tail = CONS(data, data);
    for (; count > 1; count--)
	res = CONS(data, res);
    CDR(tail) = NIL;
    GCUProtect();

    return (res);
}

LispObj *
Lisp_Makunbound(LispMac *mac, LispObj *list, char *fname)
{
    if (!SYMBOL_P(CAR(list)))
	LispDestroy(mac, "%s is not a symbol, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    LispUnsetVar(mac, CAR(list));

    return (CAR(list));
}

LispObj *
Lisp_Mapcar(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj, *eval, *res, *cdres, *car, *cdr, *ptr, *fun, *frm = FRM;
    int i, level;

    fun = EVAL(CAR(list));
    if (!SYMBOL_P(fun) && fun->type != LispLambda_t)
	LispDestroy(mac, "%s is not a valid function name, at %s",
		    LispStrObj(mac, fun), fname);
    cdres = NIL;
    for (level = 0, res = NIL; ; level++) {
	LispObj *tfrm = FRM;

	eval = cdr = CONS(fun, NIL);
	FRM = CONS(eval, FRM);	/* protect eval, as there is n EVAL's below */
	for (ptr = CDR(list); ptr != NIL; ptr = CDR(ptr)) {
	    car = EVAL(CAR(ptr));
	    if (car->type != LispCons_t)
		goto mapcar_done;
	    for (i = 0, obj = car; i < level; i++) {
		if ((obj = CDR(obj)) == NIL)
		    goto mapcar_done;
	    }
	    /* quote back to avoid double eval */
	    GCProtect();
	    car = QUOTE(CAR(obj));
	    CDR(cdr) = CONS(car, NIL);
	    cdr = CDR(cdr);
	    GCUProtect();
	}
	obj = EVAL(eval);
	FRM = tfrm;
	if (res == NIL) {
	    GCProtect();
	    res = cdres = CONS(obj, NIL);
	    FRM = CONS(res, FRM);	/* protect res linking to FRM */
	    GCUProtect();
	}
	else {
	    CDR(cdres) = CONS(obj, NIL);
	    cdres = CDR(cdres);
	}
    }

    FRM = frm;				/* no need for GC protection now */

    /* to be CL compatible */
mapcar_done:
    return (res);
}

LispObj *
Lisp_Max(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispMinMax(mac, list, fname, 1));
}

LispObj *
Lisp_Member(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list), *res = NIL;

    list = CAR(CDR(list));
    if (list->type == LispNil_t)
	return (NIL);
    else if (list->type != LispCons_t)
	LispDestroy(mac, ExpectingListAt, fname);

    for (; list != NIL; list = CDR(list))
	if (_LispEqual(mac, obj, CAR(list)) == T) {
	    res = list;
	    break;
	}

    return (res);
}

LispObj *
Lisp_Min(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispMinMax(mac, list, fname, 0));
}

LispObj *
Lisp_Minusp(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list);

    if (!NUMBER_P(obj))
	LispDestroy(mac, "expecting number, at %s", fname);

    return (NUMBER_VALUE(obj) < 0.0 ? T : NIL);
}

LispObj *
Lisp_Nth(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispNth(mac, list, fname, 0));
}

LispObj *
Lisp_Nthcdr(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispNth(mac, list, fname, 1));
}

LispObj *
Lisp_Null(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res = NIL;

    if (list != NIL && CAR(list)->type == LispNil_t)
	res = T;
    return (res);
}

LispObj *
Lisp_Numberp(LispMac *mac, LispObj *list, char *fname)
{
    return (NUMBER_P(CAR(list)) ? T : NIL);
}

LispObj *
Lisp_Oddp(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list);

    if (!INTEGER_P(obj))
	LispDestroy(mac, "expecting integer, at %s", fname);

    return ((int)NUMBER_VALUE(obj) % 2 ? T : NIL);
}

LispObj *
Lisp_Or(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res = NIL;

    for (; list != NIL; list = CDR(list)) {
	if ((res = EVAL(CAR(list))) != NIL)
	    break;
    }
    return (res);
}

LispObj *
Lisp_Plusp(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list);

    if (!NUMBER_P(obj))
	LispDestroy(mac, "expecting number, at %s", fname);

    return (NUMBER_VALUE(obj) > 0.0 ? T : NIL);
}

LispObj *
Lisp_Prin1(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list), *stream;

    list = CDR(list);
    if (list == NIL || CAR(list) == NIL)
	stream = NIL;
    else
	stream = CAR(list);

    LispPrint(mac, obj, stream, 0);

    return (obj);
}

LispObj *
Lisp_Princ(LispMac *mac, LispObj *list, char *fname)
{
    int princ = mac->princ;
    LispObj *obj = CAR(list), *stream;

    list = CDR(list);
    if (list == NIL || CAR(list) == NIL)
	stream = NIL;
    else
	stream = CAR(list);

    mac->princ = 1;
    LispPrint(mac, obj, stream, 0);
    mac->princ = princ;

    return (obj);
}

LispObj *
Lisp_Print(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list), *stream;

    list = CDR(list);
    if (list == NIL || CAR(list) == NIL)
	stream = NIL;
    else
	stream = CAR(list);

    LispPrint(mac, obj, stream, 1);

    return (obj);
}

LispObj *
Lisp_Prog1(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *frm, *res = EVAL(CAR(list));

    frm = FRM;
    FRM = CONS(res, FRM);
    for (list = CDR(list); list != NIL; list = CDR(list))
	(void)EVAL(CAR(list));
    FRM = frm;

    return (res);
}

LispObj *
Lisp_Prog2(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *frm, *res;

    (void)EVAL(CAR(list));
    list = CDR(list);
    res = EVAL(CAR(list));
    frm = FRM;
    FRM = CONS(res, FRM);
    for (list = CDR(list); list != NIL; list = CDR(list))
	(void)EVAL(CAR(list));
    FRM = frm;

    return (res);
}

LispObj *
Lisp_Progn(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res = NIL;

    for (; list != NIL; list = CDR(list))
	res = EVAL(CAR(list));

    return (res);
}

LispObj *
Lisp_Progv(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *old_frm, *old_env, *res, *cons = NIL, *valist = NIL;
    LispObj *syms, *values, *body;

    old_frm = FRM;

    /* get symbol names */
    syms = EVAL(CAR(list));
    GCProtect();
    FRM = CONS(syms, FRM);
    GCUProtect();

    /* get symbol values */
    list = CDR(list);
    values = EVAL(CAR(list));
    GCProtect();
    FRM = CONS(values, FRM);
    GCUProtect();
    list = CDR(list);

    /* the body to be executed */
    body = list;

    /* fill variable list */
    for (; syms->type == LispCons_t; syms = CDR(syms)) {
	if (values->type != LispCons_t)
	    break;
	if (!SYMBOL_P(CAR(syms)))
	    LispDestroy(mac, "%s is not a symbol, at %s",
			LispStrObj(mac, CAR(syms)), fname);
	if (valist == NIL) {
	    GCProtect();
	    valist = cons = CONS(CONS(CAR(syms), CAR(values)), NIL);
	    FRM = CONS(valist, FRM);
	    GCUProtect();
	}
	else {
	    CDR(cons) = CONS(CONS(CAR(syms), CAR(values)), NIL);
	    cons = CDR(cons);
	}
	values = CDR(values);
    }

    /* add variables */
    old_env = ENV;
    for (; valist != NIL; valist = CDR(valist)) {
	cons = CAR(valist);
	LispAddVar(mac, CAR(cons), CDR(cons));
    }

    res = Lisp_Progn(mac, body, fname);

    ENV = old_env;
    FRM = old_frm;

    return (res);
}

LispObj *
Lisp_Provide(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *feat = CAR(list), *obj;

    if (!STRING_P(feat) && !SYMBOL_P(feat))
	LispDestroy(mac, "cannot provide %s", LispStrObj(mac, feat));

    for (obj = MOD; obj != NIL; obj = CDR(obj)) {
	if (STRPTR(CAR(obj)) == STRPTR(feat))
	    return (feat);
    }

    if (MOD == NIL)
	MOD = CONS(feat, NIL);
    else {
	CDR(MOD) = CONS(CAR(MOD), CDR(MOD));
	CAR(MOD) = feat;
    }

    return (feat);
}

LispObj *
Lisp_Quit(LispMac *mac, LispObj *list, char *fname)
{
    int status = 0;

    if (list != NIL) {
	if (!INTEGER_P(CAR(list)))
	    LispDestroy(mac, "bad exit status argument %s, at %s",
			LispStrObj(mac, CAR(list)), fname);
	status = (int)NUMBER_VALUE(CAR(list));
    }

    exit(status);
}

LispObj *
Lisp_Quote(LispMac *mac, LispObj *list, char *fname)
{
    return (CAR(list));
}

/* XXX needs to be extended to accept parameters */
LispObj *
Lisp_Read(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj;

    obj = LispRun(mac);
    if (obj == EOLIST)
	LispDestroy(mac, "object cannot start with #\\)");

    return (obj);
}

/* Destructively replaces seq1 elements with seq2 elements
 *	(replace sequence1 sequence2 &key :start1 :end1 :start2 :end2)
 */
LispObj *
Lisp_Replace(LispMac *mac, LispObj *list, char *fname)
{
    int len, len1, len2, start1, end1, start2, end2;
    LispObj *seq1, *seq2, *ostart1, *oend1, *ostart2, *oend2;

    seq1 = CAR(list);
    len1 = Lisp_Length(mac, list, fname)->data.real;

    list = CDR(list);
    seq2 = CAR(list);
    len2 = Lisp_Length(mac, list, fname)->data.real;

    list = CDR(list);
    LispGetKeys(mac, fname, "START1:END1:START2:END2", list,
		&ostart1, &oend1, &ostart2, &oend2);

    if (ostart1 == NIL)
	start1 = 0;
    else if (INDEX_P(ostart1))
	goto replace_not_integer;
    else
	start1 = NUMBER_VALUE(ostart1);
    if (oend1 == NIL)
	end1 = len1;
    else if (INDEX_P(oend1))
	goto replace_not_integer;
    else
	end1 = NUMBER_VALUE(oend1);

    if (ostart2 == NIL)
	start2 = 0;
    else if (INDEX_P(ostart2))
	goto replace_not_integer;
    else
	start2 = NUMBER_VALUE(ostart2);
    if (oend2 == NIL)
	end2 = len2;
    else if (INDEX_P(oend2))
	goto replace_not_integer;
    else
	end2 = NUMBER_VALUE(oend2);

    if (start1 == end1 || start2 == end2)
	goto replace_done;

    if (start1 > end1 || end1 > len1 || start2 > end2 || end2 > len2)
	goto replace_out_of_range;

    len = end1 - start1;
    if (len > end2 - start2)
	len = end2 - start2;

    if (STRING_P(seq1)) {
	char *string;

	if (!STRING_P(seq2))
	    goto replace_type_mismatch;

	string = LispStrdup(mac, STRPTR(seq1));
	strncpy(string + start1, STRPTR(seq2) + start2, len);
	seq1->data.atom = LispDoGetAtom(mac, string, 0, 0);
	LispFree(mac, string);
    }
    else {
	int i;
	LispObj *from, *to;

	if (seq1->type == LispArray_t)
	    seq1 = seq1->data.array.list;
	if (seq2->type == LispArray_t)
	    seq2 = seq2->data.array.list;

	/* adjust pointers */
	for (i = 0, from = seq2; i < start2; i++, from = CDR(from))
	    ;
	for (i = 0, to = seq1; i < start1; i++, to = CDR(to))
	    ;

	/* copy data */
	for (i = 0; i < len; i++, from = CDR(from), to = CDR(to))
	    CAR(to) = CAR(from);
    }

    goto replace_done;

replace_not_integer:
    LispDestroy(mac, "expecting a positive integer, at %s", fname);

replace_out_of_range:
    LispDestroy(mac, "index out of range, at %s", fname);

replace_type_mismatch:
    LispDestroy(mac, "sequences type don't match, at %s", fname);

replace_done:
    return (seq1);
}

LispObj *
Lisp_Return(LispMac *mac, LispObj *list, char *fname)
{
    unsigned blevel = mac->block.block_level;

    while (blevel) {
	LispBlock *block = mac->block.block[--blevel];

	if (block->type == LispBlockClosure)
	    /* if reached a function call */
	    break;
	if (block->type == LispBlockTag && block->tag.type == LispNil_t) {
	    mac->block.block_ret = list == NIL ? NIL : EVAL(CAR(list));
	    LispBlockUnwind(mac);
	    longjmp(block->jmp, 1);
	}
    }
    LispDestroy(mac, "no visible NIL block, at %s", fname);
    /*NOTREACHED*/

    return (NIL);
}

LispObj *
Lisp_ReturnFrom(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *tag = CAR(list);
    unsigned blevel = mac->block.block_level;

    if (tag != NIL && tag != T && !SYMBOL_P(tag))
	LispDestroy(mac, "%s is not a symbol, at %s",
		    LispStrObj(mac, tag), fname);

    list = CDR(list);
    while (blevel) {
	int jmp = 1;
	LispBlock *block = mac->block.block[--blevel];

	if (tag->type == block->tag.type) {
	    switch (tag->type) {
		case LispNil_t:
		case LispTrue_t:
		    break;
		case LispAtom_t:
		    jmp = tag->data.atom == block->tag.data.atom;
		    break;
		default:
		    /* only atom, nil or t can be used */
		    jmp = 0;
		    break;
	    }
	}
	if (block->type != LispBlockTag && block->type != LispBlockClosure)
	    break;
	if (jmp) {
	    mac->block.block_ret = list == NIL ? NIL : EVAL(CAR(list));
	    LispBlockUnwind(mac);
	    longjmp(block->jmp, 1);
	}
	if (block->type != LispBlockTag)
	    /* can use return-from only in the current function */
	    break;
    }
    LispDestroy(mac, "no visible block named %s, at %s",
		LispStrObj(mac, tag), fname);
    /*NOTREACHED*/

    return (NIL);
}

LispObj *
Lisp_Reverse(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res;

    switch (CAR(list)->type) {
	case LispNil_t:
	    return (NIL);
	    break;
	case LispCons_t:
	    break;
	default:
	    LispDestroy(mac, ExpectingListAt, fname);
	    /*NOTREACHED*/
    }

    GCProtect();
    res = NIL;
    list = CAR(list);
    while (list->type == LispCons_t && list != NIL) {
	res = CONS(CAR(list), res);
	list = CDR(list);
    }
    GCUProtect();

    return (res);
}

LispObj *
Lisp_Rplaca(LispMac *mac, LispObj *list, char *fname)
{
    if (CAR(list)->type != LispCons_t)
	LispDestroy(mac, "%s is not of type cons, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    CAR(CAR(list)) = CAR(CDR(list));

    return (CAR(list));
}

LispObj *
Lisp_Rplacd(LispMac *mac, LispObj *list, char *fname)
{
    if (CAR(list)->type != LispCons_t)
	LispDestroy(mac, "%s is not of type cons, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    CDR(CAR(list)) = CAR(CDR(list));

    return (CAR(list));
}

LispObj *
Lisp_Set(LispMac *mac, LispObj *list, char *fname)
{
    int count;
    LispObj *var, *val, *res = NIL;
			/* res always set, is minimum args is 2 */

    for (count = 0, var = list; var != NIL; count++, var = CDR(var))
	;
    if (count & 1)
	LispDestroy(mac, "odd number of arguments, at %s", fname);

    for (var = CAR(list), val = CAR(CDR(list)); list != NIL;
	 list = CDR(CDR(list)))
	res = _LispSet(mac, CAR(list), CAR(CDR(list)), fname, 0);

    return (res);
}

LispObj *
Lisp_SetQ(LispMac *mac, LispObj *list, char *fname)
{
    int count;
    LispObj *var, *val, *res = NIL;
			/* res always set, is minimum args is 2 */

    for (count = 0, var = list; var != NIL; count++, var = CDR(var))
	;
    if (count & 1)
	LispDestroy(mac, "odd number of arguments, at %s", fname);

    for (var = CAR(list), val = CAR(CDR(list)); list != NIL;
	 list = CDR(CDR(list)))
	res = _LispSet(mac, CAR(list), CAR(CDR(list)), fname, 1);

    return (res);
}

LispObj *
Lisp_Setf(LispMac *mac, LispObj *list, char *fname)
{
    int count;
    LispAtom *atom;
    LispObj *place, *setf, *res = NIL;

    for (count = 0, place = list; place != NIL; count++, place = CDR(place))
	;
    if (count & 1)
	LispDestroy(mac, "odd number of arguments, at %s", fname);

    for (place = CAR(list), list = CDR(list);
	 ; place = CAR(list), list = CDR(list)) {
	/* if a variable, just work like setq */
	if (SYMBOL_P(place))
	    res = _LispSet(mac, place, CAR(list), fname, 1);
	else if (place->type == LispCons_t) {
	    int struc_access = 0;

	    /* the default setf method for structures is generated here
	     * (cannot be done in EVAL as SETF is a macro), and the
	     * code executed is as if this definition were supplied:
	     *	(defsetf THE-STRUCT-FIELD (struct) (value)
	     *		`(xedit::struct-store 'THE-STRUCT-FIELD ,struct ,value))
	     */

	    setf = CAR(place);
	    if (!SYMBOL_P(setf) || setf->data.atom->property == NULL)
		LispDestroy(mac, "%s is a invalid %s place",
			    LispStrObj(mac, place), fname);

	    atom = setf->data.atom;

	    if (atom->property->defsetf == 0) {
		if (atom->property->defstruct &&
		    atom->property->structure.function >= 0) {
		    /* user didn't provide any special defsetf */
		    setf = ATOM("XEDIT::STRUCT-STORE");
		    struc_access = 1;
		}
		else
		    LispDestroy(mac, "%s is a invalid %s place",
				LispStrObj(mac, place), fname);
	    }
	    else
		setf = setf->data.atom->property->setf;

	    if (SYMBOL_P(setf)) {
		/* just change function call, and append value to arguments */
		LispObj *cod, *cdr, *obj, *frm = FRM;

		GCProtect();
		cod = cdr = CONS(setf, NIL);
		FRM = CONS(cod, FRM);
		GCUProtect();

		if (struc_access) {
		    /* using builtin setf method for structure field */
		    CDR(cdr) = CONS(QUOTE(CAR(place)), NIL);
		    cdr = CDR(cdr);
		}

		for (obj = CDR(place); obj != NIL; obj = CDR(obj)) {
		    CDR(cdr) = CONS(CAR(obj), NIL);
		    cdr = CDR(cdr);
		}
		CDR(cdr) = CONS(CAR(list), NIL);
		res = EVAL(cod);
		frm = FRM;
	    }
	    else
		res = LispRunSetf(mac, setf, place, CAR(list));

	}
	else
	    LispDestroy(mac, "%s is not a %s place",
			LispStrObj(mac, place), fname);

	if ((list = CDR(list)) == NIL)
	    break;
    }

    return (res);
}

LispObj *
Lisp_Stringp(LispMac *mac, LispObj *list, char *fname)
{
    return (STRING_P(CAR(list)) ? T : NIL);
}

LispObj *
Lisp_Subseq(LispMac *mac, LispObj *list, char *fname)
{
    int start, end, length, sublen;
    LispObj *seq, *ostart, *oend, *res;

    seq = CAR(list);
    if (seq != NIL && seq->type != LispCons_t &&
	!STRING_P(seq) && seq->type != LispArray_t)
	LispDestroy(mac, "%s is not a sequence, at %s",
		    LispStrObj(mac, seq), fname);
    length = Lisp_Length(mac, list, fname)->data.real;

    list = CDR(list);
    ostart = CAR(list);
    list = CDR(list);
    oend = list == NIL ? NIL : CAR(list);
    if (!INDEX_P(ostart) || (oend != NIL && !INDEX_P(oend)))
	LispDestroy(mac, "expecting positive integer, at %s", fname);

    start = NUMBER_VALUE(ostart);

    if (oend != NIL)
	end = NUMBER_VALUE(oend);
    else
	end = length;

    if (start > end || end > length)
	LispDestroy(mac, "bad index, at %s", fname);

    sublen = end - start;

    if (seq == NIL)
	res = NIL;
    else if (STRING_P(seq)) {
	char *str = LispMalloc(mac, sublen + 1);

	strncpy(str, STRPTR(seq) + start, sublen);
	str[sublen] = '\0';
	res = STRING(str);
	LispFree(mac, str);
    }
    else {
	LispObj *obj;

	GCProtect();
	if (end > start) {
	    /* list or array */
	    int count;
	    LispObj *cdr;

	    if (seq->type == LispArray_t)
		obj = seq->data.array.list;
	    else
		obj = seq;
	    /* goto first element to copy */
	    for (count = 0; count < start; count++, obj = CDR(obj))
		;
	    res = cdr = CONS(CAR(obj), NIL);
	    for (++count, obj = CDR(obj); count < end; count++, obj = CDR(obj)) {
		CDR(cdr) = CONS(CAR(obj), NIL);
		cdr = CDR(cdr);
	    }
	}
	else
	    res = NIL;

	if (seq->type == LispArray_t) {
	    obj = LispNew(mac, res, NIL);
	    obj->type = LispArray_t;
	    obj->data.array.list = res;
	    obj->data.array.dim = CONS(REAL(sublen), NIL);
	    obj->data.array.rank = 1;
	    obj->data.array.type = seq->data.array.type;
	    obj->data.array.zero = length == 0;
	    res = obj;
	}
	GCUProtect();
    }

    return (res);
}

LispObj *
Lisp_Symbolp(LispMac *mac, LispObj *list, char *fname)
{
    switch (CAR(list)->type) {
	case LispNil_t:
	case LispTrue_t:
	case LispAtom_t:
	case LispLambda_t:
	    return (T);
	default:
	    return (NIL);
    }
    /*NOTREACHED*/
}

LispObj *
Lisp_SymbolPlist(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *sym = CAR(list);

    if (sym == NIL || sym == T)
	return (sym);
    else if (!SYMBOL_P(sym))
	LispDestroy(mac, "%s is not a symbol, at %s",
		    LispStrObj(mac, sym), fname);

    return (sym->data.atom->property && sym->data.atom->property->property ?
	    sym->data.atom->property->properties : NIL);
}

LispObj *
Lisp_Tagbody(LispMac *mac, LispObj *list, char *fname)
{
    int did_jump, *pdid_jump = &did_jump, body_jump, *pbody_jump = &body_jump;
    LispObj * volatile body;
    LispObj *res, **pres = &res;
    LispBlock *block, *body_block;

    for (body = list; body != NIL; body = CDR(body))
	if (body->type == LispCons_t)
	    break;

    if (body == NIL)
	return (NIL);

    *pdid_jump = 1;
    *pres = NIL;
    block = LispBeginBlock(mac, NIL, LispBlockTag);
    if (setjmp(block->jmp) == 0) {
	body = list;
	while (1) {
	    *pbody_jump = 1;
	    body_block = LispBeginBlock(mac, NIL, LispBlockBody);
	    if (setjmp(body_block->jmp) == 0) {
		for (; body != NIL; body = CDR(body)) {
		    if (CAR(body)->type == LispCons_t)
			*pres = EVAL(CAR(body));
		}
		*pbody_jump = 0;
	    }
	    LispEndBlock(mac, body_block);
	    if (*pbody_jump) {
		int found = 0;
		LispObj *ptr, *tag;

		tag = mac->block.block_ret;
		for (ptr = body; ptr != NIL; ptr = CDR(ptr)) {
		    if (CAR(ptr)->type == tag->type &&
			((CAR(ptr) == NIL && tag->type == LispNil_t) ||
			 (CAR(ptr) == T && tag->type == LispTrue_t) ||
			 (NUMBER_P(ptr) && NUMBER_P(tag) &&
			  NUMBER_VALUE(ptr) == NUMBER_VALUE(tag)) ||
			 (SYMBOL_P(CAR(ptr)) && SYMBOL_P(tag) &&
			  CAR(ptr)->data.atom == tag->data.atom))) {
			found = 1;
			break;
		    }
		}
		if (ptr == NIL) {
		    for (ptr = list; ptr != body; ptr = CDR(ptr)) {
			if (CAR(ptr)->type == tag->type &&
			    ((CAR(ptr) == NIL && tag->type == LispNil_t) ||
			     (CAR(ptr) == T && tag->type == LispTrue_t) ||
			     (NUMBER_P(ptr) && NUMBER_P(tag) &&
			      NUMBER_VALUE(ptr) == NUMBER_VALUE(tag)) ||
			     (SYMBOL_P(CAR(ptr)) && SYMBOL_P(tag) &&
			      CAR(ptr)->data.atom == tag->data.atom))) {
			    found = 1;
			    break;
			}
		    }
		}
		/* XXX no search for duplicated tags, if there are
		 * duplicated tags, will just search the body for the tag,
		 * if the end of the list is reached, search again from
		 * beginning. This is (I believe) allowable for an interpreter,
		 * but if (byte) compiled code is to be generated, duplicated
		 * tags must not be allowed. */
		if ((body = ptr) == NIL)
		    LispDestroy(mac, "no such tag %s, at %s",
				LispStrObj(mac, tag), fname);

		/* search for start of code */
		for (body = CDR(body); body != NIL; body = CDR(body)) {
		    if (CAR(body)->type == LispCons_t)
			break;
		}

		/* just jumped to the bottom of the code body */
		if (body == NIL)
		    break;
	    }
	    else
		/* 'go' not called */
		break;
	    *pdid_jump = 1;
	}
	*pdid_jump = 0;
    }
    LispEndBlock(mac, block);
    if (*pdid_jump)
	*pres = mac->block.block_ret;

    return (*pres);
}

LispObj *
Lisp_Terpri(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *stream = NIL;

    if (list == NIL || CAR(list) == NIL)
	stream = NIL;
    else if (CAR(list)->type != LispStream_t)
	LispDestroy(mac, "%s is not a stream, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    else
	stream = CAR(list);
    LispPrintf(mac, stream, "\n");
    mac->newline = 1;
    mac->column = 0;
    fflush(lisp_stdout);

    return (NIL);
}

LispObj *
Lisp_Throw(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *tag = EVAL(CAR(list));
    unsigned blevel = mac->block.block_level;

    if (blevel == 0)
	LispDestroy(mac, "%s called not within a block", fname);

    while (blevel) {
	int jmp = 1;
	LispBlock *block = mac->block.block[--blevel];

	if (block->type == LispBlockCatch && tag->type == block->tag.type) {
	    switch(tag->type) {
		case LispNil_t:
		case LispTrue_t:
		    break;
		case LispAtom_t:
		case LispString_t:
		    jmp = tag->data.atom == block->tag.data.atom;
		    break;
		case LispCharacter_t:
		case LispInteger_t:
		    jmp = tag->data.integer == block->tag.data.integer;
		    break;
		case LispReal_t:
		    jmp = tag->data.real == block->tag.data.real;
		    break;
		default:
		    jmp = memcmp(tag, &(block->tag), sizeof(LispObj)) == 0;
		    break;
	    }
	    if (jmp) {
		mac->block.block_ret = EVAL(CAR(CDR(list)));
		LispBlockUnwind(mac);
		longjmp(block->jmp, 1);
	    }
	}
    }
    LispDestroy(mac, "%s is not a tag to %s", LispStrObj(mac, tag), fname);
    /*NOTREACHED*/

    return (NIL);
}

LispObj *
Lisp_Typep(LispMac *mac, LispObj *list, char *fname)
{
    LispType type = LispStruct_t;
    LispObj *obj;
    char *atom = NULL;

    obj = CAR(CDR(list));
    if (obj == NIL || obj == T)
	return (obj);
    else if (!SYMBOL_P(obj))
	LispDestroy(mac, "%s is a bad type specification, at %s",
		    LispStrObj(mac, obj), fname);
    else {
	atom = STRPTR(obj);
	if (strcmp(atom, "ATOM") == 0)
	    type = LispAtom_t;
	else if (strcmp(atom, "REAL") == 0)
	    type = LispReal_t;
	else if (strcmp(atom, "LIST") == 0)
	    type = LispCons_t;
	else if (strcmp(atom, "STRING") == 0)
	    type = LispString_t;
	else if (strcmp(atom, "OPAQUE") == 0)
	    type = LispOpaque_t;
    }

    obj = CAR(list);
    if (type != LispStruct_t && obj->type == type)
	return (T);
    else if (obj->type == LispStruct_t)
	return (STRPTR(CAR(obj->data.struc.def)) == atom ? T : NIL);

    return (NIL);
}

LispObj *
Lisp_Unless(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispWhenUnless(mac, list, 0));
}

LispObj *
Lisp_UnwindProtect(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *prot = CAR(list), *res, **pres = &res;
    int did_jump, *pdid_jump = &did_jump;
    LispBlock *block;

    /* run protected code */
    *pres = NIL;
    *pdid_jump = 1;
    block = LispBeginBlock(mac, NIL, LispBlockProtect);
    if (setjmp(block->jmp) == 0) {
	*pres = EVAL(prot);
	*pdid_jump = 0;
    }
    LispEndBlock(mac, block);
    if (!mac->destroyed && *pdid_jump)
	*pres = mac->block.block_ret;

    /* run cleanup, unprotected code */
    if (CDR(list) != NIL)
	res = Lisp_Progn(mac, CDR(list), fname);
    else if (mac->destroyed)
	/* no cleanup code */
	LispDestroy(mac, NULL);	/* special handling if mac->destroyed */

    return (res);
}

LispObj *
Lisp_Vector(LispMac *mac, LispObj *list, char *fname)
{
    int count;
    LispObj *dim, *ary = list, *obj;

    for (count = 0; list != NIL; count++, list = CDR(list))
	;
    dim = CONS(REAL((double)count), NIL);

    obj = LispNew(mac, ary, dim); /* no need to gc protect, as dim is argument*/
    obj->type = LispArray_t;
    obj->data.array.list = ary;
    obj->data.array.dim = dim;
    obj->data.array.rank = 1;
    obj->data.array.type = LispTrue_t;
    obj->data.array.zero = count == 0;

    return (obj);
}

LispObj *
Lisp_When(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispWhenUnless(mac, list, 1));
}

LispObj *
Lisp_Until(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispWhileUntil(mac, list, 0));
}

LispObj *
Lisp_While(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispWhileUntil(mac, list, 1));
}

/* helper functions for setf
 *	DONT explicitly call these functions. Non standard functions
 */
LispObj *
Lisp_XeditEltStore(LispMac *mac, LispObj *list, char *fname)
{
    int len, pos;
    LispObj *seq, *opos, *value;

    seq = CAR(list);
    /* if not a sequence, Lisp_Length will see it */
    len = Lisp_Length(mac, list, fname)->data.real;

    list = CDR(list);
    opos = CAR(list);
    if (!INDEX_P(opos) || NUMBER_VALUE(opos) >= len)
	LispDestroy(mac, "bad index %s, at %s", LispStrObj(mac, opos), fname);
    pos = NUMBER_VALUE(opos);

    list = CDR(list);
    value = CAR(list);
    if (STRING_P(seq)) {
	int c;
	char *string;

	if (value->type != LispCharacter_t)
	    LispDestroy(mac, "%s is not a character, at %s",
			LispStrObj(mac, value), fname);

	c = value->data.integer;
	if (c < 0 || c > 255)
	    LispDestroy(mac, "cannot represent character %d, at %s", c, fname);

	string = LispStrdup(mac, STRPTR(seq));
	string[pos] = c;

	seq->data.atom = LispDoGetAtom(mac, string, 0, 0);
	LispFree(mac, string);
    }
    else {
	if (seq->type == LispArray_t)
	    seq = seq->data.array.list;

	for (; pos > 0; pos--, seq = CDR(seq))
	    ;
	CAR(seq) = value;
    }

    return (value);
}

LispObj *
Lisp_XeditPut(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *sym, *key, *val;

    if ((sym = CAR(list))->type != LispAtom_t)
	LispDestroy(mac, "expecting symbol, at %s", fname);
    list = CDR(list);
    key = CAR(list);
    list = CDR(list);
    val = CAR(list);

    return (CAR(LispPutAtomProperty(mac, sym->data.atom, key, val)));
}

LispObj *
Lisp_XeditVectorStore(LispMac *mac, LispObj *list, char *fname)
{
    long c, count, idx, seq;
    LispObj *ary = CAR(list), *dim = CDR(list), *obj;

    if (ary->type != LispArray_t)
	LispDestroy(mac, "%s is not an array, at %s",
		    LispStrObj(mac, ary), fname);

    for (count = 0, list = dim, obj = ary->data.array.dim; CDR(list) != NIL;
	 count++, list = CDR(list), obj = CDR(obj)) {
	if (count >= ary->data.array.rank)
	    LispDestroy(mac, "too many subscripts %s, at %s",
			LispStrObj(mac, dim), fname);
	if (!INDEX_P(CAR(list)) ||
	    NUMBER_VALUE(CAR(list)) >= NUMBER_VALUE(CAR(obj)))
	    LispDestroy(mac, "%s is out of range or a bad index, at %s",
			LispStrObj(mac, CAR(list)), fname);
    }
    if (count < ary->data.array.rank)
	LispDestroy(mac, "too few subscripts %s, at %s",
		    LispStrObj(mac, dim), fname);

    for (count = seq = 0, list = dim; CDR(list) != NIL;
	 list = CDR(list), seq++) {
	for (idx = 0, obj = ary->data.array.dim; idx < seq; obj = CDR(obj), ++idx)
	    ;
	for (c = 1, obj = CDR(obj); obj != NIL; obj = CDR(obj))
	    c *= NUMBER_VALUE(CAR(obj));
	count += c * NUMBER_VALUE(CAR(list));
    }

    for (ary = ary->data.array.list; count > 0; ary = CDR(ary), count--)
	;

    CAR(ary) = CAR(list);

    return (CAR(list));
}

LispObj *
Lisp_Zerop(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list);

    if (!NUMBER_P(obj))
	LispDestroy(mac, "expecting number, at %s", fname);

    return (NUMBER_VALUE(obj) == 0 ? T : NIL);
}

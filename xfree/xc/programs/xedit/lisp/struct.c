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

/* $XFree86: xc/programs/xedit/lisp/struct.c,v 1.5 2001/10/28 03:34:31 tsi Exp $ */

#include "struct.h"

/*
 * Implementation
 */
LispObj *
Lisp_Defstruct(LispMac *mac, LispObj *list, char *fname)
{
    LispAtom *atom;
    int i, sz, len, slen;
    char *name, *strname, *sname;
    LispObj *str = list, *obj;

    /* get structure name */
    if (!SYMBOL_P(CAR(list)) ||
	/* reserved name(s) */
	strcmp(STRPTR(CAR(list)), "ARRAY") == 0)
	LispDestroy(mac, "%s cannot be a structure name, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    strname = STRPTR(CAR(list));
    len = strlen(strname);

    /* get structure fields and default values */
    for (list = CDR(list); list != NIL; list = CDR(list)) {
	if ((!SYMBOL_P(CAR(list)) &&
	     /* if not field name, with NIL as default value */
	     (CAR(list)->type != LispCons_t ||
	      !SYMBOL_P(CAR(CAR(list))))) ||
	    /* and not a pair, with field name and default value */
	    STRPTR(CAR(list))[0] == ':' ||
	    /* and it is a valid field name */
	    strcmp(STRPTR(CAR(list)), "P") == 0)
	    /* p is invalid as a field name due to `type'-p */
	    LispDestroy(mac, "%s cannot be a field for %s, at %s",
			LispStrObj(mac, CAR(list)), STRPTR(CAR(str)), fname);

	/* check for repeated field names */
	for (obj = CDR(str); obj != list; obj = CDR(obj)) {
	    if (STRPTR(CAR(obj)) == STRPTR(CAR(list)))
		LispDestroy(mac, "only one slot named :%s allowed, at %s",
			    LispStrObj(mac, CAR(obj)), fname);
	}
    }

    /* XXX any memory allocation failure below should be a fatal error */

	    /* MAKE- */
    sz = len + 6;
    name = LispMalloc(mac, sz);

    sprintf(name, "MAKE-%s", strname);
    atom = ATOM(name)->data.atom;
    LispSetAtomStructProperty(mac, atom, str, STRUCT_CONSTRUCTOR);

    sprintf(name, "%s-P", strname);
    atom = ATOM(name)->data.atom;
    LispSetAtomStructProperty(mac, atom, str, STRUCT_CHECK);

    for (i = 0, list = CDR(str); list != NIL; i++, list = CDR(list)) {
	sname = STRPTR(CAR(list));
	slen = strlen(sname);
	if (len + slen + 2 > sz) {
	    sz = len + slen + 2;
	    name = LispRealloc(mac, name, sz);
	}
	sprintf(name, "%s-%s", strname, sname);
	atom = ATOM(name)->data.atom;
	LispSetAtomStructProperty(mac, atom, str, i);
    }

    LispFree(mac, name);

    atom = CAR(str)->data.atom;
    if (atom->property && atom->property->defstruct)
	fprintf(lisp_stderr, "*** Warning: structure %s is being redefined\n",
		strname);

    return (CAR(str));
}

/* helper functions
 *	DONT explicitly call them. Non standard functions.
 */
LispObj *
Lisp_XeditMakeStruct(LispMac *mac, LispObj *list, char *fname)
{
    int count;
    LispAtom *atom = NULL;
    LispObj *obj, *str = CAR(list), *fld, *nam, *val, *frm;

    if (!SYMBOL_P(str) ||
	(atom = str->data.atom)->property == NULL ||
	atom->property->defstruct == 0 ||
	atom->property->structure.function != STRUCT_CONSTRUCTOR)
	LispDestroy(mac, "invalid arguments, at %s", fname);
    str = atom->property->structure.definition;

    fld = NIL;
    frm = FRM;

    /* create structure fields, using default initial values */
    for (obj = CDR(str); obj != NIL; obj = CDR(obj)) {
	if (SYMBOL_P(CAR(obj)))
	    val = NIL;
	else
	    val = EVAL(CAR(CDR(CAR(obj))));

	if (fld == NIL) {
	    GCProtect();
	    fld = CONS(val, NIL);
	    FRM = CONS(fld, FRM);	/* GC protect fld linking to FRM */
	    GCUProtect();
	}
	else {
	    CDR(fld) = CONS(CAR(fld), CDR(fld));
	    CAR(fld) = val;
	}
    }
    fld = CAR(FRM) = LispReverse(fld);

    for (list = CDR(list); list != NIL; list = CDR(list)) {
	if ((nam = EVAL(CAR(list)))->type != LispAtom_t ||
	    STRPTR(nam)[0] != ':')
	    LispDestroy(mac, "%s is a illegal field for %s",
			LispStrObj(mac, nam), fname);

	/* check if field name is a valid field name */
	for (count = 0, obj = CDR(str); obj != NIL; ++count, obj = CDR(obj)) {
	    if ((SYMBOL_P(CAR(obj)) &&
		 strcmp(STRPTR(CAR(obj)), STRPTR(nam) + 1) == 0) ||
		(CAR(obj)->type == LispCons_t &&
		 strcmp(STRPTR(CAR(CAR(obj))), STRPTR(nam) + 1) == 0))
		break;
	}

	/* check if structure has named field */
	if (obj == NIL)
	    LispDestroy(mac, ":%s is not a %s field, at %s",
			STRPTR(nam), STRPTR(CAR(str)), fname);

	/* value supplied? */
	if ((list = CDR(list)) == NIL)
	    LispDestroy(mac, "expecting value for field, at %s", fname);

	/* set structure field value */
	for (obj = fld; count; obj = CDR(obj))
	    --count;
	if (obj->prot == LispNil_t) {
	    CAR(obj) = CAR(list);
	    /* set value only if the first time */
	    obj->prot = LispTrue_t;
	}
    }

    /* clean protect flag */
    for (obj = fld; obj != NIL; obj = CDR(obj))
	obj->prot = LispNil_t;

    FRM = frm;	/* GC Uprotect fld */

    return (STRUCT(fld, str));
}

LispObj *
Lisp_XeditStructAccess(LispMac *mac, LispObj *list, char *fname)
{
    int len = 0;
    LispAtom *atom = NULL;
    LispObj *str = CAR(list), *obj = CAR(CDR(list));

    if (!SYMBOL_P(str) ||
	(atom = str->data.atom)->property == NULL ||
	atom->property->defstruct == 0 ||
	(len = atom->property->structure.function) < 0)
	LispDestroy(mac, "invalid arguments, at %s", fname);
    str = atom->property->structure.definition;

    /* check if the object is of the required type */
    if (obj->type != LispStruct_t || obj->data.struc.def != str)
	LispDestroy(mac, "%s is not a %s",
		    LispStrObj(mac, obj), STRPTR(CAR(str)));

    for (obj = CAR(obj); len; obj = CDR(obj))
	--len;

    return (CAR(obj));
}

LispObj *
Lisp_XeditStructStore(LispMac *mac, LispObj *list, char *fname)
{
    int len = 0;
    LispAtom *atom = NULL;
    LispObj *strdef, *str, *value;

    strdef = CAR(list);
    list = CDR(list);
    str = CAR(list);
    list = CDR(list);
    value = CAR(list);

    if (!SYMBOL_P(strdef) ||
	(atom = strdef->data.atom)->property == NULL ||
	atom->property->defstruct == 0 ||
	(len = atom->property->structure.function) < 0)
	LispDestroy(mac, "invalid arguments, at %s", fname);
    strdef = atom->property->structure.definition;

    /* check if the object is of the required type */
    if (str->type != LispStruct_t || str->data.struc.def != strdef)
	LispDestroy(mac, "%s is not a %s",
		    LispStrObj(mac, str),
		    STRPTR(CAR(strdef)));

    for (str = CAR(str); len; str = CDR(str))
	--len;

    return (CAR(str) = value);
}

LispObj *
Lisp_XeditStructType(LispMac *mac, LispObj *list, char *fname)
{
    LispAtom *atom = NULL;
    LispObj *str = CAR(list), *obj = CAR(CDR(list));

    if (!SYMBOL_P(str) ||
	(atom = str->data.atom)->property == NULL ||
	atom->property->defstruct == 0 ||
	atom->property->structure.function != STRUCT_CHECK)
	LispDestroy(mac, "invalid arguments, at %s", fname);
    str = atom->property->structure.definition;

    /* check if the object is of the required type */
    if (obj->type == LispStruct_t && obj->data.struc.def == str)
	return (T);

    return (NIL);
}

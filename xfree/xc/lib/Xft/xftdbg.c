/*
 * $XFree86: xc/lib/Xft/xftdbg.c,v 1.1 2000/11/29 08:39:21 keithp Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "xftint.h"
#include <stdio.h>

void
XftValuePrint (XftValue v)
{
    switch (v.type) {
    case XftTypeVoid:
	printf (" <void>");
	break;
    case XftTypeInteger:
	printf (" %d", v.u.i);
	break;
    case XftTypeDouble:
	printf (" %g", v.u.d);
	break;
    case XftTypeString:
	printf (" \"%s\"", v.u.s);
	break;
    case XftTypeBool:
	printf (" %s", v.u.b ? "True" : "False");
	break;
    }
}

void
XftValueListPrint (XftValueList *l)
{
    for (; l; l = l->next)
	XftValuePrint (l->value);
}

void
XftPatternPrint (XftPattern *p)
{
    int		    i;
    XftPatternElt   *e;
    
    printf ("Pattern %d of %d\n", p->num, p->size);
    for (i = 0; i < p->num; i++)
    {
	e = &p->elts[i];
	printf ("\t%s:", e->object);
	XftValueListPrint (e->values);
	printf ("\n");
    }
    printf ("\n");
}

void
XftOpPrint (XftOp op)
{
    switch (op) {
    case XftOpInteger: printf ("Integer"); break;
    case XftOpDouble: printf ("Double"); break;
    case XftOpString: printf ("String"); break;
    case XftOpBool: printf ("Bool"); break;
    case XftOpField: printf ("Field"); break;
    case XftOpAssign: printf ("Assign"); break;
    case XftOpPrepend: printf ("Prepend"); break;
    case XftOpAppend: printf ("Append"); break;
    case XftOpQuest: printf ("Quest"); break;
    case XftOpOr: printf ("Or"); break;
    case XftOpAnd: printf ("And"); break;
    case XftOpEqual: printf ("Equal"); break;
    case XftOpNotEqual: printf ("NotEqual"); break;
    case XftOpLess: printf ("Less"); break;
    case XftOpLessEqual: printf ("LessEqual"); break;
    case XftOpMore: printf ("More"); break;
    case XftOpMoreEqual: printf ("MoreEqual"); break;
    case XftOpPlus: printf ("Plus"); break;
    case XftOpMinus: printf ("Minus"); break;
    case XftOpTimes: printf ("Times"); break;
    case XftOpDivide: printf ("Divide"); break;
    case XftOpNot: printf ("Not"); break;
    case XftOpNil: printf ("Nil"); break;
    }
}

void
XftTestPrint (XftTest *test)
{
    switch (test->qual) {
    case XftQualAny:
	printf ("any ");
	break;
    case XftQualAll:
	printf ("all ");
	break;
    }
    printf ("%s ", test->field);
    XftOpPrint (test->op);
    printf (" ");
    XftValuePrint (test->value);
    printf ("\n");
}

void
XftExprPrint (XftExpr *expr)
{
    switch (expr->op) {
    case XftOpInteger: printf ("%d", expr->u.ival); break;
    case XftOpDouble: printf ("%g", expr->u.dval); break;
    case XftOpString: printf ("\"%s\"", expr->u.sval); break;
    case XftOpBool: printf ("%s", expr->u.bval ? "true" : "false"); break;
    case XftOpField: printf ("%s", expr->u.field); break;
    case XftOpQuest:
	XftExprPrint (expr->u.tree.left);
	printf (" quest ");
	XftExprPrint (expr->u.tree.right->u.tree.left);
	printf (" colon ");
	XftExprPrint (expr->u.tree.right->u.tree.right);
	break;
    case XftOpOr:
    case XftOpAnd:
    case XftOpEqual:
    case XftOpNotEqual:
    case XftOpLess:
    case XftOpLessEqual:
    case XftOpMore:
    case XftOpMoreEqual:
    case XftOpPlus:
    case XftOpMinus:
    case XftOpTimes:
    case XftOpDivide:
	XftExprPrint (expr->u.tree.left);
	printf (" ");
	switch (expr->op) {
	case XftOpOr: printf ("Or"); break;
	case XftOpAnd: printf ("And"); break;
	case XftOpEqual: printf ("Equal"); break;
	case XftOpNotEqual: printf ("NotEqual"); break;
	case XftOpLess: printf ("Less"); break;
	case XftOpLessEqual: printf ("LessEqual"); break;
	case XftOpMore: printf ("More"); break;
	case XftOpMoreEqual: printf ("MoreEqual"); break;
	case XftOpPlus: printf ("Plus"); break;
	case XftOpMinus: printf ("Minus"); break;
	case XftOpTimes: printf ("Times"); break;
	case XftOpDivide: printf ("Divide"); break;
	default: break;
	}
	printf (" ");
	XftExprPrint (expr->u.tree.right);
	break;
    case XftOpNot:
	printf ("Not ");
	XftExprPrint (expr->u.tree.left);
	break;
    default:
	break;
    }
}

void
XftEditPrint (XftEdit *edit)
{
    printf ("Edit %s ", edit->field);
    XftOpPrint (edit->op);
    printf (" ");
    XftExprPrint (edit->expr);
}

void
XftSubstPrint (XftSubst *subst)
{
    XftEdit	*e;
    XftTest	*t;
    
    printf ("match\n");
    for (t = subst->test; t; t = t->next)
    {
	printf ("\t");
	XftTestPrint (t);
    }
    printf ("edit\n");
    for (e = subst->edit; e; e = e->next)
    {
	printf ("\t");
	XftEditPrint (e);
	printf (";\n");
    }
    printf ("\n");
}

void
XftFontSetPrint (XftFontSet *s)
{
    int	    i;

    printf ("FontSet %d of %d\n", s->nfont, s->sfont);
    for (i = 0; i < s->nfont; i++)
    {
	printf ("Font %d ", i);
	XftPatternPrint (s->fonts[i]);
    }
}

/*
 * Copyright (c) 1999 by The XFree86 Project, Inc.
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

/* $XFree86: xc/programs/xedit/c-mode.c,v 1.5 2001/01/30 15:03:33 paulo Exp $ */

#include "xedit.h"
#include <X11/IntrinsicP.h>
#include <X11/Xaw/TextSinkP.h>
#include <X11/Xaw/TextSrcP.h>
#include <X11/Xmu/Xmu.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>		/* for bsearch() */
#endif
#include <ctype.h>

#define C_Peek(parser)	((parser)->next)

/*
 * Types
 */
typedef struct _C_ParserEnt C_ParserEnt;
struct _C_ParserEnt {
    XawTextPosition position;
    Cardinal length;
    XrmQuark identifier;
    C_ParserEnt *next;
};

typedef struct _C_Parser {
    Widget source;
    XawTextBlock block;
    XawTextPosition position, offset, start, end,
		    update_from, update_to, clear_from, clear_to, last;
    C_ParserEnt *ent;
    XrmQuark quark;
    int i, ch, next;
    Bool interactive;
} C_Parser;

/*
 * Prototypes
 */
void C_ModeStart(Widget);
void C_ModeEnd(Widget);
static void C_ModeInit(void);
static Bool C_IsKeyword(char*);
static int C_Get(C_Parser*);
static void C_Commit(C_Parser*);
static int C_Parse1(C_Parser*);
static int C_Parse2(C_Parser*);
static int C_Parse3(C_Parser*);
static int C_Parse4(C_Parser*);
static void C_ParseCallback(Widget, XtPointer, XtPointer);

extern void _XawTextNeedsUpdating(TextWidget, XawTextPosition, XawTextPosition);


/*
 * Initialization
 */
static XrmQuark
	Qkeyword,
	Qpreprocessor,
	Qcomment,
	Qstring,
	Qconstant,
	Qoctal,
	Qhexa,
	Qinteger,
	Qfloat,
	Qpunctuation,
	Qdefault,
	Qerror;

static char *keywords[] = {
    "asm",
    "auto",
    "break",
    "case",
    "catch",
    "char",
    "class",
    "const",
    "continue",
    "default",
    "delete",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "new",
    "operator",
    "private",
    "protected",
    "public",
    "register",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "template",
    "this",
    "throw",
    "try",
    "typedef",
    "union",
    "unsigned",
    "virtual",
    "void",
    "volatile",
    "while"
};

/*
 * Implementation
 */
void
C_ModeStart(Widget src)
{
    C_Parser parser;

    C_ModeInit();

    /* initialize parser state */
    parser.source = src;
    parser.position = XawTextSourceRead(parser.source, 0,
					&parser.block, 4096);
    parser.offset = -1;
    parser.quark = NULLQUARK;
    parser.i = 0;
    if (parser.block.length == 0)
	parser.ch = parser.next = EOF;
    else
	(void)C_Get(&parser);
    parser.interactive = False;
    parser.last = XawTextSourceScan(src, 0, XawstAll, XawsdRight, 1, 1);

    while (C_Parse1(&parser) != EOF)
	;

    /* add callbacks for interactive changes */
    XtAddCallback(src, XtNpropertyCallback, C_ParseCallback, NULL);
}

void
C_ModeEnd(Widget src)
{
    XtRemoveCallback(src, XtNpropertyCallback, C_ParseCallback, NULL);
}

#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#endif
#ifndef ABS
#define ABS(a)		((a) > 0 ? (a) : -(a))
#endif

static XawTextPosition
C_ParseSearch(Widget w, XawTextPosition from, XawTextPosition to, char *ptr)
{
    XawTextBlock block;
    char *match = ptr;

    if (!ptr || !*ptr || from >= to)
	return (XawTextSearchError);

    while (from < to) {
	int i;

	from = XawTextSourceRead(w, from, &block, to - from);
	for (i = 0; i < block.length; i++) {
	    if (block.ptr[i] == *match) {
		if (*++match == '\0')
		    return (block.firstPos + i - (match - ptr));
	    }
	    else
		match = ptr;
	}
    }

    return (XawTextSearchError);
}

/* find start/end pair of a preprocessor directive */
static void
C_ParsePreprocessor(Widget w, XawTextAnchor *anchor, XawTextEntity *entity,
		    XawTextPosition *left_return, XawTextPosition *right_return)
{
    XawTextPosition left, right, ltmp, rtmp;
    char *ptr;
    XawTextBlock block;

    left = anchor->position + entity->offset;
    right = left + entity->length;

    (void)XawTextSourceRead(w, left, &block, 1);
    if (block.length != 1 || block.ptr[0] != '#')
	ltmp = XawTextSourceScan(w, left, XawstEOL, XawsdLeft, 1, False);
    else
	ltmp = left;

    /* left is resolved */
    left = MIN(left, ltmp);

    ptr = "\\\n";
    rtmp = left + 1;
    /*CONSTCOND*/
    while (True) {
	ltmp = XawTextSourceScan(w, rtmp, XawstEOL, XawsdRight, 1, True);
	if ((rtmp = C_ParseSearch(w, rtmp, ltmp, ptr)) ==
	    XawTextSearchError || ltmp < rtmp + 2) {
	    rtmp = ltmp;
	    break;
	}
	rtmp += 2;
    }

    ptr = "/*";
    if ((ltmp = C_ParseSearch(w, left, rtmp, ptr)) !=
	XawTextSearchError && ltmp < rtmp) {
	block.ptr = "*/";
	if ((ltmp = XawTextSourceSearch(w, ltmp + 2, XawsdRight, &block)) ==
	    XawTextSearchError)
	    rtmp = XawTextSourceScan(w, 0, XawstAll, XawsdRight, 1, 1);
	else
	    rtmp = MAX(ltmp + 2, rtmp);

	ptr = "\\\n";
	/*CONSTCOND*/
	while (True) {
	    ltmp = XawTextSourceScan(w, rtmp, XawstEOL, XawsdRight, 1, True);
	    if ((rtmp = C_ParseSearch(w, rtmp, ltmp, ptr)) ==
		XawTextSearchError || ltmp < rtmp + 2) {
		rtmp = ltmp;
		break;
	    }
	    rtmp += 2;
	}
    }
    right = MAX(right, rtmp);

    *left_return = left;
    *right_return = right;
}

/* better to have a public interface to things like this... */
static void
C_ParseNeedsUpdating(Widget w, XawTextPosition from, XawTextPosition to)
{
    TextSrcObject src = (TextSrcObject)w;
    int i;

    for (i = 0; i < src->textSrc.num_text; i++)
	_XawTextNeedsUpdating((TextWidget)(src->textSrc.text[i]), from, to);
}

static void
C_ParseCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    XawTextPropertyInfo *info = (XawTextPropertyInfo*)call_data;
    XawTextAnchor *anchor;
    XawTextEntity *entity, *eprev;
    XawTextPosition left, right, last, rtmp, ltmp, pleft, pright;
    XawTextPosition position = info->left;
    XawTextBlock block;

    block.firstPos = 0;
    block.format = FMT8BIT;

    rtmp = ltmp = position;
    left = 0;
    right = last = XawTextSourceScan(w, 0, XawstAll, XawsdRight, 1, 1);

    if ((anchor = XawTextSourceFindAnchor(w, position)) != NULL) {
	XawTextEntity *prep = NULL;

previous_anchor:
	entity = eprev = anchor->entities;
	while (entity) {
	    if (anchor->position + entity->offset + entity->length >= position) {
		if (eprev && eprev->property == Qpreprocessor) {
		    prep = eprev;
		    C_ParsePreprocessor(w, anchor, eprev, &pleft, &pright);
		    ltmp = MIN(ltmp, pleft);
		    rtmp = MAX(rtmp, pright);
		}
		else if (entity && entity->property == Qpreprocessor) {
		    prep = entity;
		    C_ParsePreprocessor(w, anchor, entity, &pleft, &pright);
		    ltmp = MIN(ltmp, pleft);
		    rtmp = MAX(rtmp, pright);
		}
		break;
	    }
	    eprev = entity;
	    entity = entity->next;
	}
	if (eprev == NULL ||
	    anchor->position + eprev->offset + eprev->length >= position) {
	    if ((anchor = XawTextSourcePrevAnchor(w, anchor)) != NULL)
		goto previous_anchor;
	    if (anchor == NULL || eprev == NULL) {
		ltmp = 0;
		if ((anchor = XawTextSourceFindAnchor(w, 0)) != NULL)
		    eprev = anchor->entities;
	    }
	}
	if (anchor && eprev) {
	    left = anchor->position + eprev->offset + eprev->length;
	    if (eprev->property == Qerror)
		left -= eprev->length;
	    if ((entity = eprev->next) == NULL) {
		if ((anchor = XawTextSourceNextAnchor(w, anchor)) != NULL) {
		    entity = anchor->entities;
		    if (entity && entity->property == Qpreprocessor) {
			C_ParsePreprocessor(w, anchor, entity, &pleft, &pright);
			ltmp = MIN(ltmp, pleft);
			rtmp = MAX(rtmp, pright);
		    }
		}
	    }
	    if (entity && (entity = entity->next) == NULL) {
		if ((anchor = XawTextSourceNextAnchor(w, anchor)) != NULL)
		    entity = anchor->entities;
		if (entity && entity->property == Qpreprocessor &&
		    entity != prep) {
		    C_ParsePreprocessor(w, anchor, entity, &pleft, &pright);
		    ltmp = MIN(ltmp, pleft);
		    rtmp = MAX(rtmp, pright);
		}
	    }
	    if (entity && (entity = entity->next) == NULL) {
		if ((anchor = XawTextSourceNextAnchor(w, anchor)) != NULL)
		    entity = anchor->entities;
	    }
	    if (anchor && entity) {
		int count = 0;

		if (entity->property == Qpreprocessor && entity != prep) {
		    C_ParsePreprocessor(w, anchor, entity, &pleft, &pright);
		    ltmp = MIN(ltmp, pleft);
		    rtmp = MAX(rtmp, pright);
		}
		right = anchor->position + entity->offset;
		while (entity && count++ < 3) {
		    if (entity->property == Qerror) {
			right = anchor->position + entity->offset + entity->length;
			count = 0;
		    }
		    if ((entity = entity->next) == NULL &&
			(anchor = XawTextSourceNextAnchor(w, anchor)) != NULL)
			entity = anchor->entities;
		}
	    }
	}
    }

    left  = MAX(0, MIN(ltmp, left));
    right = MIN(last, MAX(right, rtmp));

    if (left < right) {
	C_Parser parser;
	C_ParserEnt *ent, *nent;
	XawTextAnchor *kanc;
	XawTextEntity *kent;
	XawTextPosition kfrom, kto;
	int delta = info->block->length - (info->right - info->left);

	if (XawTextSourceAnchorAndEntity(w, position - MAX(1, ABS(delta)),
					 &kanc, &kent) == False)
	    kent = NULL;
	else {
	    kfrom = kanc->position + kent->offset;
	    kto = kfrom + kent->length;
	}

	parser.position = XawTextSourceRead(parser.source = w, left,
					    &parser.block, 4096);
	parser.offset = left - 1;
	ltmp = XawTextSourceScan(w, left, XawstEOL, XawsdLeft, 1, False);
	if (ltmp == left ||
	    XawTextSourceScan(w, XawTextSourceScan(w, ltmp, XawstWhiteSpace,
			      			   XawsdRight, 1, False),
			      XawstWhiteSpace, XawsdLeft, 1, False) == left)
	    parser.quark = NULLQUARK;
	else
	    parser.quark = Qdefault;
	parser.i = 0;
	if (parser.block.length == 0)
	    parser.ch = parser.next = EOF;
	else
	    (void)C_Get(&parser);
	parser.interactive = True;
	parser.clear_from = left;
	parser.clear_to = right;
	parser.update_from = last;
	parser.update_to = 0;
	parser.ent = NULL;
	parser.last = last;

	while (C_Parse1(&parser) != EOF)
	    if (parser.offset > MAX(right, parser.update_to))
		break;

	if (parser.update_from > info->left)
	    parser.update_from -= delta;
	if (parser.update_to > info->left)
	    parser.update_to -= delta;
	XawTextSourceClearEntities(parser.source, parser.clear_from,
				   parser.clear_to);
	C_ParseNeedsUpdating(parser.source, parser.update_from,
			     parser.update_to);

	ent = parser.ent;
	while (ent) {
	    nent = ent->next;
	    XawTextSourceAddEntity(parser.source, 0, 0, NULL, ent->position,
				   ent->length, ent->identifier);
	    XtFree((XtPointer)ent);
	    ent = nent;
	}

	if (kent) {
	    if (XawTextSourceAnchorAndEntity(w, kfrom, &kanc, &kent) == False ||
		kanc->position + kent->offset != kfrom ||
		kfrom + kent->length != kto) {
		if (kfrom > info->left)
		    kfrom -= delta;
		if (kto > info->left)
		    kto -= delta;
		C_ParseNeedsUpdating(parser.source, kfrom, kto);
	    }
	}
    }
}

static int
bcmp_keywords(_Xconst void *left, _Xconst void *right)
{
    return (strcmp((char*)left, *(char**)right));
}

static Bool
C_IsKeyword(char *str)
{
    return (bsearch(str, keywords, sizeof(keywords) / sizeof(keywords[0]),
		    sizeof(char*), bcmp_keywords) != NULL);
}

static int
C_Get(C_Parser *parser)
{
    if (parser->ch == EOF)
	return (EOF);
    if (parser->i >= parser->block.length) {
	parser->i = 0;
	parser->position = XawTextSourceRead(parser->source, parser->position,
					     &parser->block, 4096);
    }
    parser->ch = parser->next;
    if (parser->block.length == 0)
	parser->next = EOF;
    else
	parser->next = (unsigned char)parser->block.ptr[parser->i++];
    parser->offset++;

    return (parser->ch);
}

static void
C_Commit(C_Parser *parser)
{
    XawTextPosition position;
    int length;

    position = parser->start;
    length = parser->end - parser->start;
    if (position < 0) {
	length += position;
	position = 0;
    }
    if (position + length > parser->last + 1)
	length -= (position + length) - parser->last + 1;

    if (parser->quark != Qdefault && parser->quark != NULLQUARK && length > 0) {
	XrmQuark quark = parser->quark;

	parser->quark = Qdefault;

	if (parser->interactive) {
	    XawTextAnchor *anchor;
	    XawTextEntity *entity;
	    C_ParserEnt *ent = XtNew(C_ParserEnt);

	    ent->position = position;
	    ent->length = length;
	    ent->identifier = quark;
	    ent->next = parser->ent;
	    parser->ent = ent;

	    if (position < parser->clear_from)
		parser->clear_from = position;
	    if (position + length > parser->clear_to)
		parser->clear_to = position + length;

	    if (XawTextSourceAnchorAndEntity(parser->source, position,
					     &anchor, &entity) == False ||
		anchor->position + entity->offset != position ||
		entity->length != length ||
		entity->property != quark) {
		if (position < parser->update_from)
		    parser->update_from = position;
		if (position + length > parser->update_to)
		    parser->update_to = position + length;
	    }
	}
	else
	    XawTextSourceAddEntity(parser->source, 0, 0, NULL, position, length,
				   quark);
    }
}

/* comments */
static int
C_Parse4(C_Parser *parser)
{
    int ch;

    for (;;) {
	if ((ch = C_Get(parser)) == '/') {
	    switch (C_Peek(parser)) {
		case '/': {
		    XrmQuark quark = parser->quark;

		    parser->end = parser->offset - 1;
		    C_Commit(parser);
		    parser->quark = Qcomment;
		    parser->start = parser->end;
		    while ((ch = C_Peek(parser)) != '\n' && ch != EOF)
			(void)C_Get(parser);
		    if (quark != Qpreprocessor && ch != EOF)
			(void)C_Get(parser);
		    parser->end = parser->offset;
		    C_Commit(parser);
		    if (quark == Qpreprocessor) {
			parser->start = parser->end;
			parser->quark = quark;
		    }
		    else
			parser->quark = NULLQUARK;
		}   break;
		case '*': {
		    XrmQuark quark = parser->quark;

		    parser->end = parser->offset - 1;
		    C_Commit(parser);
		    parser->quark = Qcomment;
		    parser->start = parser->end;
		    while ((ch = C_Get(parser)) != EOF)
			if (ch == '*' && C_Peek(parser) == '/')
			    break;
		    if (ch != EOF)
			ch = C_Get(parser);
		    parser->end = parser->offset;
		    C_Commit(parser);

		    parser->start = parser->end;
		    parser->quark = quark;
		}   break;
		default:
		    return (ch);
	    }
	}
	else
	    return (ch);
    }
    /*NOTREACHED*/
}

/* identifiers, preprocessor and blanks */
static int
C_Parse3(C_Parser *parser)
{
    for (;;) {
	int ch = C_Parse4(parser);

	if (ch >= 'a' && ch <= 'w') {
	    char buf[32];
	    int sz = 0;

	    parser->end = parser->offset - 1;
	    C_Commit(parser);
	    parser->start = parser->end;
	    buf[sz++] = ch;
	    while ((ch = C_Peek(parser)) != EOF && ch >= 'a' && ch <= 'z' &&
		   sz + 1 < sizeof(buf))
		buf[sz++] = C_Get(parser);
	    buf[sz] = '\0';
	    if (!isalnum(ch) && ch != '_' && C_IsKeyword(buf)) {
		parser->quark = Qkeyword;
		parser->end = parser->offset;
		C_Commit(parser);
		parser->quark = Qdefault;
		parser->start = parser->end;
	    }
	    else if (isalnum(ch) || ch == '_')
		while ((ch = C_Peek(parser)) != EOF &&
		       (isalnum(ch) || ch == '_'))
		    (void)C_Get(parser);
	    parser->quark = Qdefault;
	    continue;
	}
	else if (ch == '_' || isalpha(ch)) {
	    parser->end = parser->offset - 1;
	    C_Commit(parser);
	    parser->quark = Qdefault;
	    while ((ch = C_Peek(parser)) != EOF &&
		   (isalnum(ch) || ch == '_'))
		(void)C_Get(parser);
	    continue;
	}

	switch (ch) {
	    case '\'':
	    case '"': {
		char value = ch;

		parser->end = parser->offset - 1;
		C_Commit(parser);
		parser->start = parser->end;
		parser->quark = ch == '\'' ? Qconstant : Qstring;
		while ((ch = C_Get(parser)) != value && ch != '\n' && ch != EOF)
		    if (ch == '\\')
			(void)C_Get(parser);
		parser->end = parser->offset;
		C_Commit(parser);
		if (ch != value) {
		    parser->start = parser->end;
		    parser->quark = Qerror;
		    while ((ch = C_Peek(parser)) != '\n' && ch != EOF)
			(void)C_Get(parser);
		}
	    }	break;
	    case '#':
		parser->end = parser->offset - 1;
		C_Commit(parser);
		if (parser->quark != NULLQUARK)
		    parser->quark = Qerror;
		else
		    parser->quark = Qpreprocessor;
		parser->start = parser->end;
		if (parser->quark == Qerror) {
		    while ((ch = C_Peek(parser)) != '\n' && ch != EOF)
			(void)C_Get(parser);
		}
		else {
		    while ((ch = C_Parse4(parser)) != '\n' && ch != EOF) {
			if (ch == '\\')
			    (void)C_Get(parser);
			else if (ch == '"' || ch == '\'') {
			    int c;

			    while ((c = C_Peek(parser)) != '\n' && c != EOF) {
				(void)C_Get(parser);
				if (c == '\\')
				    (void)C_Get(parser);
				else if (c == ch)
				    break;
			    }
			}
		    }
		}
		parser->end = parser->offset;
		C_Commit(parser);
		parser->quark = NULLQUARK;
		break;
	    default:
		return (ch);
	}
    }
    /*NOTREACHED*/
}

static int
C_Parse2Fail(C_Parser *parser)
{
    int ch;

    parser->quark = Qerror;
    while ((isalnum(ch = C_Peek(parser)) || ch == '_') && ch != EOF)
	(void)C_Get(parser);

    parser->end = parser->offset;
    C_Commit(parser);
    parser->start = parser->end;

    return (C_Parse3(parser));
}

/* numbers */
static int
C_Parse2(C_Parser *parser)
{
    int ch = C_Parse3(parser);
    Bool dot = False, E = False, U = False, L = False, sign = False,
	octal = False, real = False, hexa = False, first = True, did_get = False;

    for (;;) {
	switch (ch) {
	    case '+':
	    case '-':
		if (!E || sign || hexa || octal)
		    goto check_first;
		sign = True;
		break;
	    case '.':
		if (dot || E || L || U || hexa)
		    return (C_Parse2Fail(parser));
		if (first && !isdigit(C_Peek(parser)))
		    return ('.');
		octal = hexa = False;
		dot = real = True;
		break;
	    case 'e':
	    case 'E':
		if (!hexa && E)
		    return (C_Parse2Fail(parser));
		else if (!hexa)
		    E = real = True;
		break;
	    case 'a': case 'A': case 'b': case 'B': case 'c': case 'C':
	    case 'd': case 'D': case 'f': case 'F':
		if (dot || E || !hexa)
		    return (C_Parse2Fail(parser));
		break;
	    case 'l': case 'L':
		if (L)
		    return (C_Parse2Fail(parser));
		L = True;
		break;
	    case 'u': case 'U':
		if (U || real)
		    return (C_Parse2Fail(parser));
		U = True;
		break;
	    case 'x': case 'X':
		if (L || U || E || sign || hexa || !octal)
		    return (C_Parse2Fail(parser));
		octal = False;
		hexa = True;
		(void)C_Get(parser);
		did_get = True;
		if (!isdigit(C_Peek(parser))) {
		    switch (tolower(C_Peek(parser))) {
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':
			    break;
			default:
			    return (C_Parse2Fail(parser));
		    }
		}
		break;
	    case '0':
		if (L || U)
		    return (C_Parse2Fail(parser));
		if (first)
		    octal = True;
		break;
	    case '1': case '2': case '3': case '4': case '5': case '6':
	    case '7':
		if (L || U)
		    return (C_Parse2Fail(parser));
		break;
	    case '8': case '9':
		if (L || U || octal)
		    return (C_Parse2Fail(parser));
		break;
	    default:
	    check_first:
		if (isalpha(ch) || ch == '_')
		    return (C_Parse2Fail(parser));
		if (!first) {
		    parser->end = parser->offset;
		    if (octal && parser->end - parser->start > (1 + L + U))
			parser->quark = Qoctal;
		    else if (hexa)
			parser->quark = Qhexa;
		    else if (real || E)
			parser->quark = Qfloat;
		    else
			parser->quark = Qinteger;
		    C_Commit(parser);
		    parser->start = parser->end;
		}
		return (first ? ch : C_Parse3(parser));
	}
	if (first) {
	    parser->end = parser->offset - 1;
	    C_Commit(parser);
	    parser->start = parser->end;
	    first = False;
	}
	else {
	    if (did_get == False)
		(void)C_Get(parser);
	    did_get = False;
	}
	ch = C_Peek(parser); 
    }
    /*NOTREACHED*/
}

/* punctuations */
static int
C_Parse1(C_Parser *parser)
{
    int ch;

    for (;;) {
	switch (ch = C_Parse2(parser)) {
	    case '/': case '*': case '+': case '-': case ':': case ';':
	    case '=': case '(': case ')': case '<': case '>': case ',':
	    case '&': case '.': case '!': case '{': case '}': case '[':
	    case ']': case '%': case '|': case '^': case '~': case '?':
		if (parser->quark != Qpunctuation) {
		    parser->end = parser->offset - 1;
		    C_Commit(parser);
		    parser->quark = Qpunctuation;
		    parser->start = parser->end;
		}
		return (ch);
	    case EOF:
		parser->end = parser->offset - 1;
		C_Commit(parser);
		return (EOF);
	    default:
		parser->end = parser->offset - 1;
		C_Commit(parser);
		if (ch == '\n')
		    parser->quark = NULLQUARK;
		else if (ch == ' ' || ch == '\t')
		    parser->quark = parser->quark == NULLQUARK ? NULLQUARK : Qdefault;
		else
		    parser->quark = Qerror;
		parser->start = parser->end;
		return (ch);
	}
    }
}

void
C_ModeInit(void)
{
    static int initialized;

    if (initialized)
	return;

    Qkeyword		= XrmPermStringToQuark("keyword");
    Qpreprocessor	= XrmPermStringToQuark("preprocessor");
    Qcomment		= XrmPermStringToQuark("comment");
    Qstring		= XrmPermStringToQuark("string");
    Qconstant		= XrmPermStringToQuark("constant");
    Qoctal		= XrmPermStringToQuark("octal");
    Qhexa		= XrmPermStringToQuark("hexa");
    Qinteger		= XrmPermStringToQuark("integer");
    Qfloat		= XrmPermStringToQuark("float");
    Qpunctuation	= XrmPermStringToQuark("punctuation");
    Qdefault		= XrmPermStringToQuark("default");
    Qerror		= XrmPermStringToQuark("error");

    initialized = True;
}

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

/* $XFree86: xc/programs/xedit/lisp/string.h,v 1.2 2001/10/18 03:15:22 paulo Exp $ */

#ifndef Lisp_string_h
#define Lisp_string_h

#include "internal.h"

LispObj *Lisp_Char(LispMac*, LispObj*, char*);		   /* char */
LispObj *Lisp_CharLess(LispMac*, LispObj*, char*);	   /* char< */
LispObj *Lisp_CharLessEqual(LispMac*, LispObj*, char*);	   /* char<= */
LispObj *Lisp_CharEqual_(LispMac*, LispObj*, char*);	   /* char= */
LispObj *Lisp_CharGreater(LispMac*, LispObj*, char*);	   /* char> */
LispObj *Lisp_CharGreaterEqual(LispMac*, LispObj*, char*); /* char>= */
LispObj *Lisp_CharNotEqual_(LispMac*, LispObj*, char*);	   /* char/= */
LispObj *Lisp_CharLessp(LispMac*, LispObj*, char*);	   /* char-lessp */
LispObj *Lisp_CharNotGreaterp(LispMac*, LispObj*, char*);  /* char-not-greaterp */
LispObj *Lisp_CharEqual(LispMac*, LispObj*, char*);	   /* char-equal */
LispObj *Lisp_CharGreaterp(LispMac*, LispObj*, char*);	   /* char-greaterp */
LispObj *Lisp_CharNotLessp(LispMac*, LispObj*, char*);	   /* char-not-lessp */
LispObj *Lisp_CharNotEqual(LispMac*, LispObj*, char*);	   /* char-not-equal */
LispObj *Lisp_Character(LispMac*, LispObj*, char*);	    /* character */
LispObj *Lisp_CharDowncase(LispMac*, LispObj*, char*);	    /* char-downcase */
LispObj *Lisp_CharInt(LispMac*, LispObj*, char*);	    /* char-int */
LispObj *Lisp_CharUpcase(LispMac*, LispObj*, char*);	    /* char-upcase */
LispObj *Lisp_IntChar(LispMac*, LispObj*, char*);	    /* int-char */
LispObj *Lisp_ReadFromString(LispMac*, LispObj*, char*);    /* read-from-string */
LispObj *Lisp_String(LispMac*, LispObj*, char*);	    /* string */
LispObj *Lisp_StringTrim(LispMac*, LispObj*, char*);	    /* string-trim */
LispObj *Lisp_StringLeftTrim(LispMac*, LispObj*, char*);    /* string-left-trim */
LispObj *Lisp_StringRightTrim(LispMac*, LispObj*, char*);   /* string-right-trim */
LispObj *Lisp_StringEqual_(LispMac*, LispObj*, char*);	    /* string= */
LispObj *Lisp_StringLess(LispMac*, LispObj*, char*);	    /* string< */
LispObj *Lisp_StringGreater(LispMac*, LispObj*, char*);	    /* string> */
LispObj *Lisp_StringLessEqual(LispMac*, LispObj*, char*);   /* string<= */
LispObj *Lisp_StringGreaterEqual(LispMac*, LispObj*, char*);/* string>= */
LispObj *Lisp_StringNotEqual_(LispMac*, LispObj*, char*);   /* string/= */
LispObj *Lisp_StringEqual(LispMac*, LispObj*, char*);	    /* string-equal */
LispObj *Lisp_StringGreaterp(LispMac*, LispObj*, char*);    /* string-greaterp */
LispObj *Lisp_StringLessp(LispMac*, LispObj*, char*);	    /* string-lessp */
LispObj *Lisp_StringNotLessp(LispMac*, LispObj*, char*);    /* string-not-lessp */
LispObj *Lisp_StringNotGreaterp(LispMac*, LispObj*, char*); /* string-not-greaterp */
LispObj *Lisp_StringNotEqual(LispMac*, LispObj*, char*);    /* string-not-equal */
LispObj *Lisp_StringUpcase(LispMac*, LispObj*, char*);	    /* string-upcase */
LispObj *Lisp_StringDowncase(LispMac*, LispObj*, char*);    /* string-downcase */
LispObj *Lisp_StringCapitalize(LispMac*, LispObj*, char*);  /* string-capitalize */
LispObj *Lisp_XeditCharStore(LispMac*, LispObj*, char*);    /* xedit::char-store */

#endif /* Lisp_String_h */

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

/* $XFree86: xc/programs/xedit/lisp/string.c,v 1.3 2001/10/28 03:34:31 tsi Exp $ */

#include "string.h"
#include "private.h"
#include <ctype.h>

LispObj *
Lisp_Char(LispMac *mac, LispObj *list, char *fname)
{
    int c, pos;
    LispObj *str = CAR(list), *idx = CAR(CDR(list));

    if (!STRING_P(str))
	LispDestroy(mac, "%s is not a string, at %s",
		    LispStrObj(mac, str), fname);
    if (!NUMBER_P(idx))
	LispDestroy(mac, "%s is not a number, at %s",
		    LispStrObj(mac, str), fname);

    if (INDEX_P(idx))
	LispDestroy(mac, "bad index %s, at %s", LispStrObj(mac, idx), fname);

    if ((pos = NUMBER_VALUE(idx)) >= strlen(STRPTR(str)))
	LispDestroy(mac, "index %d is too large, at %s", pos, fname);

    c = *(unsigned char*)(STRPTR(str) + pos);

    return (CHAR(c));
}

/* helper function for setf
 *	DONT explicitly call. Non standard function
 */
LispObj *
Lisp_XeditCharStore(LispMac *mac, LispObj *list, char *fname)
{
    char *string;
    int c, pos, len;
    LispObj *str, *idx, *value;

    str = CAR(list);
    if (!STRING_P(str))
	LispDestroy(mac, "%s is not a string, at %s",
		    LispStrObj(mac, str), fname);
    list = CDR(list);
    idx = CAR(list);
    if (idx->type != LispReal_t)
	LispDestroy(mac, "%s is not a number, at %s",
		    LispStrObj(mac, str), fname);
    if (!INDEX_P(idx))
	LispDestroy(mac, "bad index %s, at %s", LispStrObj(mac, idx), fname);
    if ((pos = NUMBER_VALUE(idx)) >= (len = strlen(STRPTR(str))))
	LispDestroy(mac, "index %d is too large, at %s", pos, fname);
    list = CDR(list);
    value = CAR(list);
    if (value->type != LispCharacter_t)
	LispDestroy(mac, "%s is not a character, at %s",
		    LispStrObj(mac, value), fname);

    c = value->data.integer;

    if (c < 0 || c > 255)
	LispDestroy(mac, "cannot represent character %d, at %s", c, fname);

    string = LispStrdup(mac, STRPTR(str));
    string[pos] = c;

    str->data.atom = LispDoGetAtom(mac, string, 0, 0);
    LispFree(mac, string);

    return (value);
}

LispObj *
Lisp_CharLess(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, LESS, 0));
}

LispObj *
Lisp_CharLessEqual(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, LESS_EQUAL, 0));
}

LispObj *
Lisp_CharEqual_(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, EQUAL, 0));
}

LispObj *
Lisp_CharGreater(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, GREATER, 0));
}

LispObj *
Lisp_CharGreaterEqual(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, GREATER_EQUAL, 0));
}

LispObj *
Lisp_CharNotEqual_(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, NOT_EQUAL, 0));
}

LispObj *
Lisp_CharLessp(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, LESS, 1));
}

LispObj *
Lisp_CharNotGreaterp(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, LESS_EQUAL, 1));
}

LispObj *
Lisp_CharEqual(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, EQUAL, 1));
}

LispObj *
Lisp_CharGreaterp(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, GREATER, 1));
}

LispObj *
Lisp_CharNotLessp(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, GREATER_EQUAL, 1));
}

LispObj *
Lisp_CharNotEqual(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispCharBoolCond(mac, list, fname, NOT_EQUAL, 1));
}

LispObj *
Lisp_Character(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list);

    if (obj->type == LispCharacter_t)
	return (obj);
    else if ((SYMBOL_P(obj) || STRING_P(obj)) &&
	     STRPTR(obj)[1] == '\0')
	return (CHAR((unsigned char)STRPTR(obj)[0]));
    else if (INDEX_P(obj)) {
	int c = NUMBER_VALUE(obj);

	if (c <= 0xffff)
	    return (CHAR(c));
    }

    LispDestroy(mac, "cannot convert %s to character, at %s",
		LispStrObj(mac, obj), fname);

    /*NOTREACHED*/
    return (NIL);
}

LispObj *
Lisp_CharDowncase(LispMac *mac, LispObj *list, char *fname)
{
    int c;
    LispObj *obj = CAR(list);

    if (obj->type != LispCharacter_t)
	LispDestroy(mac, "%s is not a character, at %s",
		    LispStrObj(mac, obj), fname);

    c = tolower((int)obj->data.integer);
    if (c == obj->data.integer)
	return (obj);

    return (CHAR(c));
}

LispObj *
Lisp_CharInt(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list);

    if (obj->type != LispCharacter_t)
	LispDestroy(mac, "%s is not a character, at %s",
		    LispStrObj(mac, obj), fname);

    return (REAL(obj->data.integer));
}

LispObj *
Lisp_CharUpcase(LispMac *mac, LispObj *list, char *fname)
{
    int c;
    LispObj *obj = CAR(list);

    if (obj->type != LispCharacter_t)
	LispDestroy(mac, "%s is not a character, at %s",
		    LispStrObj(mac, obj), fname);

    c = toupper((int)obj->data.integer);
    if (c == obj->data.integer)
	return (obj);

    return (CHAR(c));
}

LispObj *
Lisp_IntChar(LispMac *mac, LispObj *list, char *fname)
{
    long character = 0;
    LispObj *obj = CAR(list);

    if (INTEGER_P(obj))
	character = NUMBER_VALUE(obj);
    else
	LispDestroy(mac, "cannot convert %s to character, at %s",
		    LispStrObj(mac, obj), fname);

    return (character >= 0 && character < 0xffff ? CHAR(character) : NIL);
}

LispObj *
Lisp_String(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *obj = CAR(list);

    if (STRING_P(obj))
	return (obj);
    else if (SYMBOL_P(obj))
	return (STRING(STRPTR(obj)));
    else if (obj->type == LispCharacter_t) {
	char string[2];

	string[0] = obj->data.integer;
	string[1] = '\0';
	return (STRING(string));
    }
    else
	LispDestroy(mac, "expecting string, symbol or character, at %s", fname);
    /*NOTREACHED*/
    return (NIL);
}

LispObj *
Lisp_ReadFromString(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    int level, length, start, end;
    LispObj *res, *eof_error, *eof_value, *ostart, *oend, *preserve;

    if (!STRING_P(CAR(list)))
	LispDestroy(mac, "expecting string, at %s", fname);
    str = STRPTR(CAR(list));

    length = strlen(str);
    eof_error = eof_value = ostart = oend = preserve = NIL;

    list = CDR(list);
    if (list != NIL) {
	eof_error = CAR(list);
	list = CDR(list);
	if (list != NIL) {
	    eof_value = CAR(list);
	    list = CDR(list);
	    if (list != NIL)
		LispGetKeys(mac, fname, "START:END:PRESERVE-WHITESPACE", list,
			    &ostart, &oend, &preserve);
	}
    }

    if (ostart == NIL)
	start = 0;
    else {
	if (!INDEX_P(ostart))
	    LispDestroy(mac, "expecting positive integer, at %s", fname);
	start = NUMBER_VALUE(ostart);
    }
    if (oend == NIL)
	end = length;
    else {
	if (!INDEX_P(oend))
	    LispDestroy(mac, "expecting positive integer, at %s", fname);
	end = NUMBER_VALUE(oend);
    }
    if (start >= end || end > length)
	LispDestroy(mac, "bad string index or length, at %s", fname);

    if (mac->stream.stream_level + 1 >= mac->stream.stream_size) {
	LispStream *stream = (LispStream*)
	    realloc(mac->stream.stream, sizeof(LispStream) *
		    (mac->stream.stream_size + 1));

	if (stream == NULL) {
	    fprintf(lisp_stderr, "out of memory");
	    return (NIL);
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
    length = end - start;
    mac->st = mac->cp = LispMalloc(mac, length + 1);
    strncpy(mac->st, str + start, length);
    mac->st[length] = '\0';
    mac->tok = 0;

    level = mac->level;
    mac->level = 0;

    res = LispRun(mac);

    LispFree(mac, mac->st);
    mac->level = level;
    --mac->stream.stream_level;

    mac->fp = mac->stream.stream[mac->stream.stream_level].fp;
    mac->st = mac->stream.stream[mac->stream.stream_level].st;
    mac->cp = mac->stream.stream[mac->stream.stream_level].cp;
    mac->tok = mac->stream.stream[mac->stream.stream_level].tok;

    if (res == EOLIST)
	LispDestroy(mac, "object cannot start with #\\)");
    if (res == NULL)
	LispDestroy(mac, "unexpected end of input, at %s", fname);

    return (res);
}

LispObj *
Lisp_StringTrim(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispStringDoTrim(mac, list, fname, 1, 1));
}

LispObj *
Lisp_StringLeftTrim(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispStringDoTrim(mac, list, fname, 1, 0));
}

LispObj *
Lisp_StringRightTrim(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispStringDoTrim(mac, list, fname, 0, 1));
}

LispObj *
Lisp_StringEqual_(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, len;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    len = end1 - start1;

    if (len != (end2 - start2) ||
	strncmp(string1 + start1, string2 + start2, len))
	return (NIL);

    return (T);
}

/* Note, most functions bellow also compare with the ending '\0'.
 * This is expected, to avoid an extra if */
LispObj *
Lisp_StringLess(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, c1, c2;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    string1 += start1;
    string2 += start2;

    for (c1 = start1, c2 = start2; c1 <= end1 && c2 <= end2;
	 c1++, c2++, string1++, string2++)
	if (*string1 < *string2)
	    return (REAL(c1));
	else if (*string1 != *string2)
	    break;

    return (NIL);
}

LispObj *
Lisp_StringGreater(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, c1, c2;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    string1 += start1;
    string2 += start2;

    for (c1 = start1, c2 = start2; c1 <= end1 && c2 <= end2;
	 c1++, c2++, string1++, string2++)
	if (*string1 > *string2)
	    return (REAL(c1));
	else if (*string1 != *string2)
	    break;

    return (NIL);
}

LispObj *
Lisp_StringLessEqual(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, c1, c2;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    string1 += start1;
    string2 += start2;

    for (c1 = start1, c2 = start2; c1 <= end1 && c2 <= end2;
	 c1++, c2++, string1++, string2++)
	if (*string1 < *string2)
	    return (REAL(c1));
	else if (*string1 != *string2)
	    return (NIL);
	else if (!*string1)
	    break;

    return (REAL(c1));
}

LispObj *
Lisp_StringGreaterEqual(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, c1, c2;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    string1 += start1;
    string2 += start2;

    for (c1 = start1, c2 = start2; c1 <= end1 && c2 <= end2;
	 c1++, c2++, string1++, string2++)
	if (*string1 > *string2)
	    return (REAL(c1));
	else if (*string1 != *string2)
	    return (NIL);
	else if (!*string1)
	    break;

    return (REAL(c1));
}

LispObj *
Lisp_StringNotEqual_(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, c1, c2;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    string1 += start1;
    string2 += start2;

    for (c1 = start1, c2 = start2; c1 <= end1 && c2 <= end2;
	 c1++, c2++, string1++, string2++)
	if (*string1 != *string2)
	    return (REAL(c1));

    return (NIL);
}

LispObj *
Lisp_StringEqual(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, len;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    len = end1 - start1;

    if (len != (end2 - start2) ||
	strncasecmp(string1 + start1, string2 + start2, len))
	return (NIL);

    return (T);
}

LispObj *
Lisp_StringLessp(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, c1, c2, ch1, ch2;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    string1 += start1;
    string2 += start2;

    for (c1 = start1, c2 = start2; c1 <= end1 && c2 <= end2;
	 c1++, c2++, string1++, string2++) {
	ch1 = toupper(*string1);
	ch2 = toupper(*string2);
	if (ch1 < ch2)
	    return (REAL(c1));
	else if (ch1 != ch2)
	    break;
    }

    return (NIL);
}

LispObj *
Lisp_StringGreaterp(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, c1, c2, ch1, ch2;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    string1 += start1;
    string2 += start2;

    for (c1 = start1, c2 = start2; c1 <= end1 && c2 <= end2;
	 c1++, c2++, string1++, string2++) {
	ch1 = toupper(*string1);
	ch2 = toupper(*string2);
	if (ch1 > ch2)
	    return (REAL(c1));
	else if (ch1 != ch2)
	    break;
    }

    return (NIL);
}

LispObj *
Lisp_StringNotGreaterp(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, c1, c2, ch1, ch2;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    string1 += start1;
    string2 += start2;

    for (c1 = start1, c2 = start2; c1 <= end1 && c2 <= end2;
	 c1++, c2++, string1++, string2++) {
	ch1 = toupper(*string1);
	ch2 = toupper(*string2);
	if (ch1 < ch2)
	    return (REAL(c1));
	else if (ch1 != ch2)
	    return (NIL);
	else if (!*string1)
	    break;
    }

    return (REAL(c1));
}

LispObj *
Lisp_StringNotLessp(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, c1, c2, ch1, ch2;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    string1 += start1;
    string2 += start2;

    for (c1 = start1, c2 = start2; c1 <= end1 && c2 <= end2;
	 c1++, c2++, string1++, string2++) {
	ch1 = toupper(*string1);
	ch2 = toupper(*string2);
	if (ch1 > ch2)
	    return (REAL(c1));
	else if (ch1 != ch2)
	    return (NIL);
	else if (!*string1)
	    break;
    }

    return (REAL(c1));
}

LispObj *
Lisp_StringNotEqual(LispMac *mac, LispObj *list, char *fname)
{
    char *string1, *string2;
    int start1, end1, start2, end2, c1, c2;

    _LispGetStringArgs(mac, list, fname, &string1, &string2,
		       &start1, &end1, &start2, &end2);

    string1 += start1;
    string2 += start2;

    for (c1 = start1, c2 = start2; c1 <= end1 && c2 <= end2;
	 c1++, c2++, string1++, string2++)
	if (toupper(*string1) != toupper(*string2))
	    return (REAL(c1));

    return (NIL);
}

LispObj *
Lisp_StringUpcase(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res;
    char *string, *str;
    int start, end, c, done;

    _LispGetStringCaseArgs(mac, list, fname, &string, &start, &end);

    /* first check if something need to be done */
    for (done = 1, c = start; c < end; c++)
	if (string[c] != toupper(string[c])) {
	    done = 0;
	    break;
	}

    if (done)
	return (CAR(list));

    /* upcase a copy of argument */
    str = LispStrdup(mac, string);
    for (c = start; c < end; c++)
	str[c] = toupper(str[c]);

    res = STRING(str);
    LispFree(mac, str);

    return (res);
}

LispObj *
Lisp_StringDowncase(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res;
    char *string, *str;
    int start, end, c, done;

    _LispGetStringCaseArgs(mac, list, fname, &string, &start, &end);

    /* first check if something need to be done */
    for (done = 1, c = start; c < end; c++)
	if (string[c] != tolower(string[c])) {
	    done = 0;
	    break;
	}

    if (done)
	return (CAR(list));

    /* downcase a copy of argument */
    str = LispStrdup(mac, string);
    for (c = start; c < end; c++)
	str[c] = tolower(str[c]);

    res = STRING(str);
    LispFree(mac, str);

    return (res);
}

LispObj *
Lisp_StringCapitalize(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res;
    char *string, *str;
    int start, end, c, done, up;

    _LispGetStringCaseArgs(mac, list, fname, &string, &start, &end);

    /* first check if something need to be done */
    for (done = up = 1, c = start; c < end; c++) {
	if (up) {
	    if (!isalpha(string[c]))
		continue;
	    if (string[c] != toupper(string[c])) {
		done = 0;
		break;
	    }
	    up = 0;
	}
	else {
	    if (isalpha(string[c])) {
		if (string[c] != tolower(string[c])) {
		    done = 0;
		    break;
		}
	    }
	    else
		up = 1;
	}
    }

    if (done)
	return (CAR(list));

    /* capitalize a copy of argument */
    str = LispStrdup(mac, string);
    for (up = 1, c = start; c < end; c++) {
	if (up) {
	    if (!isalpha(str[c]))
		continue;
	    str[c] = toupper(str[c]);
	    up = 0;
	}
	else {
	    if (isalpha(string[c]))
		str[c] = tolower(str[c]);
	    else
		up = 1;
	}
    }

    res = STRING(str);
    LispFree(mac, str);

    return (res);
}

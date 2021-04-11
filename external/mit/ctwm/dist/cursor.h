/*
 * Cursor include file
 *
 *
 * Copyright 1992 Claude Lecommandeur.
 *
 * 26-Feb-97 Richard Levitte    Initial Version.
 */

#ifndef _CTWM_CURSOR_H
#define _CTWM_CURSOR_H

void NewFontCursor(Cursor *cp, const char *str);
int NewBitmapCursor(Cursor *cp, char *source, char *mask);
Cursor MakeStringCursor(char *string);
#endif /* _CTWM_CURSOR_H */


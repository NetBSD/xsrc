/*
 * utility routines header file
 *
 *
 *        Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                           Salt Lake City, Utah
 *   Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                         Cambridge, Massachusetts
 *
 * $XConsortium: util.h,v 1.10 89/12/10 17:47:04 jim Exp $
 *
 * 28-Oct-87 Thomas E. LaStrange                File created
 *
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_UTIL_H
#define _CTWM_UTIL_H

#include <ctype.h>
#include <stdio.h>


/* Simple int utils */
static inline int max(int a, int b)
{
	return a > b ? a : b;
}

static inline int min(int a, int b)
{
	return a < b ? a : b;
}

#ifndef MAX
#define MAX(x,y) (max(x,y))
#endif
#ifndef MIN
#define MIN(x,y) (min(x,y))
#endif
#ifndef ABS
#define ABS(x) ((x)<0?-(x):(x))
#endif

/*
 * Define some helper macros, because "The argument to toupper() must be
 * EOF or representable as an unsigned char; otherwise, the behavior is
 * undefined." In particular, an argument of type "char" is problematic
 * (gcc:  warning: array subscript has type 'char').
 */
#define Isascii(c)      isascii((int)(unsigned char)(c))
#define Isdigit(c)      isdigit((int)(unsigned char)(c))
#define Islower(c)      islower((int)(unsigned char)(c))
#define Isupper(c)      isupper((int)(unsigned char)(c))
#define Tolower(c)      tolower((int)(unsigned char)(c))
#define Toupper(c)      toupper((int)(unsigned char)(c))

char     *ExpandFilename(const char *name);
char     *ExpandFilePath(char *path);

void GetColor(int kind, Pixel *what, const char *name);
void GetShadeColors(ColorPair *cp);
bool UpdateFont(MyFont *font, int height);
void CreateFonts(ScreenInfo *scr);
#if 0
void move_to_after(TwmWindow *t, TwmWindow *after);
#endif
void RescueWindows(void);
void DebugTrace(char *file);


void safe_strncpy(char *dest, const char *src, size_t size);

extern FILE *tracefile;

#endif /* _CTWM_UTIL_H */

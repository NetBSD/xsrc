/*
Copyright (c) 1997 by Mark Leisher
Copyright (c) 1998-2000 by Juliusz Chroboczek

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/* $XFree86: xc/lib/font/FreeType/ft.h,v 1.13 2000/11/14 16:54:42 dawes Exp $ */

#undef DEBUG_TRUETYPE

#ifdef DEBUG_TRUETYPE
#define MUMBLE(s) (ErrorF((s)))
#define MUMBLE1(s,x) (ErrorF((s),(x)))
#else
#define MUMBLE(s)
#define MUMBLE1(s,x)
#endif

#undef MAX
#define MAX(h,i) ((h) > (i) ? (h) : (i))
#define ADJUSTMAX(m,v) if((v)>(m)) (m)=(v)
#undef MIN
#define MIN(l,o) ((l) < (o) ? (l) : (o))
#define ADJUSTMIN(m,v) if ((v)<(m)) (m)=(v)

/* When comparing floating point values, we want to ignore small errors. */
#define NEGLIGIBLE ((double)0.001)
/* Are x and y significantly different? */
#define DIFFER(x,y) (fabs((x)-(y))>=NEGLIGIBLE*fabs(x))
/* Is x significantly different from 0 w.r.t. y? */
#define DIFFER0(x,y) (fabs(x)>=NEGLIGIBLE*fabs(y))

/* Two to the sixteenth power, as a double. */
#define TWO_SIXTEENTH ((double)(1<<16))
#define TWO_SIXTH ((double)(1<<6))

/* nameID macros for getting strings from the TT font. */

#define TTF_COPYRIGHT 0
#define TTF_TYPEFACE  1
#define TTF_PSNAME    6

/* Data structures used across files */

struct ttf_mapping
{
  int has_cmap;
  TT_CharMap cmap;
  int base;
  struct font_encoding *encoding;
  struct font_encoding_mapping *mapping;
};

/* Prototypes */

/* ftfuncs.c */

void FreeTypeRegisterFontFileFunctions(void);

/* ftenc.c */

int ttf_pick_cmap(char*, int, char*, TT_Face, struct ttf_mapping *);
int ftstrcasecmp(const char *s1, const char *s2);
unsigned ttf_remap(unsigned code, struct ttf_mapping *tm);

/* ftutil.c */

long ttf_atol(char*, char**, int);
int ttf_u2a(int, char*, char*, int);
int FTtoXReturnCode(int);
int ttf_GetEnglishName(TT_Face, char *, int);
int ttf_checkForTTCName(char*, char**, int*);


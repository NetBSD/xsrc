/*
Copyright (c) 1997 by Mark Leisher
Copyright (c) 1998 by Juliusz Chroboczek

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

/* $XFree86: xc/lib/font/FreeType/ftutil.c,v 1.11 2000/08/11 21:12:42 dawes Exp $ */

#ifndef FONTMODULE
#include <ctype.h>
#endif
#include "font.h"
#include "ttconfig.h"
#include "freetype.h"
#include "ft.h"

#ifndef LSBFirst
#define LSBFirst 0
#define MSBFirst 1
#endif

#define LOBYTE(s,byte) ((byte)==LSBFirst?*(char*)(s):*((char*)(s)+1))
#define HIBYTE(s,byte) ((byte)==LSBFirst?*((char*)(s)+1):*(char*)(s))

static unsigned char a2i[128] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static unsigned char odigits[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char ddigits[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char hdigits[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
    0x7e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

#define isdigok(m, d) (m[(d) >> 3] & (1 << ((d) & 7)))

long
ttf_atol(char *s, char **end, int base)
{
    long v, neg;
    unsigned char *dmap;

    if (s==0 || *s==0)
      return 0;

    /*
     * Make sure the radix is something recognizable.  Default to 10.
     */
    switch (base) {
      case 8: dmap = odigits; break;
      case 16: dmap = hdigits; break;
      default: base = 10; dmap = ddigits; break;
    }

    /*
     * Check for a minus sign.
     */
    neg = 0;
    if (*s=='-') {
        s++;
        neg = 1;
    }

    /*
     * Check for the special hex prefix.
     */
    if (*s=='0' && (*(s + 1)=='x' || *(s + 1)=='X')) {
        base = 16;
        dmap = hdigits;
        s += 2;
    }

    for (v = 0; isdigok(dmap, *s); s++)
      v = (v * base) + a2i[(int) *s];

    if (end != 0)
      *end = s;

    return (!neg) ? v : -v;
}

/* Take slen bytes from a Unicode string and convert them to ISO
 * 8859-1.  byte specifies the endianness of the string. */
int
ttf_u2a(int slen, char *from, char *to, int byte)
{
  int i;

  for (i = 0; i < slen; i += 2) {
    if(HIBYTE(from+i,byte)!=0)
      *to++='?';
    else
      *to++ = LOBYTE(from+i,byte);
  }
  *to = 0;
  return (slen >> 1);
}


int FTtoXReturnCode(int rc)
{
  if(rc>>8==0x01)
    return AllocError;
  else return BadFontFormat;
}



/* A generic routine to get a name from the TT name table.  This
 * routine tries to get a name in English.  The encoding will be
 * converted to ISO 8859-1.
 *
 * The particular name ID mut be provided (e.g. nameID = 0 for
 * copyright string, nameID = 6 for Postscript name, nameID = 1 for
 * typeface name.
 *
 * Returns the number of bytes added, -1 on failure. */

int 
ttf_GetEnglishName(TT_Face face, char *name, int nameID)
{
  int i, nrec;
  unsigned short slen;
  unsigned short nrPlatformID, nrEncodingID, nrLanguageID, nrNameID;
  char *s;

  nrec = TT_Get_Name_Count(face);

  for (i = 0; i < nrec; i++) {
    if(TT_Get_Name_ID(face, i, &nrPlatformID, &nrEncodingID,
                      &nrLanguageID, &nrNameID))
      continue;
    if (/* check for Microsoft, Unicode, English */
        (nrPlatformID==3 && nrEncodingID==1 &&
         nrNameID==nameID &&
         (nrLanguageID==0x0409 || nrLanguageID==0x0809 ||
          nrLanguageID==0x0c09 || nrLanguageID==0x1009 ||
          nrLanguageID==0x1409 || nrLanguageID==0x1809)) ||
        /* or for Apple, Unicode, English */
        ((nrPlatformID==0 && nrNameID==nameID &&
          nrLanguageID==0))) {
      if(!TT_Get_Name_String(face, i, &s, &slen))
        return ttf_u2a(slen, s, name, MSBFirst);
    }
  }

  /* Must be some dodgy font.  Pretend that Apple Roman is ISO 8859-1. */
  for (i = 0; i < nrec; i++) {
    if(TT_Get_Name_ID(face, i, &nrPlatformID, &nrEncodingID,
                      &nrLanguageID, &nrNameID))
      continue;
    /* Check for Apple, Roman, English */
    if (nrPlatformID==1 && nrEncodingID==0 &&
        nrLanguageID==0 && nrNameID==nameID) {
      TT_Get_Name_String(face, i, &s, &slen);
      memcpy(name,s,slen);
      name[slen]=0;
      return slen;
    }
  }

  /* Must be some font that can only be named in Polish or something. */
  return -1;
}

int
ttf_checkForTTCName(char *fileName, char **realFileName, int *faceNumber)
{
  int length;
  int fn;
  int i, j;
  char *start, *realName;

  length=strlen(fileName);
  if(length<4)
    return 0;

  if(strcasecmp(fileName+(length-4), ".ttc"))
    return 0;

  if (!(realName = xalloc(length + 1)))
    return 0;

  strcpy(realName, fileName);
  *realFileName=realName;
  start = strchr(realName, ':');
  if (start) {
    fn=0;
    i=1;
    while(isdigit(start[i])) {
      fn*=10;
      fn+=start[i]-'0';
      i++;
    }
    if(start[i]==':') {
      *faceNumber=fn;
      i++;
      j = 0;
      while (start[i]) {
	start[j++] = start[i++];
      }
      start[j] = '\0';
      return 1;
    }
  }

  *faceNumber=0;
  return 1;
}

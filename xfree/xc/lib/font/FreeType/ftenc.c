/* $XFree86: xc/lib/font/FreeType/ftenc.c,v 1.18 2001/10/28 03:32:43 tsi Exp $ */

/* 
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

/* TrueType-dependent encoding code */
#include "fontmisc.h"		/* defines xalloc and friends */
#include "fontenc.h"
#include "ttconfig.h"
#include "freetype.h"
#include "ft.h"
#ifndef FONTMODULE
#include <string.h>
#else
#include "xf86_ansic.h"
#endif


static int find_cmap(int, int, int, TT_Face, TT_CharMap *);
static int find_cmap_default(TT_Face, struct ttf_mapping*);

int
ttf_pick_cmap(char *xlfd, int length, char *filename, TT_Face face,
              struct ttf_mapping *tm)
{
  FontEncPtr encoding;
  FontMapPtr mapping;
  TT_CharMap cmap;
  
  char *encoding_name=0;

  if(xlfd)
    encoding_name=FontEncFromXLFD(xlfd, length);
  if(!encoding_name)
    encoding_name="iso8859-1";

  if(!strcasecmp(encoding_name, "truetype-raw")) {
    tm->has_cmap=0;
    tm->base=0;
    tm->mapping=0;
    return 0;
  } else {
    if((encoding=FontEncFind(encoding_name, filename))) {
      for(mapping=encoding->mappings; mapping; mapping=mapping->next) {
        if(!find_cmap(mapping->type, mapping->pid, mapping->eid, face, 
                      &cmap)) {
          tm->has_cmap=1;
          tm->cmap=cmap;
          if(!strcasecmp(encoding_name, "microsoft-symbol")) {
            /* deal with undocumented lossage */
            TT_Face_Properties properties;
            if(!TT_Get_Face_Properties(face, &properties) &&
               properties.os2!=0)
              tm->base=properties.os2->usFirstCharIndex-0x20;
            else
              tm->base=0;
          } else
            tm->base=0;
          tm->mapping=mapping;
          return 0;
        }
      }
    }
  }

  /* Failed to find a suitable mapping and cmap */
  return find_cmap_default(face, tm);
}

static int
find_cmap_default(TT_Face face, struct ttf_mapping *tm)
{
  TT_CharMap cmap;

  /* Try to find a Unicode charmap */
    if(!find_cmap(FONT_ENCODING_UNICODE, 0, 0, face, &cmap)) {
      tm->has_cmap=1;
      tm->cmap=cmap;
      tm->base=0;
      tm->mapping=0;
      return 0;
    }

  /* Try to get the first charmap in the file */
  if(!TT_Get_CharMap(face, 0, &cmap)) {
    tm->has_cmap=1;
    tm->cmap=cmap;
    tm->base=0;
    tm->mapping=0;
    return 0;
  }

  /* Tough. */
  tm->has_cmap=0;
  tm->base=0;
  tm->mapping=0;
  return 0;
}


static int 
find_cmap(int type, int pid, int eid, TT_Face face, TT_CharMap *cmap)
{
  int i, n;
  unsigned short p,e;

  n=TT_Get_CharMap_Count(face);

  switch(type) {
  case FONT_ENCODING_TRUETYPE:  /* specific cmap */
    for(i=0; i<n; i++) {
      if(!TT_Get_CharMap_ID(face, i, &p, &e) && p==pid && e==eid) {
        if(!TT_Get_CharMap(face, i, cmap))
          return 0;
      }
    }
    break;
  case FONT_ENCODING_UNICODE:   /* any Unicode cmap */
    /* prefer Microsoft Unicode */
    for(i=0; i<n; i++) {
      if(!TT_Get_CharMap_ID(face, i, &p, &e) && p==3 && e==1) {
        if(!TT_Get_CharMap(face, i, cmap)) {
          return 0;
        } else
          break;
      }
    }
    /* Try Apple Unicode */
    for(i=0; i<n; i++) {
      if(!TT_Get_CharMap_ID(face, i, &p, &e) && p==0) {
        if(!TT_Get_CharMap(face, i, cmap)) {
          return 0;
        }
        /* but don't give up yet -- there may be more than one cmap
         * with pid=0 */
      }
    }
    /* ISO Unicode? */
    for(i=0; i<n; i++) {
      if(!TT_Get_CharMap_ID(face, i, &p, &e) && p==2 && e==1) {
        if(!TT_Get_CharMap(face, i, cmap)) {
          return 0;
        } else
          break;
      }
    }
    break;
  default:
    return 1;
  }
  return 1;
}

unsigned 
ttf_remap(unsigned code, struct ttf_mapping *tm)
{
  unsigned index;

  if(tm->mapping) {
    index=FontEncRecode(code, tm->mapping);
  } else {
    if(code<0x100 || !tm->has_cmap)
      index=code;
    else
      return 0;
  }
  index += tm->base;
  if(tm->has_cmap)
    return TT_Char_Index(tm->cmap, index);
  else
    return index;
}


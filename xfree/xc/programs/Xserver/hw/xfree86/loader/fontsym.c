/* $XFree86: xc/programs/Xserver/hw/xfree86/loader/fontsym.c,v 1.6 2000/02/13 06:15:41 dawes Exp $ */

#include "font.h"
#include "sym.h"
#include "fntfilst.h"
#include "fontenc.h"
#include "fntfilio.h"
#include "fntfil.h"
#include "fontutil.h"
#include "fontxlfd.h"
#include "fontcache.h"

LOOKUP fontLookupTab[] = {

  SYMFUNC(TwoByteSwap)
  SYMFUNC(FourByteSwap)
  SYMFUNC(FontCouldBeTerminal)
  SYMFUNC(BufFileRead)
  SYMFUNC(BufFileWrite)
  SYMFUNC(CheckFSFormat)
  SYMFUNC(FontFileOpen)
  SYMFUNC(FontFileRegisterRenderer)
  SYMFUNC(FontParseXLFDName)
  SYMFUNC(FontFileCloseFont)
  SYMFUNC(FontFileOpenBitmap)
  SYMFUNC(FontFileCompleteXLFD)
  SYMFUNC(FontFileCountDashes)
  SYMFUNC(FontFileFindNameInDir)
  SYMFUNC(FontFileClose)
  SYMFUNC(FontComputeInfoAccelerators)
  SYMFUNC(FontDefaultFormat)
  SYMFUNC(NameForAtom)
  SYMFUNC(BitOrderInvert)
  SYMFUNC(FontFileMatchRenderer)
  SYMFUNC(RepadBitmap)
  SYMFUNC(font_encoding_name)
  SYMFUNC(font_encoding_recode)
  SYMFUNC(font_encoding_find)
  SYMFUNC(font_encoding_from_xlfd)
  SYMFUNC(CreateFontRec)
  SYMFUNC(DestroyFontRec)
  
  SYMVAR(FontFileBitmapSources)

  /* fontcache.c */
  SYMFUNC(FontCacheGetSettings)
  SYMFUNC(FontCacheGetStatistics)
  SYMFUNC(FontCacheChangeSettings)
  SYMFUNC(FontCacheOpenCache)
  SYMFUNC(FontCacheCloseCache)
  SYMFUNC(FontCacheSearchEntry)
  SYMFUNC(FontCacheGetEntry)
  SYMFUNC(FontCacheInsertEntry)
  SYMFUNC(FontCacheGetBitmap)

  { 0, 0 },

};

/* $XFree86: xc/extras/freetype2/src/autofit/afdummy.h,v 1.2 2005/02/28 23:19:13 dawes Exp $ */

#ifndef __AFDUMMY_H__
#define __AFDUMMY_H__

#include "aftypes.h"

FT_BEGIN_HEADER

 /* a dummy script metrics class used when no hinting should
  * be performed. This is the default for non-latin glyphs !
  */

#ifndef FT_MAKE_OPTION_SINGLE_OBJECT
  FT_LOCAL( const AF_ScriptClassRec )    af_dummy_script_class;
#endif

/* */

FT_END_HEADER

#endif /* __AFDUMMY_H__ */

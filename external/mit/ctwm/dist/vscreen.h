/* 
 *  [ ctwm ]
 *
 *  Copyright 1992 Claude Lecommandeur.
 *            
 * Permission to use, copy, modify  and distribute this software  [ctwm] and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above  copyright notice appear  in all copies and that both that
 * copyright notice and this permission notice appear in supporting documen-
 * tation, and that the name of  Claude Lecommandeur not be used in adverti-
 * sing or  publicity  pertaining to  distribution of  the software  without
 * specific, written prior permission. Claude Lecommandeur make no represen-
 * tations  about the suitability  of this software  for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * Claude Lecommandeur DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL  IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL  Claude Lecommandeur  BE LIABLE FOR ANY SPECIAL,  INDIRECT OR
 * CONSEQUENTIAL  DAMAGES OR ANY  DAMAGES WHATSOEVER  RESULTING FROM LOSS OF
 * USE, DATA  OR PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR
 * OTHER  TORTIOUS ACTION,  ARISING OUT OF OR IN  CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Claude Lecommandeur [ lecom@sic.epfl.ch ][ April 1992 ]
 */
#include "types.h"

#ifndef _VSCREEN_
#define _VSCREEN_

struct virtualScreen {
  int   x, y, w, h;		/* x,y relative to XineramaRoot */
  Window window;
  /* Boolean main; */
  struct WorkSpaceWindow *wsw;
  struct virtualScreen *next;
};

void InitVirtualScreens (ScreenInfo *scr);
virtualScreen *findIfVScreenOf (int x, int y);
virtualScreen *getVScreenOf (int x, int y);
Bool CtwmGetVScreenMap (Display *display, Window rootw,
			char *outbuf, int *outbuf_len);
Bool CtwmSetVScreenMap(Display *display, Window rootw,
		       struct virtualScreen *firstvs);

#endif /* -VSCREEN- */

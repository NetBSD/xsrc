/*
 * Copyright 1998,1999 by Alan Hourihane, Wigan, England.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *           Mike Chapman <mike@paranoia.com>, 
 *           Juanjo Santamarta <santamarta@ctv.es>, 
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp> 
 *           David Thomas <davtom@dream.org.uk>. 
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_cursor.h,v 1.2 2001/04/19 12:40:33 alanh Exp $ */

#define CS(x)   (0x8500+(x<<2))

#define sis300EnableHWCursor()\
  *(volatile CARD32 *)(pSiS->IOBase + CS(0)) |= 0x40000000;
#define sis300DisableHWCursor()\
  *(volatile CARD32 *)(pSiS->IOBase + CS(0)) &= 0x3FFFFFFF;

#define sis300SetCursorBGColor(color)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(1)) = (color);
#define sis300SetCursorFGColor(color)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(2)) = (color);

#define sis300SetCursorPositionX(x,preset)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(3)) = (x) | ((preset) << 16);
#define sis300SetCursorPositionY(y,preset)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(4)) = (y) | ((preset) << 16);

#define sis300SetCursorAddress(address)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(0)) &= 0xFFFF0000;\
  *(volatile CARD32 *)(pSiS->IOBase + CS(0)) |= address;

#define sis300SetCursorPatternSelect(pat_id)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(0)) &= 0xF0FFFFFF;\
  *(volatile CARD32 *)(pSiS->IOBase + CS(0)) |= ((pat_id) << 24);



#define sis301EnableHWCursor()\
  *(volatile CARD32 *)(pSiS->IOBase + CS(8)) |= 0x40000000;
#define sis301DisableHWCursor()\
  *(volatile CARD32 *)(pSiS->IOBase + CS(8)) &= 0xBFFFFFFF;

#define sis301SetCursorBGColor(color)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(9)) = (color);
#define sis301SetCursorFGColor(color)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(10)) = (color);

#define sis301SetCursorPositionX(x,preset)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(11)) = (x) | ((preset) << 16);
#define sis301SetCursorPositionY(y,preset)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(12)) = (y) | ((preset) << 16);

#define sis301SetCursorAddress(address)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(8)) &= 0xFFFF0000;\
  *(volatile CARD32 *)(pSiS->IOBase + CS(8)) |= address;

#define sis301SetCursorPatternSelect(pat_id)\
  *(volatile CARD32 *)(pSiS->IOBase + CS(8)) &= 0xF0FFFFFF;\
  *(volatile CARD32 *)(pSiS->IOBase + CS(8)) |= ((pat_id) << 24);

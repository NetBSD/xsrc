/*
 * $TOG: Porthole.h /main/4 1998/02/11 14:54:31 kaleb $
 *
Copyright 1990, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */
/* $XFree86: xc/lib/Xaw/Porthole.h,v 1.4 1998/10/03 08:42:15 dawes Exp $ */

#ifndef _XawPorthole_h
#define _XawPorthole_h

#include <X11/Xaw/Reports.h>

/*****************************************************************************
 * 
 * Porthole Widget (subclass of Composite)
 * 
 * This widget is similar to a viewport without scrollbars.  Child movement
 * is done by external panners or scrollbars.
 * 
 * Parameters:
 * 
 *  Name		Class		Type		Default
 *  ----		-----		----		-------
 * 
 *  background		Background	Pixel		XtDefaultBackground
 *  border	        BorderColor	Pixel		XtDefaultForeground
 *  borderWidth		BorderWidth	Dimension	1
 *  height		Height		Dimension	0
 *  reportCallback	ReportCallback	Pointer		NULL
 *  width		Width		Dimension	0
 *  x 			Position	Position	0
 *  y			Position	Position	0
 * 
 *****************************************************************************/

extern WidgetClass portholeWidgetClass;
typedef struct _PortholeClassRec *PortholeWidgetClass;
typedef struct _PortholeRec      *PortholeWidget;

#endif /* _XawPorthole_h */

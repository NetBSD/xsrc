/* $TOG: LogoP.h /main/12 1998/02/09 14:09:19 kaleb $ */
/*

Copyright 1988, 1993, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/


#ifndef _XawLogoP_h
#define _XawLogoP_h

#include "Logo.h"
#include <X11/Xaw/SimpleP.h>

typedef struct {
	 Pixel	 fgpixel;
	 GC	 foreGC;
	 GC	 backGC;
	 Boolean shape_window;
	 Boolean need_shaping;
   } LogoPart;

typedef struct _LogoRec {
   CorePart core;
   SimplePart simple;
   LogoPart logo;
   } LogoRec;

typedef struct {int dummy;} LogoClassPart;

typedef struct _LogoClassRec {
   CoreClassPart core_class;
   SimpleClassPart simple_class;
   LogoClassPart logo_class;
   } LogoClassRec;

extern LogoClassRec logoClassRec;

#endif /* _XawLogoP_h */
